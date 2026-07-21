#include "tsfi_parc_contract_coefficients.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

#define MOTZKIN_PRIME_FIELD 953467954114363ULL

static double hash_string_to_coefficient(const char *str) {
    if (!str) return 0.0;
    uint64_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    uint64_t mod_val = hash % MOTZKIN_PRIME_FIELD;
    return (double)mod_val / (double)MOTZKIN_PRIME_FIELD;
}

int tsfi_contract_coefficients_init(tsfi_contract_coefficient_matrix_t *matrix, const char *dynamic_contract_addr) {
    if (!matrix || !dynamic_contract_addr) return -1;
    memset(matrix, 0, sizeof(tsfi_contract_coefficient_matrix_t));
    matrix->is_initialized = true;

    if (strncmp(dynamic_contract_addr, "dynamic_", 8) == 0) {
        snprintf(matrix->target_contract_address, sizeof(matrix->target_contract_address), "%s", dynamic_contract_addr);
    } else {
        snprintf(matrix->target_contract_address, sizeof(matrix->target_contract_address), "dynamic_%s", dynamic_contract_addr);
    }
    return 0;
}

int tsfi_contract_coefficients_set_metadata(tsfi_contract_coefficient_matrix_t *matrix, const char *name, const char *symbol, uint64_t total_supply) {
    if (!matrix || !name || !symbol) return -1;
    snprintf(matrix->name, sizeof(matrix->name), "%s", name);
    snprintf(matrix->symbol, sizeof(matrix->symbol), "%s", symbol);
    matrix->total_supply = total_supply;

    matrix->name_coefficient = hash_string_to_coefficient(name);
    matrix->symbol_coefficient = hash_string_to_coefficient(symbol);
    matrix->total_supply_coefficient = (total_supply > 0) ? (log10((double)total_supply) / 18.0) : 0.0;
    return 0;
}

int tsfi_contract_coefficients_register_setting(tsfi_contract_coefficient_matrix_t *matrix, const char *setting_key, uint64_t raw_val) {
    if (!matrix || !setting_key) return -1;
    if (matrix->setting_count >= TSFI_MAX_SETTING_COEFFICIENTS) return -2;

    tsfi_setting_coefficient_t *s = &matrix->settings[matrix->setting_count];
    snprintf(s->key, sizeof(s->key), "%s", setting_key);
    s->raw_value = raw_val;

    double key_hash = hash_string_to_coefficient(setting_key);
    s->scalar_coefficient = key_hash * ((double)(raw_val % 1000) / 1000.0 + 0.1);

    matrix->setting_count++;
    return 0;
}

int tsfi_contract_coefficients_register_holder(tsfi_contract_coefficient_matrix_t *matrix, const char *holder_addr, uint64_t balance) {
    if (!matrix || !holder_addr) return -1;
    if (matrix->holder_count >= TSFI_MAX_HOLDER_COEFFICIENTS) return -2;

    tsfi_holder_coefficient_t *h = &matrix->holders[matrix->holder_count];
    snprintf(h->holder_address, sizeof(h->holder_address), "%s", holder_addr);
    h->balance = balance;
    h->normalized_coefficient = 0.0;

    matrix->holder_count++;
    return 0;
}

int tsfi_contract_coefficients_register_lp(tsfi_contract_coefficient_matrix_t *matrix, const char *pair_addr, uint64_t reserve_a, uint64_t reserve_b) {
    if (!matrix || !pair_addr) return -1;
    if (matrix->lp_count >= TSFI_MAX_LP_COEFFICIENTS) return -2;

    tsfi_lp_coefficient_t *lp = &matrix->lps[matrix->lp_count];
    snprintf(lp->pair_address, sizeof(lp->pair_address), "%s", pair_addr);
    lp->reserve_a = reserve_a;
    lp->reserve_b = reserve_b;
    lp->ratio_coefficient = (reserve_b > 0) ? ((double)reserve_a / (double)reserve_b) : 1.0;

    matrix->lp_count++;
    return 0;
}

int tsfi_contract_coefficients_evaluate(tsfi_contract_coefficient_matrix_t *matrix) {
    if (!matrix || !matrix->is_initialized) return -1;

    /* 1. Evaluate Holder Coefficients */
    uint64_t total_holder_bal = 0;
    for (uint32_t i = 0; i < matrix->holder_count; i++) {
        total_holder_bal += matrix->holders[i].balance;
    }
    double holder_sum = 0.0;
    for (uint32_t i = 0; i < matrix->holder_count; i++) {
        matrix->holders[i].normalized_coefficient = (total_holder_bal > 0) ? ((double)matrix->holders[i].balance / (double)total_holder_bal) : 0.0;
        holder_sum += matrix->holders[i].normalized_coefficient;
    }

    /* 2. Evaluate LP Coefficients */
    double lp_sum = 0.0;
    for (uint32_t j = 0; j < matrix->lp_count; j++) {
        lp_sum += matrix->lps[j].ratio_coefficient;
    }

    /* 3. Evaluate Function/Setting Coefficients */
    double setting_sum = 0.0;
    for (uint32_t k = 0; k < matrix->setting_count; k++) {
        setting_sum += matrix->settings[k].scalar_coefficient;
    }

    /* 4. Sum Universal Contract Coefficients */
    matrix->universal_coefficient_sum = matrix->name_coefficient + 
                                        matrix->symbol_coefficient + 
                                        matrix->total_supply_coefficient + 
                                        setting_sum + 
                                        holder_sum + 
                                        lp_sum;

    /* 5. Map into Vulkan Lissajous Frequencies & EDO-22 Octave */
    matrix->dynamic_lissajous_f_x = 1.0 + (matrix->name_coefficient + matrix->symbol_coefficient + holder_sum) * 4.0;
    matrix->dynamic_lissajous_f_y = 1.0 + (matrix->total_supply_coefficient + lp_sum) * 3.0;
    matrix->dynamic_lissajous_f_z = 1.0 + setting_sum * 2.0;

    matrix->EDO22_pitch_class = fmod(matrix->universal_coefficient_sum * 22.0, 22.0);

    return 0;
}

uint32_t tsfi_contract_coefficients_yul_resolve_offset(uint32_t field_id) {
    // Mimics execution of Yul DDL ContractCoefficientsDDL selector 0xc0ef0001
    switch (field_id) {
        case 0: return 0;   // magic ("COEF")
        case 1: return 4;   // name_hash (32 bytes)
        case 2: return 36;  // symbol_hash (16 bytes)
        case 3: return 52;  // total_supply (8 bytes)
        case 4: return 60;  // setting_count (4 bytes)
        case 5: return 64;  // holder_count (4 bytes)
        case 6: return 68;  // lp_count (4 bytes)
        case 7: return 72;  // lissajous_f_x (8 bytes)
        case 8: return 80;  // lissajous_f_y (8 bytes)
        case 9: return 88;  // lissajous_f_z (8 bytes)
        case 10: return 96; // edo22_pitch_class (8 bytes)
        case 11: return 104;// motzkin_prime (8 bytes)
        default: return 0xFFFFFFFF;
    }
}

int tsfi_contract_coefficients_call_and_resolve(tsfi_contract_coefficient_matrix_t *matrix, const char *func_signature, uint32_t selector) {
    if (!matrix || !matrix->is_initialized || !func_signature) return -1;

    // Simulate EVM call execution against target contract (dynamic_<address>) via standard 4-byte selector
    switch (selector) {
        case 0x18160ddd: { // totalSupply()
            uint64_t simulated_supply = 1000000000000000000ULL;
            matrix->total_supply = simulated_supply;
            matrix->total_supply_coefficient = log10((double)simulated_supply) / 18.0;
            break;
        }
        case 0x06fdde03: { // name()
            snprintf(matrix->name, sizeof(matrix->name), "Atropa PulseChain Dynamic Token");
            matrix->name_coefficient = hash_string_to_coefficient(matrix->name);
            break;
        }
        case 0x95d89b41: { // symbol()
            snprintf(matrix->symbol, sizeof(matrix->symbol), "ATROPA");
            matrix->symbol_coefficient = hash_string_to_coefficient(matrix->symbol);
            break;
        }
        case 0x0902f1de: { // getReserves()
            tsfi_contract_coefficients_register_lp(matrix, "0xUniswapPairContract", 5000000, 10000000);
            break;
        }
        default: { // Arbitrary function call resolution (e.g., owner(), decimals(), paused())
            uint64_t simulated_return = (uint64_t)selector % 1000;
            tsfi_contract_coefficients_register_setting(matrix, func_signature, simulated_return);
            break;
        }
    }

    return tsfi_contract_coefficients_evaluate(matrix);
}

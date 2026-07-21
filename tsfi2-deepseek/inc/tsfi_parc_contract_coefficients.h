#ifndef TSFI_PARC_CONTRACT_COEFFICIENTS_H
#define TSFI_PARC_CONTRACT_COEFFICIENTS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define TSFI_MAX_HOLDER_COEFFICIENTS   32
#define TSFI_MAX_LP_COEFFICIENTS       16
#define TSFI_MAX_SETTING_COEFFICIENTS  16

typedef struct {
    char key[32];
    uint64_t raw_value;
    double scalar_coefficient; // c_setting,k
} tsfi_setting_coefficient_t;

typedef struct {
    char holder_address[64];
    uint64_t balance;
    double normalized_coefficient; // c_holder,i
} tsfi_holder_coefficient_t;

typedef struct {
    char pair_address[64];
    uint64_t reserve_a;
    uint64_t reserve_b;
    double ratio_coefficient; // c_lp,j
} tsfi_lp_coefficient_t;

typedef struct {
    bool is_initialized;
    char target_contract_address[64]; // dynamic_<address>
    
    /* Metadata Coefficients */
    char name[64];
    double name_coefficient; // c_name
    char symbol[16];
    double symbol_coefficient; // c_symbol
    uint64_t total_supply;
    double total_supply_coefficient; // c_totalSupply

    /* Contract Functions & Settings Coefficients */
    uint32_t setting_count;
    tsfi_setting_coefficient_t settings[TSFI_MAX_SETTING_COEFFICIENTS];

    /* Holders & LP Coefficients */
    uint32_t holder_count;
    tsfi_holder_coefficient_t holders[TSFI_MAX_HOLDER_COEFFICIENTS];
    uint32_t lp_count;
    tsfi_lp_coefficient_t lps[TSFI_MAX_LP_COEFFICIENTS];
    
    /* Evaluated Universal Contract Polynomial Characteristics */
    double universal_coefficient_sum;
    double dynamic_lissajous_f_x;
    double dynamic_lissajous_f_y;
    double dynamic_lissajous_f_z;
    double EDO22_pitch_class;
} tsfi_contract_coefficient_matrix_t;

/* Initialize Dynamic Contract Coefficient Matrix */
int tsfi_contract_coefficients_init(tsfi_contract_coefficient_matrix_t *matrix, const char *dynamic_contract_addr);

/* Set Metadata Coefficients (Name, Symbol, Total Supply) */
int tsfi_contract_coefficients_set_metadata(tsfi_contract_coefficient_matrix_t *matrix, const char *name, const char *symbol, uint64_t total_supply);

/* Register Any Function or Setting as a Contract Coefficient */
int tsfi_contract_coefficients_register_setting(tsfi_contract_coefficient_matrix_t *matrix, const char *setting_key, uint64_t raw_val);

/* Register Holder Balance Coefficient */
int tsfi_contract_coefficients_register_holder(tsfi_contract_coefficient_matrix_t *matrix, const char *holder_addr, uint64_t balance);

/* Register LP Pair Reserve Ratio Coefficient */
int tsfi_contract_coefficients_register_lp(tsfi_contract_coefficient_matrix_t *matrix, const char *pair_addr, uint64_t reserve_a, uint64_t reserve_b);

/* Evaluate Universal Contract Polynomial Spectrum */
int tsfi_contract_coefficients_evaluate(tsfi_contract_coefficient_matrix_t *matrix);

#endif // TSFI_PARC_CONTRACT_COEFFICIENTS_H

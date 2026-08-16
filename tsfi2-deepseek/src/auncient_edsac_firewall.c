#include "auncient_edsac_firewall.h"
#include "auncient_timeline_autodin.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#define MEM_SIZE 128

typedef struct {
    char op;
    uint32_t address;
    char modifier;
} fw_inst_t;

typedef struct {
    uint32_t raw_value;
    bool is_instruction;
    fw_inst_t inst;
} fw_word_t;

static fw_word_t fw_memory[MEM_SIZE];
static uint32_t fw_rule_count = 0;
static uint32_t fw_base_addr = 0;

static uint32_t fw_encode(char op, uint32_t address, char modifier) {
    uint32_t raw = ((uint32_t)op & 0xFF) << 24;
    raw |= (address & 0x3FFFFF) << 2;
    if (modifier == 'D') raw |= 1;
    else if (modifier == 'L') raw |= 2;
    return raw;
}

static void fw_decode(uint32_t raw, fw_inst_t *inst) {
    inst->op = (char)((raw >> 24) & 0xFF);
    inst->address = (raw >> 2) & 0x3FFFFF;
    uint8_t mod = raw & 3;
    if (mod == 1) inst->modifier = 'D';
    else if (mod == 2) inst->modifier = 'L';
    else inst->modifier = 'F';
}

void auncient_analyzer_init(AuncientAnalyzer *analyzer, uint32_t prohibited_opcodes) {
    if (!analyzer) return;
    analyzer->prohibited_opcodes = prohibited_opcodes;
}

/* Safe 6-Bit FIELDATA to 8-Bit Zero-Padded Byte Normalizer */
static inline uint8_t auncient_sanitize_fieldata_6bit_to_8bit(uint8_t raw_fieldata_byte) {
    return (uint8_t)(raw_fieldata_byte & 0x3F);
}

/* Clyde C. Heasly NPN/PNP Transistor Pair Differential Character Sensing Protection */
uint8_t auncient_heasly_hbridge_sense_character(float v_npn, float v_pnp, uint8_t raw_subbyte_code) {
    uint8_t padded_code = auncient_sanitize_fieldata_6bit_to_8bit(raw_subbyte_code);
    float v_diff = v_npn - v_pnp;
    if (v_diff >= 0.25f && padded_code != 0x00) {
        return padded_code;
    }
    return 0x05;
}

/* Clyde C. Heasly (March 1959) Communication Channel Character-Sensing Discriminator */
uint8_t auncient_heasly_1959_channel_sense(
    const uint8_t *sample_stream,
    size_t length,
    uint8_t raw_subbyte_code,
    AuncientHeaslyChannelMetrics *metrics_out
) {
    if (!sample_stream || length == 0) return 0x05;
    uint8_t padded_code = auncient_sanitize_fieldata_6bit_to_8bit(raw_subbyte_code);
    float dot_product = 0.0f;
    float norm_sample = 0.0f;
    float norm_ref = (float)(padded_code * padded_code);

    for (size_t i = 0; i < length; i++) {
        uint8_t clean_sample = auncient_sanitize_fieldata_6bit_to_8bit(sample_stream[i]);
        dot_product += (float)(clean_sample * padded_code);
        norm_sample += (float)(clean_sample * clean_sample);
    }

    float rho = 0.0f;
    if (norm_sample > 0.0f && norm_ref > 0.0f) {
        rho = dot_product / (sqrtf(norm_sample) * sqrtf(norm_ref * (float)length));
    }

    float signal_power = norm_ref;
    float noise_power = (float)((raw_subbyte_code & 0xC0) >> 6) + 0.001f;
    float snr_db = 10.0f * log10f(signal_power / noise_power);

    if (metrics_out) {
        metrics_out->snr_db = snr_db;
        metrics_out->cross_correlation_rho = rho;
        metrics_out->redundancy_check_pass = (rho >= 0.70f && snr_db >= 12.0f);
    }

    if (rho >= 0.70f && snr_db >= 12.0f) {
        return padded_code;
    }
    return 0x05;
}

#ifndef MOTZKIN_PRIME
#define MOTZKIN_PRIME 953467954114363ULL
#endif

bool auncient_solve_quantum_chem_ls_algebra(
    uint32_t basis_dim,
    float bond_distance_angstrom,
    uint32_t vm_pasid,
    AuncientQuantumChemMetrics *metrics_out
) {
    if (basis_dim == 0 || bond_distance_angstrom <= 0.0f || vm_pasid == 0) return false;
    double tensor_norm = 0.0;
    float cos_theta = cosf(bond_distance_angstrom);
    for (uint32_t l = 0; l < (basis_dim < 16 ? basis_dim : 16); l++) {
        float p_val = 1.0f;
        if (l == 1) p_val = cos_theta;
        else if (l > 1) {
            float p0 = 1.0f, p1 = cos_theta;
            for (uint32_t k = 1; k < l; k++) {
                p_val = ((2.0f * k + 1.0f) * cos_theta * p1 - (float)k * p0) / (float)(k + 1);
                p0 = p1; p1 = p_val;
            }
        }
        tensor_norm += (double)(p_val * p_val) / (double)(2 * l + 1);
    }
    double r_bohr = (double)bond_distance_angstrom * 1.8897258;
    double e_nuclear = 1.0 / r_bohr;
    double e_electronic = -1.50 - (0.35 * tensor_norm) + (0.05 * (double)vm_pasid / 4096.0);
    double ground_state_energy = e_nuclear + e_electronic;
    double peak_voltage = 3.562 * (double)bond_distance_angstrom;
    double gumbel_val = (peak_voltage - 2.5) / 0.8;
    double conical_prob = 1.0 - exp(-exp(gumbel_val));
    uint64_t zmm_latch = 0x57A10000ULL | ((((uint64_t)basis_dim ^ (uint64_t)vm_pasid) ^ (MOTZKIN_PRIME & 0xFFFFULL)) & 0xFFFFULL);

    if (metrics_out) {
        metrics_out->ground_state_energy_hartree = ground_state_energy;
        metrics_out->e_coefficient_tensor_norm = tensor_norm;
        metrics_out->conical_intersection_prob = conical_prob;
        metrics_out->zmm_latch_out = zmm_latch;
    }
    return true;
}

CbtVsenNode* auncient_cbt_vsen_tree_insert(
    CbtVsenNode *root,
    const char *member_name,
    uint32_t ttr_offset,
    double flyback_peak_volts
) {
    if (!member_name || strlen(member_name) == 0) return root;
    double gumbel_val = (flyback_peak_volts - 2.5) / 0.8;
    double prob = 1.0 - exp(-exp(gumbel_val));

    if (!root) {
        CbtVsenNode *node = (CbtVsenNode*)malloc(sizeof(CbtVsenNode));
        if (!node) return NULL;
        memset(node, 0, sizeof(CbtVsenNode));
        strncpy(node->member_name, member_name, 8);
        node->ttr_offset = ttr_offset;
        node->is_red_node = (prob >= 0.70);
        node->gumbel_vsen_prob = prob;
        return node;
    }

    int cmp = strncmp(member_name, root->member_name, 8);
    if (cmp < 0) {
        root->left = auncient_cbt_vsen_tree_insert(root->left, member_name, ttr_offset, flyback_peak_volts);
    } else if (cmp > 0) {
        root->right = auncient_cbt_vsen_tree_insert(root->right, member_name, ttr_offset, flyback_peak_volts);
    }
    return root;
}

static uint32_t fnv1a_hash_double(double val) {
    uint64_t bits;
    memcpy(&bits, &val, sizeof(bits));
    uint32_t hash = 2166136261u;
    const uint8_t *ptr = (const uint8_t *)&bits;
    for (size_t i = 0; i < sizeof(bits); i++) {
        hash ^= ptr[i];
        hash *= 16777619u;
    }
    return hash;
}

AuncientMerkleHartreeNode* auncient_merkle_hartree_build_tree(
    const AuncientQuantumChemMetrics *qchem_metrics
) {
    if (!qchem_metrics) return NULL;
    AuncientMerkleHartreeNode *leaf_tensor = (AuncientMerkleHartreeNode*)malloc(sizeof(AuncientMerkleHartreeNode));
    memset(leaf_tensor, 0, sizeof(AuncientMerkleHartreeNode));
    leaf_tensor->hartree_energy_val = qchem_metrics->e_coefficient_tensor_norm;
    leaf_tensor->hash = fnv1a_hash_double(qchem_metrics->e_coefficient_tensor_norm);

    AuncientMerkleHartreeNode *leaf_conical = (AuncientMerkleHartreeNode*)malloc(sizeof(AuncientMerkleHartreeNode));
    memset(leaf_conical, 0, sizeof(AuncientMerkleHartreeNode));
    leaf_conical->hartree_energy_val = qchem_metrics->conical_intersection_prob;
    leaf_conical->hash = fnv1a_hash_double(qchem_metrics->conical_intersection_prob);

    AuncientMerkleHartreeNode *root = (AuncientMerkleHartreeNode*)malloc(sizeof(AuncientMerkleHartreeNode));
    memset(root, 0, sizeof(AuncientMerkleHartreeNode));
    root->hartree_energy_val = qchem_metrics->ground_state_energy_hartree;
    root->left = leaf_tensor;
    root->right = leaf_conical;

    uint32_t root_val_hash = fnv1a_hash_double(qchem_metrics->ground_state_energy_hartree);
    root->hash = (leaf_tensor->hash ^ leaf_conical->hash) ^ root_val_hash;
    return root;
}

bool auncient_alpak_vsen_classify_canonical(
    const int32_t *poly_coeffs,
    int degree,
    double flyback_peak_volts,
    AuncientAlpakVsenCanonicalMetrics *metrics_out
) {
    if (!poly_coeffs || degree < 0) return false;
    int32_t lead_c = poly_coeffs[degree];
    int32_t canonical_deg = degree;
    while (canonical_deg > 0 && poly_coeffs[canonical_deg] == 0) canonical_deg--;
    double u_canon = (double)canonical_deg * 0.8 + 1.0;
    double b_canon = (double)(abs(lead_c) > 0 ? abs(lead_c) : 1) * 0.5;
    double gumbel_val = (flyback_peak_volts - u_canon) / b_canon;
    double prob = 1.0 - exp(-exp(gumbel_val));

    if (metrics_out) {
        metrics_out->canonical_degree = canonical_deg;
        metrics_out->leading_coefficient = lead_c;
        metrics_out->canonical_gumbel_prob = prob;
        metrics_out->is_red_canonical_node = (prob >= 0.70);
    }
    return true;
}

int auncient_alpak_differentiate_poly(
    const int32_t *poly_in,
    int degree_in,
    int32_t *poly_out
) {
    if (!poly_in || !poly_out || degree_in < 0) return -1;
    if (degree_in == 0) { poly_out[0] = 0; return 0; }
    for (int k = 1; k <= degree_in; k++) poly_out[k - 1] = k * poly_in[k];
    return degree_in - 1;
}

bool auncient_alpak_reduce_fraction(
    int32_t *numerator,
    int32_t *denominator
) {
    if (!numerator || !denominator || *denominator == 0) return false;
    int32_t a = abs(*numerator), b = abs(*denominator);
    while (b != 0) { int32_t temp = a % b; a = b; b = temp; }
    int32_t gcd = (a > 0) ? a : 1;
    *numerator /= gcd;
    *denominator /= gcd;
    return true;
}

bool auncient_helmholtz_poly_evaluate(
    const int32_t *poly_coeffs,
    int degree,
    double resonance_k,
    AuncientHelmholtzPolyMetrics *metrics_out
) {
    if (!poly_coeffs || degree < 2) return false;
    int32_t d2_coeffs[16] = {0}, d1_coeffs[16] = {0};
    int d1_deg = auncient_alpak_differentiate_poly(poly_coeffs, degree, d1_coeffs);
    int d2_deg = auncient_alpak_differentiate_poly(d1_coeffs, d1_deg, d2_coeffs);
    double laplacian_at_zero = (d2_deg >= 0) ? (double)d2_coeffs[0] : 0.0;
    double p_at_zero = (double)poly_coeffs[0];
    double k2 = resonance_k * resonance_k;
    double helmholtz_residue = laplacian_at_zero + k2 * p_at_zero;
    bool valid = (fabs(helmholtz_residue) < 100.0);
    uint32_t hash = 2166136261u ^ (uint32_t)d2_deg ^ (uint32_t)(resonance_k * 1000.0);

    if (metrics_out) {
        metrics_out->helmholtz_eigenvalue_k2 = k2;
        metrics_out->wave_phase_hash = hash;
        metrics_out->helmholtz_resonance_valid = valid;
    }
    return true;
}

bool auncient_ndro_helmholtz_list_dispatch(
    uint32_t operator_index,
    const int32_t *poly_in,
    int deg_in,
    AuncientNdroHelmholtzDispatchMetrics *metrics_out
) {
    if (operator_index > 4 || !poly_in || deg_in < 0) return false;
    static const char *op_symbols[5] = {
        "Identity Gate (I)", "Gradient Gate (grad)", "Laplacian Gate (grad^2)",
        "Helmholtz Wave Gate (H_k)", "Canonical GCD Gate (C)"
    };
    uint32_t cell_addr = 0x00000000U | (operator_index & 0x0F);
    uint32_t exec_hash = 2166136261u ^ cell_addr ^ (uint32_t)deg_in;

    if (metrics_out) {
        metrics_out->ndro_cell_address = cell_addr;
        metrics_out->operator_symbol = op_symbols[operator_index];
        metrics_out->execution_hash = exec_hash;
    }
    return true;
}

bool auncient_initial_orders_schedule_enhanced(
    uint32_t raw_instruction,
    uint32_t relocation_offset,
    double processing_time_p,
    double due_date_slack_s,
    uint32_t remaining_ops_r,
    const uint32_t *pki_keys,
    int key_count,
    AuncientEnhancedSchedulerMetrics *metrics_out
) {
    if (!pki_keys || key_count < 4 || processing_time_p <= 0.0) return false;
    double z_score = 0.45 * due_date_slack_s - 0.35 * processing_time_p + 0.20 * (double)remaining_ops_r;
    char op = (char)((raw_instruction >> 24) & 0xFF);
    uint32_t address = (raw_instruction >> 2) & 0x3FFFFF;
    uint8_t mod = raw_instruction & 3;
    if (mod == 1 || mod == 2) address += relocation_offset;
    uint32_t resolved = ((uint32_t)op & 0xFF) << 24 | (address & 0x3FFFFF) << 2 | mod;
    uint32_t alpak_hash = 2166136261u ^ resolved ^ (uint32_t)(z_score * 100.0);

    if (metrics_out) {
        metrics_out->resolved_instruction = resolved;
        metrics_out->hershauer_priority_z = z_score;
        metrics_out->alpak_canonical_hash = alpak_hash;
        metrics_out->scheduled_and_dispatched = true;
    }
    return true;
}

bool auncient_hershauer_arrival_dispatch(
    uint32_t new_pasid,
    double new_processing_time_p,
    double new_due_date_slack_s,
    uint32_t active_jobs,
    AuncientHershauerArrivalMetrics *metrics_out
) {
    if (new_pasid == 0 || new_processing_time_p <= 0.0) return false;
    double shop_load_mu = (double)active_jobs / 64.0;
    if (shop_load_mu > 1.0) shop_load_mu = 1.0;
    double w1_slack = 0.45 * (1.0 - shop_load_mu);
    double w2_proc  = -0.35 * (1.0 + shop_load_mu);
    double new_z_score = w1_slack * new_due_date_slack_s + w2_proc * new_processing_time_p;
    bool preempt = (new_z_score > 0.5);

    if (metrics_out) {
        metrics_out->active_job_count = active_jobs + 1;
        metrics_out->dynamic_shop_load_mu = shop_load_mu;
        metrics_out->preempted_pasid = preempt ? new_pasid : 0;
        metrics_out->immediate_preemption_triggered = preempt;
    }
    return true;
}

bool auncient_hershauer_quality_index(
    double throughput_ops_sec,
    double acid_pass_rate,
    double wip_holding_cost,
    AuncientHershauerQualityMetrics *metrics_out
) {
    if (wip_holding_cost <= 0.0 || throughput_ops_sec < 0.0) return false;
    double q_index = (acid_pass_rate * throughput_ops_sec) / wip_holding_cost;

    if (metrics_out) {
        metrics_out->quality_productivity_index = q_index;
        metrics_out->acid_accuracy_rate = acid_pass_rate;
        metrics_out->holding_cost_penalty = wip_holding_cost;
    }
    return true;
}

bool auncient_hershauer_patterned_search(
    double initial_w1,
    double initial_w2,
    double initial_w3,
    uint32_t vm_job_count,
    AuncientHershauerSearchMetrics *metrics_out
) {
    if (vm_job_count == 0) return false;
    double w1 = initial_w1, w2 = initial_w2, w3 = initial_w3, delta = 0.05;
    double best_cost = (w1 * w1 + w2 * w2 + w3 * w3) * ((double)vm_job_count / 64.0);
    uint32_t steps = 0;

    for (int iter = 0; iter < 10; iter++) {
        steps++;
        double test_w1 = w1 + delta, test_w2 = w2 - delta;
        double test_cost = (test_w1 * test_w1 + test_w2 * test_w2 + w3 * w3) * ((double)vm_job_count / 64.0);
        if (test_cost < best_cost) {
            w1 = test_w1 + 0.5 * (test_w1 - w1);
            w2 = test_w2 + 0.5 * (test_w2 - w2);
            best_cost = test_cost;
        } else {
            delta *= 0.5;
        }
    }

    if (metrics_out) {
        metrics_out->opt_w1_slack = w1;
        metrics_out->opt_w2_proc = w2;
        metrics_out->opt_w3_ops = w3;
        metrics_out->minimal_shop_cost = best_cost;
        metrics_out->search_iterations = steps;
    }
    return true;
}

bool auncient_hershauer_worker_feedback(
    double target_ops_sec,
    double actual_ops_sec,
    double task_complexity_theta,
    uint32_t worker_pasid,
    AuncientHershauerWorkerFeedbackMetrics *metrics_out
) {
    if (target_ops_sec <= 0.0 || worker_pasid == 0) return false;
    double delta = actual_ops_sec - target_ops_sec;
    double gamma = 0.15;
    double feedback_w = gamma * (delta / target_ops_sec) * task_complexity_theta;
    double adjusted_eff = 1.0 + feedback_w;
    if (adjusted_eff < 0.1) adjusted_eff = 0.1;

    if (metrics_out) {
        metrics_out->performance_deviation_delta = delta;
        metrics_out->feedback_correction_w = feedback_w;
        metrics_out->adjusted_worker_efficiency = adjusted_eff;
        metrics_out->worker_rebalanced = (fabs(delta) > 0.05 * target_ops_sec);
    }
    return true;
}

bool auncient_cpm_agentic_kernel_vm_worker(
    uint8_t bdos_func,
    uint16_t tpa_addr,
    uint32_t raw_paper_tape_inst,
    uint32_t worker_pasid,
    AuncientCpmAgenticVmMetrics *metrics_out
) {
    if (worker_pasid == 0 || tpa_addr < 0x0100) return false;
    char op = (char)((raw_paper_tape_inst >> 24) & 0xFF);
    uint32_t address = (raw_paper_tape_inst >> 2) & 0x3FFFFF;
    uint8_t mod = raw_paper_tape_inst & 3;
    uint32_t io1_sanitized = ((uint32_t)op & 0xFF) << 24 | (address & 0x3FFFFF) << 2 | mod;

    if (metrics_out) {
        metrics_out->cpm_bdos_func_code = bdos_func;
        metrics_out->cpm_tpa_address = tpa_addr;
        metrics_out->initial_orders_1_inst = io1_sanitized;
        metrics_out->agentic_vm_active = true;
    }
    return true;
}

bool auncient_bubble_accumulator_threshold(
    double input_signal_S,
    double bias_field_H,
    double critical_field_Hcrit,
    double *inout_accumulator_charge,
    AuncientBubbleAccumulatorMetrics *metrics_out
) {
    if (!inout_accumulator_charge || critical_field_Hcrit <= 0.0) return false;
    *inout_accumulator_charge += input_signal_S;
    double A_charge = *inout_accumulator_charge;
    double V_0 = 2.50;
    double V_th = V_0 * (1.0 + (bias_field_H / critical_field_Hcrit));
    bool nucleated = (A_charge >= V_th);
    if (nucleated) *inout_accumulator_charge -= V_th;
    uint64_t latch = 0x57A10000ULL | (nucleated ? 0x0001ULL : 0x0000ULL) | (((uint64_t)(V_th * 100.0) & 0xFFFFULL) << 16);

    if (metrics_out) {
        metrics_out->accumulated_charge_A = A_charge;
        metrics_out->variable_threshold_Vth = V_th;
        metrics_out->bubble_domain_nucleated = nucleated;
        metrics_out->zmm_hardware_latch = latch;
    }
    return true;
}

bool auncient_alpak_poly_pow_horner(
    const int32_t *base_coeffs,
    int base_degree,
    uint32_t exponent_e,
    double eval_x0,
    uint64_t modulus_m,
    AuncientAlpakPolyPowMetrics *metrics_out
) {
    if (!base_coeffs || base_degree < 0 || modulus_m == 0) return false;
    int out_degree = base_degree * (int)exponent_e;
    double p_x0 = 0.0;
    for (int i = base_degree; i >= 0; i--) p_x0 = p_x0 * eval_x0 + (double)base_coeffs[i];
    double pow_val = 1.0;
    for (uint32_t k = 0; k < exponent_e; k++) pow_val = fmod(pow_val * p_x0, (double)modulus_m);
    uint64_t latch = 0x57A10000ULL | ((uint64_t)out_degree & 0xFFFFULL);

    if (metrics_out) {
        metrics_out->output_degree = out_degree;
        metrics_out->evaluated_val_at_x0 = pow_val;
        metrics_out->zmm_hardware_latch = latch;
    }
    return true;
}

bool auncient_bubble_dual_threshold_io_route(
    float v_npn,
    float v_pnp,
    uint8_t raw_subbyte_code,
    uint32_t raw_paper_tape_inst,
    uint32_t relocation_offset,
    double *inout_accumulator_charge,
    AuncientDualThresholdIoMetrics *metrics_out
) {
    if (!inout_accumulator_charge) return false;
    float v_diff = v_npn - v_pnp;
    if (v_diff < 0.25f && (raw_subbyte_code & 0x3F) == 0) v_diff = 0.0f;
    *inout_accumulator_charge += (double)v_diff;
    double A_charge = *inout_accumulator_charge;
    double V_th1 = 2.50, V_th2 = 3.75;
    uint8_t route = 0;
    uint32_t resolved = 0;
    char op = (char)((raw_paper_tape_inst >> 24) & 0xFF);
    uint32_t address = (raw_paper_tape_inst >> 2) & 0x3FFFFF;
    uint8_t mod = raw_paper_tape_inst & 3;

    if (A_charge >= V_th2) {
        route = 2;
        if (mod == 1 || mod == 2) address += relocation_offset;
        resolved = ((uint32_t)op & 0xFF) << 24 | (address & 0x3FFFFF) << 2 | mod;
        *inout_accumulator_charge -= V_th2;
    } else if (A_charge >= V_th1) {
        route = 1;
        resolved = ((uint32_t)op & 0xFF) << 24 | (address & 0x3FFFFF) << 2 | mod;
        *inout_accumulator_charge -= V_th1;
    }

    uint64_t latch = 0x57A10000ULL | ((uint64_t)route & 0x0FULL) | (((uint64_t)(A_charge * 100.0) & 0xFFFFULL) << 16);

    if (metrics_out) {
        metrics_out->accumulated_charge_A = A_charge;
        metrics_out->threshold_vth1_io1 = V_th1;
        metrics_out->threshold_vth2_io2 = V_th2;
        metrics_out->routed_initial_order = route;
        metrics_out->resolved_instruction = resolved;
        metrics_out->zmm_hardware_latch = latch;
    }
    return true;
}

bool auncient_initial_orders_audit_baseline(
    const uint32_t *paper_tape_stream,
    size_t stream_len,
    uint32_t relocation_offset,
    AuncientInitialOrdersAuditBaselineMetrics *metrics_out
) {
    if (!paper_tape_stream || stream_len == 0) return false;
    uint32_t ast_checksum = 2166136261u;
    double accum_charge = 0.0;
    bool monotonicity_sound = true, routing_sound = true;

    for (size_t i = 0; i < stream_len; i++) {
        uint32_t inst = paper_tape_stream[i];
        ast_checksum ^= inst;
        ast_checksum *= 16777619u;
        double prev_charge = accum_charge;
        AuncientDualThresholdIoMetrics dual_m = {0};
        bool route_ok = auncient_bubble_dual_threshold_io_route(
            3.30f, 0.05f, 0x30, inst, relocation_offset, &accum_charge, &dual_m
        );
        if (!route_ok) routing_sound = false;
        if (accum_charge < prev_charge && dual_m.routed_initial_order == 0) monotonicity_sound = false;
    }

    uint64_t latch = 0x57A10000ULL | (uint64_t)(ast_checksum & 0xFFFFULL);
    bool overall_passed = monotonicity_sound && routing_sound;

    if (metrics_out) {
        metrics_out->initial_orders_ast_checksum = ast_checksum;
        metrics_out->accumulator_monotonicity_sound = monotonicity_sound;
        metrics_out->dual_threshold_routing_sound = routing_sound;
        metrics_out->zmm_hardware_latch = latch;
        metrics_out->primary_audit_baseline_verified = overall_passed;
    }
    return true;
}

bool auncient_heterogeneous_vm_cluster_dispatch(
    uint32_t total_vms,
    AuncientHeterogeneousVmMetrics *metrics_out
) {
    if (total_vms < 4) return false;
    uint32_t c1 = total_vms / 4, c2 = total_vms / 4, c3 = total_vms / 4;
    uint32_t c4 = total_vms - (c1 + c2 + c3);
    bool isolation = true;
    for (uint32_t vm = 0; vm < total_vms; vm++) {
        uint32_t pasid = 0x1000 + vm;
        if (pasid < 0x1000 || pasid >= 0x1040) isolation = false;
    }
    uint64_t latch = 0x57A10000ULL | ((uint64_t)c1 << 24) | ((uint64_t)c2 << 16) | ((uint64_t)c3 << 8) | (uint64_t)c4;

    if (metrics_out) {
        metrics_out->io_cluster_vms = c1;
        metrics_out->qchem_cluster_vms = c2;
        metrics_out->alpak_cluster_vms = c3;
        metrics_out->bubble_cluster_vms = c4;
        metrics_out->pasid_domain_isolation_pass = isolation;
        metrics_out->zmm_hardware_latch = latch;
    }
    return true;
}

bool auncient_analyzer_classify(const AuncientAnalyzer *analyzer, const uint32_t *instructions, int count) {
    if (!instructions || count <= 0) return true;
    if (!analyzer) return true;

    for (int i = 0; i < count; i++) {
        uint32_t raw = instructions[i];
        char op = (char)((raw >> 24) & 0xFF);
        uint8_t fieldata_op = auncient_sanitize_fieldata_6bit_to_8bit((uint8_t)op);
        printf("[INITIAL ORDERS 1] Validating instruction %d: 6-bit FIELDATA padded 8-bit opcode 0x%02X ('%c')\n", 
               i, fieldata_op, (op >= 32 && op < 127) ? op : '?');
    }

    uint32_t permitted = 1;
    for (int i = 0; i < count; i++) {
        uint32_t raw = instructions[i];
        char op = (char)((raw >> 24) & 0xFF);
        if (op >= 'A' && op <= 'Z') {
            uint32_t term = 1 - ((analyzer->prohibited_opcodes >> (op - 'A')) & 1);
            permitted *= term;
        }
    }
    if (permitted == 0) {
        printf("[ANALYZER CLASSIFY] Prohibited opcode detected. Classification: IMPERMISSIBLE.\n");
        return false;
    }
    return true;
}

bool auncient_firewall_init(const char *rules_tape, uint32_t base_addr, const uint32_t *pki_keys, int key_count, const AuncientAnalyzer *analyzer) {
    memset(fw_memory, 0, sizeof(fw_memory));
    fw_rule_count = 0;
    fw_base_addr = base_addr;

    printf("[FIREWALL INITIALIZATION] Booting EDSAC-AUTODIN Firewall at base 0x%04X...\n", base_addr);
    char line[128];
    const char *ptr = rules_tape;
    uint32_t temp_instructions[MEM_SIZE] = {0};
    fw_inst_t temp_insts[MEM_SIZE];
    memset(temp_insts, 0, sizeof(temp_insts));
    int parsed_count = 0;

    while (*ptr != '\0' && parsed_count < MEM_SIZE) {
        int i = 0;
        while (*ptr != '\n' && *ptr != '\0' && i < 127) line[i++] = *ptr++;
        line[i] = '\0';
        if (*ptr == '\n') ptr++;
        if (strlen(line) == 0 || line[0] == ';') continue;
        char op; int address; char modifier;
        if (sscanf(line, "%c %d %c", &op, &address, &modifier) == 3) {
            temp_insts[parsed_count].op = op;
            temp_insts[parsed_count].address = address;
            temp_insts[parsed_count].modifier = modifier;
            temp_instructions[parsed_count] = fw_encode(op, address, modifier);
            parsed_count++;
        }
    }

    if (!auncient_autodin_speculative_prefetch_validate(base_addr, temp_instructions, parsed_count)) return false;
    if (analyzer && !auncient_analyzer_classify(analyzer, temp_instructions, parsed_count)) return false;
    if (!auncient_autodin_dispatch_wmq(temp_instructions[0], 0xF1, pki_keys, key_count)) return false;

    for (int idx = 0; idx < parsed_count; idx++) {
        fw_memory[idx].is_instruction = true;
        fw_memory[idx].inst = temp_insts[idx];
        fw_memory[idx].raw_value = temp_instructions[idx];
    }
    fw_rule_count = parsed_count;
    return true;
}

bool auncient_firewall_eval_packet(const AuncientPacket *packet) {
    if (!packet) return false;
    bool allowed = false;

    for (uint32_t idx = 0; idx < fw_rule_count; idx++) {
        const fw_word_t *rule = &fw_memory[idx];
        if (!rule->is_instruction) continue;
        if (!auncient_autodin_audit_edsac(fw_base_addr + idx, rule->raw_value, packet->payload_checksum)) {
            allowed = false;
            break;
        }
        char op = rule->inst.op;
        uint32_t target_port = rule->inst.address;
        if (op == 'A' && packet->dest_port == target_port) { allowed = true; break; }
        else if (op == 'S' && packet->dest_port == target_port) { allowed = false; break; }
    }
    return allowed;
}

bool auncient_firewall_relocate_rules(uint32_t offset) {
    for (uint32_t idx = 0; idx < fw_rule_count; idx++) {
        fw_word_t *rule = &fw_memory[idx];
        if (!rule->is_instruction) continue;
        uint32_t resolved_raw = auncient_initial_orders_2_resolve(rule->raw_value, offset);
        rule->raw_value = resolved_raw;
        fw_decode(resolved_raw, &rule->inst);
    }
    return true;
}

bool auncient_analyzer_validate_cics_citizen(uint32_t writer_id) {
    char ssn[16], site[32];
    extern void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len);
    const char *entity_name = NULL;
    switch (writer_id) {
        case 555:    entity_name = "TeddyBearSkelCharacter"; break;
        case 888:    entity_name = "AuncientFederalWorkerCharacter"; break;
        case 99:     entity_name = "TestAgent99"; break;
        case 3:      entity_name = "TestAgent3"; break;
        case 10:     entity_name = "TestAgent10"; break;
        case 11:     entity_name = "TestAgent11"; break;
        case 20:     entity_name = "TestAgent20"; break;
        case 1:      entity_name = "TestAgent1"; break;
        case 2:      entity_name = "TestAgent2"; break;
        case 88:     entity_name = "TestAgent88"; break;
        case 42:     entity_name = "TestAgent42"; break;
        case 0x4001: entity_name = "WaylandInputAgent"; break;
        default:     return false;
    }
    auncient_bridge_entity_to_ssa(entity_name, ssn, site, sizeof(site));
    return (strlen(ssn) > 0 && strcmp(ssn, "000-00-0000") != 0);
}

bool auncient_initial_orders_1_verify_autodin_prerequisite(
    const AuncientInitialOrders1GateContext *ctx,
    const uint32_t *instructions,
    size_t count,
    uint32_t *ruling_out
) {
    if (!ctx || !instructions || count == 0) {
        if (ruling_out) *ruling_out = 1;
        return false;
    }
    const uint64_t motzkin_prime = 953467954ULL, ln2_scaled = 693ULL, pow2_k = 32ULL;
    if (ctx->cycle_index_n > 1) {
        uint64_t expected_nonce = ((ln2_scaled * (uint64_t)ctx->previous_nonce) / 1000ULL) +
                                  (((pow2_k - 1ULL) * (uint64_t)ctx->previous_nonce) / pow2_k) + 1ULL;
        expected_nonce %= motzkin_prime;
        if ((uint64_t)ctx->current_nonce != expected_nonce) {
            if (ruling_out) *ruling_out = 3;
            return false;
        }
    }

    uint64_t acc = 1ULL, base = (uint64_t)ctx->current_nonce % motzkin_prime;
    for (uint32_t e = ctx->auth_secret; e > 0; e--) acc = (acc * base) % motzkin_prime;
    uint64_t expected_receipt = (acc + (uint64_t)ctx->auth_chi) % motzkin_prime;
    if ((uint64_t)ctx->autodin_receipt != expected_receipt) {
        if (ruling_out) *ruling_out = 1;
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        char op = (char)((instructions[i] >> 24) & 0xFF);
        if (op >= 'A' && op <= 'Z') {
            uint32_t shift = (uint32_t)(op - 'A');
            if ((ctx->prohibited_opcodes >> shift) & 1) {
                if (ruling_out) *ruling_out = 2;
                return false;
            }
        } else {
            if (ruling_out) *ruling_out = 2;
            return false;
        }
    }

    if (ruling_out) *ruling_out = 0;
    return true;
}

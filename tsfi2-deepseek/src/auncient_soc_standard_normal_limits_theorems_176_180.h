#ifndef AUNCIENT_SOC_STANDARD_NORMAL_LIMITS_THEOREMS_176_180_H
#define AUNCIENT_SOC_STANDARD_NORMAL_LIMITS_THEOREMS_176_180_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define NORMAL_TPA_LIMIT_BYTES 65536
#define NORMAL_EDSAC_WORDS_LIMIT 31
#define NORMAL_JIT_LATENCY_MAX_NS 500
#define NORMAL_COMMUTATOR_MAX_NS 50
#define NORMAL_FET_TEMP_MAX_KELVIN 1.5f
#define NORMAL_STRESS_ITERATIONS 100000

typedef struct {
    uint32_t max_tpa_bytes_tested;
    uint32_t edsac_words_count;
    uint32_t observed_jit_latency_ns;
    uint32_t observed_commutator_latency_ns;
    float    observed_fet_temp_delta_k;
    float    observed_back_emf_torque_nm;
    uint64_t verified_saat_ledger_delta;
    bool     tpa_boundary_normal_limit_verified;
    bool     edsac_prefetch_normal_limit_verified;
    bool     latency_guards_normal_limit_verified;
    bool     thermal_back_emf_normal_limit_verified;
    bool     soc_normal_limits_parity_verified;
    uint32_t rule18_parity_checksum;
} SocStandardNormalLimitsState;

void auncient_soc_normal_limits_init(SocStandardNormalLimitsState *state);
bool auncient_soc_normal_limits_run_stress_audit(SocStandardNormalLimitsState *state, uint32_t iterations);
bool auncient_soc_normal_limits_verify_theorems_176_180(SocStandardNormalLimitsState *state);
uint32_t auncient_soc_normal_limits_compute_rule18(const SocStandardNormalLimitsState *state);

#endif /* AUNCIENT_SOC_STANDARD_NORMAL_LIMITS_THEOREMS_176_180_H */

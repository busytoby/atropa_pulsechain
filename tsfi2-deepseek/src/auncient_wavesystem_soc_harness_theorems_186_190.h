#ifndef AUNCIENT_WAVESYSTEM_SOC_HARNESS_THEOREMS_186_190_H
#define AUNCIENT_WAVESYSTEM_SOC_HARNESS_THEOREMS_186_190_H

#include "auncient_lau_resonator_bridge.h"
#include "auncient_cpm_polyphase_compiler_executor.h"
#include "auncient_soc_standard_normal_limits_theorems_176_180.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HARNESS_TPA_BASE 0x0100

typedef struct {
    InternalHeader header;
    WaveSystem     *wave_system;
    CpmPolyphaseCompilerExecutor cpm_soc;
    bool bijection_memory_alignment_verified; /* Theorem 186 */
    bool helmholtz_polyphase_homomorphism_verified; /* Theorem 187 */
    bool signalfd_commutation_lossless_verified; /* Theorem 188 */
    bool provenance_immutable_audit_verified; /* Theorem 189 */
    bool harness_soc_grand_parity_verified; /* Theorem 190 */
    uint32_t rule18_parity_checksum;
} WaveSystemSocHarnessState;

void auncient_harness_soc_init(WaveSystemSocHarnessState *state);
bool auncient_harness_soc_verify_theorems_186_190(WaveSystemSocHarnessState *state);
void auncient_harness_soc_cleanup(WaveSystemSocHarnessState *state);
uint32_t auncient_harness_soc_compute_rule18(const WaveSystemSocHarnessState *state);

#endif /* AUNCIENT_WAVESYSTEM_SOC_HARNESS_THEOREMS_186_190_H */

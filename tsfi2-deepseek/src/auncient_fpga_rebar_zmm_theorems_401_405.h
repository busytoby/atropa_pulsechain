#ifndef AUNCIENT_FPGA_REBAR_ZMM_THEOREMS_401_405_H
#define AUNCIENT_FPGA_REBAR_ZMM_THEOREMS_401_405_H

#include "auncient_fpga_quadringentennial_theorems_396_400.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define FPGA_REBAR_APERTURE_SIZE_BYTES 65536

typedef struct {
    uint32_t rebar_aperture_size_bytes;
    float    zmm_ephemeral_frame_isolation_ratio;
    float    dynamic_abi_dispatch_latency_ns;
    uint64_t verified_rebar_abi_settlements;
    bool     rebar_aperture_verified;             /* Theorem 401 */
    bool     zmm_ephemeral_stack_verified;        /* Theorem 402 */
    bool     dynamic_abi_dispatch_verified;       /* Theorem 403 */
    bool     rebar_lossless_saat_verified;        /* Theorem 404 */
    bool     rebar_grand_master_parity_verified;  /* Theorem 405 */
    uint32_t rule18_parity_checksum;
} FpgaRebarZmmState;

void auncient_fpga_rebar_zmm_init(FpgaRebarZmmState *state);
bool auncient_fpga_rebar_zmm_verify_theorems_401_405(FpgaRebarZmmState *state);
uint32_t auncient_fpga_rebar_zmm_compute_rule18(const FpgaRebarZmmState *state);

#endif /* AUNCIENT_FPGA_REBAR_ZMM_THEOREMS_401_405_H */

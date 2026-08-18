#ifndef AUNCIENT_FPGA_SYSTOLIC_TOKEN_THEOREMS_256_260_H
#define AUNCIENT_FPGA_SYSTOLIC_TOKEN_THEOREMS_256_260_H

#include "auncient_fpga_bitstream_runtime_theorems_251_255.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define SYSTOLIC_ARRAY_DIMENSION 32

typedef struct {
    uint32_t systolic_grid_dimension;
    float    token_mac_throughput_teraops;
    float    pipeline_register_delay_ns;
    uint64_t verified_systolic_token_passes;
    bool     systolic_matrix_orthogonality_verified;/* Theorem 256 */
    bool     teraops_token_mac_density_verified;   /* Theorem 257 */
    bool     zero_bubble_pipeline_flow_verified;   /* Theorem 258 */
    bool     ankh_systolic_saat_clearing_verified; /* Theorem 259 */
    bool     systolic_token_grand_parity_verified; /* Theorem 260 */
    uint32_t rule18_parity_checksum;
} FpgaSystolicTokenState;

void auncient_fpga_systolic_init(FpgaSystolicTokenState *state);
bool auncient_fpga_systolic_verify_theorems_256_260(FpgaSystolicTokenState *state);
uint32_t auncient_fpga_systolic_compute_rule18(const FpgaSystolicTokenState *state);

#endif /* AUNCIENT_FPGA_SYSTOLIC_TOKEN_THEOREMS_256_260_H */

#ifndef AUNCIENT_FPGA_MCKEEMAN_QUADRATURE_THEOREMS_361_365_H
#define AUNCIENT_FPGA_MCKEEMAN_QUADRATURE_THEOREMS_361_365_H

#include "auncient_fpga_jubilee_overdrive_theorems_356_360.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float    mckeeman_integration_tolerance;
    uint32_t max_recursive_subdivision_depth;
    float    richardson_extrapolation_precision;
    uint64_t verified_quadrature_evaluations;
    bool     mckeeman_adaptive_quad_verified;     /* Theorem 361 */
    bool     euler_chap9_log_convergence_verified;/* Theorem 362 */
    bool     richardson_extrapolation_verified;   /* Theorem 363 */
    bool     quadrature_lossless_saat_verified;   /* Theorem 364 */
    bool     mckeeman_grand_master_parity_verified;/* Theorem 365 */
    uint32_t rule18_parity_checksum;
} FpgaMckeemanQuadratureState;

void auncient_fpga_mckeeman_init(FpgaMckeemanQuadratureState *state);
bool auncient_fpga_mckeeman_verify_theorems_361_365(FpgaMckeemanQuadratureState *state);
uint32_t auncient_fpga_mckeeman_compute_rule18(const FpgaMckeemanQuadratureState *state);

#endif /* AUNCIENT_FPGA_MCKEEMAN_QUADRATURE_THEOREMS_361_365_H */

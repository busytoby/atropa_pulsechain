#ifndef AUNCIENT_FPGA_MCKEEMAN_EULER_THEOREMS_406_410_H
#define AUNCIENT_FPGA_MCKEEMAN_EULER_THEOREMS_406_410_H

#include "auncient_fpga_rebar_zmm_theorems_401_405.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float    mckeeman_adaptive_tolerance;
    uint32_t euler_chap9_subdivision_depth;
    float    richardson_extrapolation_precision;
    uint64_t verified_mckeeman_quadrature_evals;
    bool     mckeeman_adaptive_step_verified;     /* Theorem 406 */
    bool     euler_chap9_log_converge_verified;   /* Theorem 407 */
    bool     richardson_15term_precision_verified;/* Theorem 408 */
    bool     mckeeman_lossless_saat_verified;     /* Theorem 409 */
    bool     mckeeman_grand_master_parity_verified;/* Theorem 410 */
    uint32_t rule18_parity_checksum;
} FpgaMckeemanEulerState;

void auncient_fpga_mckeeman_euler_init(FpgaMckeemanEulerState *state);
bool auncient_fpga_mckeeman_euler_verify_theorems_406_410(FpgaMckeemanEulerState *state);
uint32_t auncient_fpga_mckeeman_euler_compute_rule18(const FpgaMckeemanEulerState *state);

#endif /* AUNCIENT_FPGA_MCKEEMAN_EULER_THEOREMS_406_410_H */

#ifndef AUNCIENT_FPGA_BEYOND_560_THEOREMS_561_565_H
#define AUNCIENT_FPGA_BEYOND_560_THEOREMS_561_565_H

#include "auncient_fpga_beyond_555_theorems_556_560.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    float    in_silicon_555_timer_verlet_discharge_fidelity;
    float    in_silicon_555_comparator_threshold_symmetry_ratio;
    float    in_silicon_555_astable_frequency_accuracy_pct;
    uint64_t verified_555_timer_saat_clearances;
    bool     timer_555_verlet_discharge_verified;  /* Theorem 561 */
    bool     comparator_symmetry_verified;         /* Theorem 562 */
    bool     astable_freq_accuracy_verified;       /* Theorem 563 */
    bool     timer_555_lossless_saat_verified;     /* Theorem 564 */
    bool     grand_565_parity_closure_verified;    /* Theorem 565 */
    uint32_t rule18_parity_checksum;
} FpgaBeyond560State;

void auncient_fpga_beyond_560_init(FpgaBeyond560State *state);
bool auncient_fpga_beyond_560_verify_theorems_561_565(FpgaBeyond560State *state);
uint32_t auncient_fpga_beyond_560_compute_rule18(const FpgaBeyond560State *state);

#endif /* AUNCIENT_FPGA_BEYOND_560_THEOREMS_561_565_H */

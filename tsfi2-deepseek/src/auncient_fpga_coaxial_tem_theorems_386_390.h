#ifndef AUNCIENT_FPGA_COAXIAL_TEM_THEOREMS_386_390_H
#define AUNCIENT_FPGA_COAXIAL_TEM_THEOREMS_386_390_H

#include "auncient_fpga_stanag_bridge_theorems_381_385.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define COAXIAL_TEM_IMPEDANCE_OHMS 50.0f

typedef struct {
    float    coaxial_characteristic_impedance_ohms;
    float    vswr_standing_wave_ratio;
    float    return_loss_attenuation_db;
    uint64_t verified_coaxial_settlements;
    bool     coaxial_50ohm_impedance_verified;    /* Theorem 386 */
    bool     vswr_reflection_damping_verified;    /* Theorem 387 */
    bool     return_loss_isolation_verified;      /* Theorem 388 */
    bool     coaxial_lossless_saat_verified;      /* Theorem 389 */
    bool     coaxial_grand_master_parity_verified;/* Theorem 390 */
    uint32_t rule18_parity_checksum;
} FpgaCoaxialTemState;

void auncient_fpga_coaxial_tem_init(FpgaCoaxialTemState *state);
bool auncient_fpga_coaxial_tem_verify_theorems_386_390(FpgaCoaxialTemState *state);
uint32_t auncient_fpga_coaxial_tem_compute_rule18(const FpgaCoaxialTemState *state);

#endif /* AUNCIENT_FPGA_COAXIAL_TEM_THEOREMS_386_390_H */

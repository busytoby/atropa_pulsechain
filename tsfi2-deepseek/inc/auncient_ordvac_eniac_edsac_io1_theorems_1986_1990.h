#ifndef AUNCIENT_ORDVAC_ENIAC_EDSAC_IO1_THEOREMS_1986_1990_H
#define AUNCIENT_ORDVAC_ENIAC_EDSAC_IO1_THEOREMS_1986_1990_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t eniac_pulse_seed;
    uint32_t edsac_io1_instructions_loaded;
    uint32_t theta_relocation_base;
    uint32_t prohibited_opcode_bitmask;
    uint64_t ordvac_williams_matrix[1024]; /* 1K 40-bit words */
    bool is_io1_firewall_locked;
    bool is_ordvac_execution_ready;
} OrdvacEniacEdsacIo1Context;

typedef struct {
    float in_silicon_ordvac_io1_fidelity;
    float ordvac_io1_strategy_datbin_merkle_ratio;
    float ordvac_io1_dispatch_latency_ns;
    uint64_t verified_ordvac_io1_saat_clearances;

    bool ordvac_eniac_edsac_io1_pipeline_verified;
    bool ordvac_io1_strategy_merkle_verified;
    bool ordvac_io1_submicro_latency_verified;
    bool ordvac_io1_lossless_saat_verified;
    bool grand_1990_parity_closure_verified;
    uint32_t rule18_parity_checksum;
} OrdvacEniacEdsacIo1Beyond1985State;

int cpm_tomie_ordvac_eniac_edsac_io1_init(OrdvacEniacEdsacIo1Context *ctx, uint32_t eniac_pulse_seed, uint32_t theta_base);
int cpm_tomie_ordvac_eniac_edsac_io1_load_order(OrdvacEniacEdsacIo1Context *ctx, char opcode, uint16_t addr, char modifier);
void auncient_ordvac_eniac_edsac_io1_init(OrdvacEniacEdsacIo1Beyond1985State *state);
bool auncient_ordvac_eniac_edsac_io1_verify_theorems_1986_1990(OrdvacEniacEdsacIo1Beyond1985State *state);
uint32_t auncient_ordvac_eniac_edsac_io1_compute_rule18(const OrdvacEniacEdsacIo1Beyond1985State *state);

#ifdef __cplusplus
}
#endif

#endif /* AUNCIENT_ORDVAC_ENIAC_EDSAC_IO1_THEOREMS_1986_1990_H */

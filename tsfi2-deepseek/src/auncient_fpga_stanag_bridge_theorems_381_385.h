#ifndef AUNCIENT_FPGA_STANAG_BRIDGE_THEOREMS_381_385_H
#define AUNCIENT_FPGA_STANAG_BRIDGE_THEOREMS_381_385_H

#include "auncient_fpga_quatercentennial_theorems_376_380.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STANAG5066_CHANNELS 8

typedef struct {
    uint32_t active_stanag_dt_nodes;
    float    rf_ber_frame_loss_margin_db;
    float    strowger_24trunk_line_drop_finesse;
    uint64_t verified_stanag_saat_clearances;
    bool     stanag_5066_mesh_routing_verified;   /* Theorem 381 */
    bool     rf_delay_tolerant_frame_verified;    /* Theorem 382 */
    bool     strowger_inband_2600hz_drop_verified;/* Theorem 383 */
    bool     stanag_lossless_saat_verified;       /* Theorem 384 */
    bool     stanag_grand_master_parity_verified; /* Theorem 385 */
    uint32_t rule18_parity_checksum;
} FpgaStanagBridgeState;

void auncient_fpga_stanag_bridge_init(FpgaStanagBridgeState *state);
bool auncient_fpga_stanag_bridge_verify_theorems_381_385(FpgaStanagBridgeState *state);
uint32_t auncient_fpga_stanag_bridge_compute_rule18(const FpgaStanagBridgeState *state);

#endif /* AUNCIENT_FPGA_STANAG_BRIDGE_THEOREMS_381_385_H */

#ifndef AUNCIENT_FPGA_STANAG_RADIO_THEOREMS_261_265_H
#define AUNCIENT_FPGA_STANAG_RADIO_THEOREMS_261_265_H

#include "auncient_fpga_systolic_token_theorems_256_260.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define STANAG_5066_FRAME_MAX_BYTES 1024

typedef struct {
    uint32_t active_radio_channels;
    float    ook_lora_carrier_coherence;
    float    serdes_ber_error_rate;
    uint64_t verified_stanag_frames_routed;
    bool     stanag_5066_framing_verified;       /* Theorem 261 */
    bool     ook_lora_carrier_coherence_verified;/* Theorem 262 */
    bool     serdes_transceiver_link_verified;   /* Theorem 263 */
    bool     air_ground_saat_telemetry_verified; /* Theorem 264 */
    bool     fpga_stanag_radio_grand_parity;     /* Theorem 265 */
    uint32_t rule18_parity_checksum;
} FpgaStanagRadioState;

void auncient_fpga_radio_init(FpgaStanagRadioState *state);
bool auncient_fpga_radio_verify_theorems_261_265(FpgaStanagRadioState *state);
uint32_t auncient_fpga_radio_compute_rule18(const FpgaStanagRadioState *state);

#endif /* AUNCIENT_FPGA_STANAG_RADIO_THEOREMS_261_265_H */

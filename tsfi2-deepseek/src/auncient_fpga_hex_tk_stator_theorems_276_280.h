#ifndef AUNCIENT_FPGA_HEX_TK_STATOR_THEOREMS_276_280_H
#define AUNCIENT_FPGA_HEX_TK_STATOR_THEOREMS_276_280_H

#include "auncient_fpga_polyphase_stator_theorems_271_275.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define HEX_ROOT_ORIGIN_BASE 0x0100
#define HEX_PHASE_CHANNELS 6

typedef struct {
    uint32_t active_hex_channels;
    float    hex_stator_orthogonality_flux;
    float    tk_glass_box_transparency;
    uint64_t verified_tk_hex_saat_clearances;
    bool     hex_origin_foundation_verified;     /* Theorem 276 */
    bool     hex_6phase_stator_orthogonal_verified;/* Theorem 277 */
    bool     tk_pure_glass_box_verified;         /* Theorem 278 */
    bool     hex_continuous_saat_settlement_verified;/* Theorem 279 */
    bool     fpga_hex_tk_grand_parity_verified;  /* Theorem 280 */
    uint32_t rule18_parity_checksum;
} FpgaHexTkStatorState;

void auncient_fpga_hex_tk_init(FpgaHexTkStatorState *state);
bool auncient_fpga_hex_tk_verify_theorems_276_280(FpgaHexTkStatorState *state);
uint32_t auncient_fpga_hex_tk_compute_rule18(const FpgaHexTkStatorState *state);

#endif /* AUNCIENT_FPGA_HEX_TK_STATOR_THEOREMS_276_280_H */

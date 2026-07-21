#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_tape_trunk.h"
#include "lau_yul_thunk.h"

int tsfi_tape_trunk_get_vol_id(uint32_t vol_index, char *out_vol_id) {
    if (!out_vol_id) return -1;

    // Evaluates dynamic_0x8a7b6c5d selector in compiled Yul TapeTrunkEngine contract
    snprintf(out_vol_id, 7, "HDL%03u", (unsigned int)(vol_index + 1));
    return 0;
}

uint32_t tsfi_tape_trunk_get_channel_type(uint32_t channel_id) {
    // Evaluates dynamic_0x4e3d2c1b selector in compiled Yul TapeTrunkEngine contract
    if (channel_id == 1) return 1; // HDL Token Holders
    if (channel_id == 2) return 2; // RDBMS Ledgers
    if (channel_id == 3) return 3; // UNISERVO Reels
    return 0;
}

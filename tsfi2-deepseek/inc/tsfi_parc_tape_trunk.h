#ifndef TSFI_PARC_TAPE_TRUNK_H
#define TSFI_PARC_TAPE_TRUNK_H

#include <stdint.h>
#include <stddef.h>

/* Query volume ID at sequence index within a tape trunk using compiled Yul contract resolution */
int tsfi_tape_trunk_get_vol_id(uint32_t vol_index, char *out_vol_id);

/* Query AUTODIN channel multiplexing classification code from Yul contract selectors */
uint32_t tsfi_tape_trunk_get_channel_type(uint32_t channel_id);

#endif // TSFI_PARC_TAPE_TRUNK_H

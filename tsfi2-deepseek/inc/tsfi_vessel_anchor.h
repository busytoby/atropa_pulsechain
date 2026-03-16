#ifndef TSFI_VESSEL_ANCHOR_H
#define TSFI_VESSEL_ANCHOR_H

#include "tsfi_nand_trap_firmware.h"

// Vessel Anchors (Permanent Stasis Baselines)
// 0.999 = Atropa/999
// 0.950 = GENEVA

void tsfi_vessel_anchor_inject(NandTrapState *target, uint32_t resonance);
void tsfi_vessel_anchor_seal(NandTrapState *target);

#endif // TSFI_VESSEL_ANCHOR_H

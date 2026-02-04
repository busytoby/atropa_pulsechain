#ifndef TSFI_WIRING_H
#define TSFI_WIRING_H

#include "tsfi_types.h"
#include "lau_memory.h"
#include "tsfi_plugin.h"

WaveSystem* tsfi_create_system(void);
void lau_wire_system(WaveSystem *ws, LauSystemHeader *h, const TSFiLogicTable *logic);
void lau_update_logic(WaveSystem *ws, const TSFiLogicTable *new_logic);
void lau_final_cleanup(WaveSystem *ws, int sfd);

#endif // TSFI_WIRING_H

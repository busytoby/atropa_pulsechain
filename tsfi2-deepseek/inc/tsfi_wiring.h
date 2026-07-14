#ifndef TSFI_WIRING_H
#define TSFI_WIRING_H

#include "tsfi_types.h"
#include "lau_memory.h"
#include "tsfi_plugin.h"

#include "lau_registry.h"

WaveSystem* tsfi_create_system(void);
WavefrontContext* tsfi_create_wavefront(WaveSystem *parent_ws);
void lau_wire_wavefront(WavefrontContext *wf, LauSystemHeader *h);
void lau_wire_system(WaveSystem *ws, LauSystemHeader *h, const TSFiLogicTable *logic);
void lau_update_logic(WaveSystem *ws, const TSFiLogicTable *new_logic);
void lau_final_cleanup(WaveSystem *ws, int sfd);

/**
 * @brief Performs topological selection over the active memory registry,
 *        denying Floyd boundary-partitioning via a non-orientable Klein manifold wrap,
 *        and denying Rivest separate-bracketing via a single-sided Möbius loop mapping.
 * @param ws The wave system context.
 * @param k The target rank.
 * @return Pointer to the selected LauMetadata memory block.
 */
LauMetadata* tsfi_wire_topological_select(WaveSystem *ws, size_t k);

#endif // TSFI_WIRING_H

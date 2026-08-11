/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_CAUSTICS_H
#define TSFI_CPM_MANN_RENDERMAN_CAUSTICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_caustics_init(uint32_t photon_count);
bool tsfi_cpm_mann_caustics_map_photons(float ior, uint64_t *zmm_caustic_latch_out);
bool tsfi_cpm_mann_caustics_render(uint64_t zmm_caustic_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_CAUSTICS_H */

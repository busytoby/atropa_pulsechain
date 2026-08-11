/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_HAIR_H
#define TSFI_CPM_MANN_RENDERMAN_HAIR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_hair_init(uint32_t strand_count);
bool tsfi_cpm_mann_hair_shade_marschner(float longitudinal_shift, float azimuthal_roughness, uint64_t *zmm_hair_latch_out);
bool tsfi_cpm_mann_hair_render_curves(uint64_t zmm_hair_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_HAIR_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_MATERIAL_H
#define TSFI_CPM_MANN_RENDERMAN_MATERIAL_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_material_init(uint32_t material_id);
bool tsfi_cpm_mann_material_shade_pbr(float metallic, float roughness, uint64_t *zmm_pbr_latch_out);
bool tsfi_cpm_mann_material_render_mesh(uint64_t zmm_pbr_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_MATERIAL_H */

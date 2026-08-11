/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_SMPL_TEXTURE_H
#define TSFI_CPM_TOMIE_SMPL_TEXTURE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_texture_init(uint32_t texture_id);
bool tsfi_cpm_tomie_texture_map_uv(uint32_t uv_coords_count, uint64_t *zmm_uv_latch_out);
bool tsfi_cpm_tomie_texture_apply_normal_map(uint64_t zmm_uv_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_SMPL_TEXTURE_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_MATERIAL_PHYSICS_H
#define TSFI_CPM_MODEL_SMPL_MATERIAL_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_material_physics_init(uint32_t material_id);
bool tsfi_cpm_material_physics_evaluate(uint64_t zmm_rig_latch, uint64_t *zmm_mat_latch_out);
bool tsfi_cpm_material_physics_render_renderman(uint64_t zmm_mat_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_MATERIAL_PHYSICS_H */

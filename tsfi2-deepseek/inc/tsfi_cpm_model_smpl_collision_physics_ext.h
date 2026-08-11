/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_COLLISION_PHYSICS_EXT_H
#define TSFI_CPM_MODEL_SMPL_COLLISION_PHYSICS_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_collision_physics_ext_init(uint32_t mesh_id);
bool tsfi_cpm_collision_physics_ext_detect(uint64_t zmm_rig_latch, uint32_t step_index, uint64_t *zmm_collision_latch_out);
bool tsfi_cpm_collision_physics_ext_render_renderman(uint64_t zmm_collision_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_COLLISION_PHYSICS_EXT_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_TOMIE_SMPL_MOCAP_RETARGETING_H
#define TSFI_TOMIE_SMPL_MOCAP_RETARGETING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_tomie_mocap_init(uint32_t blendshape_count);
bool tsfi_tomie_mocap_retarget(uint32_t frame_id, uint64_t mocap_joint_hash, uint64_t *zmm_blendshape_latch_out);
bool tsfi_tomie_mocap_render_face(uint64_t zmm_blendshape_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_TOMIE_SMPL_MOCAP_RETARGETING_H */

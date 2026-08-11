/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_EXT2_H
#define TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_EXT2_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mocap_retarget_ext2_init(uint32_t stream_id);
bool tsfi_cpm_mocap_retarget_ext2_eval(uint64_t zmm_rig_latch, uint32_t joint_mask, uint64_t *zmm_retarget_latch_out);
bool tsfi_cpm_mocap_retarget_ext2_render_renderman(uint64_t zmm_retarget_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_EXT2_H */

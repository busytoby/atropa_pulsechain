/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_EXT_H
#define TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mocap_retarget_init(uint32_t stream_id);
bool tsfi_cpm_mocap_retarget_stream(uint64_t zmm_rig_latch, uint32_t frame_index, uint64_t *zmm_mocap_latch_out);
bool tsfi_cpm_mocap_retarget_render_renderman(uint64_t zmm_mocap_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_EXT_H */

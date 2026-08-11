/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_H
#define TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mocap_retarget_init(uint32_t stream_id);
bool tsfi_cpm_mocap_retarget_sync_frame(uint32_t mocap_frame_id, uint64_t *zmm_mocap_latch_out);
bool tsfi_cpm_mocap_retarget_render_renderman(uint64_t zmm_mocap_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_MOCAP_RETARGET_H */

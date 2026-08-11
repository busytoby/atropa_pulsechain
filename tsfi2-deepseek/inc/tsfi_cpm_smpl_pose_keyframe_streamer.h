/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_POSE_KEYFRAME_STREAMER_H
#define TSFI_CPM_SMPL_POSE_KEYFRAME_STREAMER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_pose_keyframe_streamer_init(uint32_t keyframe_id);
bool tsfi_cpm_smpl_pose_keyframe_streamer_eval(uint64_t zmm_rig_latch, uint32_t keyframe_id, uint64_t *zmm_stream_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_POSE_KEYFRAME_STREAMER_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_MOTION_BLUR_CURVED_TRAJECTORY_H
#define TSFI_CPM_SMPL_MOTION_BLUR_CURVED_TRAJECTORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_motion_blur_curved_trajectory_init(uint32_t subframe_samples);
bool tsfi_cpm_smpl_motion_blur_curved_trajectory_eval(uint64_t zmm_rig_latch, uint32_t subframe_samples, uint64_t *zmm_trajectory_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_MOTION_BLUR_CURVED_TRAJECTORY_H */

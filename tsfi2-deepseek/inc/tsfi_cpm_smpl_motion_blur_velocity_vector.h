/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_MOTION_BLUR_VELOCITY_VECTOR_H
#define TSFI_CPM_SMPL_MOTION_BLUR_VELOCITY_VECTOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_motion_blur_velocity_vector_init(uint32_t vertex_count);
bool tsfi_cpm_smpl_motion_blur_velocity_vector_eval(uint64_t zmm_rig_latch, uint32_t vertex_count, uint64_t *zmm_velocity_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_MOTION_BLUR_VELOCITY_VECTOR_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_MOTION_BLUR_SHUTTER_ACCUMULATOR_H
#define TSFI_CPM_SMPL_MOTION_BLUR_SHUTTER_ACCUMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_motion_blur_shutter_accumulator_init(uint32_t shutter_angle);
bool tsfi_cpm_smpl_motion_blur_shutter_accumulator_eval(uint64_t zmm_rig_latch, uint32_t shutter_angle, uint64_t *zmm_shutter_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_MOTION_BLUR_SHUTTER_ACCUMULATOR_H */

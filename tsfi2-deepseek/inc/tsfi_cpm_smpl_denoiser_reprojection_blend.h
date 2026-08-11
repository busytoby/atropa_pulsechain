/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_DENOISER_REPROJECTION_BLEND_H
#define TSFI_CPM_SMPL_DENOISER_REPROJECTION_BLEND_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_denoiser_reprojection_blend_init(uint32_t history_frames);
bool tsfi_cpm_smpl_denoiser_reprojection_blend_eval(uint64_t zmm_rig_latch, uint32_t history_frames, uint64_t *zmm_reproject_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_DENOISER_REPROJECTION_BLEND_H */

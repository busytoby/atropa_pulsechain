/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_DENOISER_ALBEDO_NORMAL_GUIDE_H
#define TSFI_CPM_SMPL_DENOISER_ALBEDO_NORMAL_GUIDE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_denoiser_albedo_normal_guide_init(uint32_t feature_channels);
bool tsfi_cpm_smpl_denoiser_albedo_normal_guide_eval(uint64_t zmm_rig_latch, uint32_t feature_channels, uint64_t *zmm_guide_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_DENOISER_ALBEDO_NORMAL_GUIDE_H */

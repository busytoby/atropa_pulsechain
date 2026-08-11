/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_DENOISER_OIDN_SPATIAL_FILTER_H
#define TSFI_CPM_SMPL_DENOISER_OIDN_SPATIAL_FILTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_denoiser_oidn_spatial_filter_init(uint32_t filter_radius);
bool tsfi_cpm_smpl_denoiser_oidn_spatial_filter_eval(uint64_t zmm_rig_latch, uint32_t filter_radius, uint64_t *zmm_filter_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_DENOISER_OIDN_SPATIAL_FILTER_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_ATMOSPHERE_PLANETARY_CURVATURE_H
#define TSFI_CPM_SMPL_ATMOSPHERE_PLANETARY_CURVATURE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_atmosphere_planetary_curvature_init(uint32_t horizon_samples);
bool tsfi_cpm_smpl_atmosphere_planetary_curvature_eval(uint64_t zmm_rig_latch, uint32_t horizon_samples, uint64_t *zmm_horizon_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_ATMOSPHERE_PLANETARY_CURVATURE_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_LIGHTING_SPHERICAL_HARMONICS_H
#define TSFI_CPM_SMPL_LIGHTING_SPHERICAL_HARMONICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_lighting_spherical_harmonics_init(uint32_t band_count);
bool tsfi_cpm_smpl_lighting_spherical_harmonics_eval(uint64_t zmm_rig_latch, uint32_t band_count, uint64_t *zmm_sh_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_LIGHTING_SPHERICAL_HARMONICS_H */

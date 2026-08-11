/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_VOLUMETRIC_SUBSURFACE_SCATTERING_H
#define TSFI_CPM_SMPL_VOLUMETRIC_SUBSURFACE_SCATTERING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_volumetric_subsurface_scattering_init(uint32_t sss_sample_count);
bool tsfi_cpm_smpl_volumetric_subsurface_scattering_eval(uint64_t zmm_rig_latch, uint32_t sss_sample_count, uint64_t *zmm_sss_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_VOLUMETRIC_SUBSURFACE_SCATTERING_H */

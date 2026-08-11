/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_PBR_AMBIENT_OCCLUSION_INTEGRATOR_H
#define TSFI_CPM_SMPL_PBR_AMBIENT_OCCLUSION_INTEGRATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_pbr_ambient_occlusion_integrator_init(uint32_t sample_radius);
bool tsfi_cpm_smpl_pbr_ambient_occlusion_integrator_eval(uint64_t zmm_rig_latch, uint32_t sample_radius, uint64_t *zmm_ao_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_PBR_AMBIENT_OCCLUSION_INTEGRATOR_H */

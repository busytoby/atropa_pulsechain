/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_PBR_FRESNEL_SCHLICK_H
#define TSFI_CPM_SMPL_PBR_FRESNEL_SCHLICK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_pbr_fresnel_schlick_init(uint32_t incident_angle);
bool tsfi_cpm_smpl_pbr_fresnel_schlick_eval(uint64_t zmm_rig_latch, uint32_t incident_angle, uint64_t *zmm_fresnel_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_PBR_FRESNEL_SCHLICK_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_ATMOSPHERE_RAYLEIGH_SKY_H
#define TSFI_CPM_SMPL_ATMOSPHERE_RAYLEIGH_SKY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_atmosphere_rayleigh_sky_init(uint32_t zenith_angle);
bool tsfi_cpm_smpl_atmosphere_rayleigh_sky_eval(uint64_t zmm_rig_latch, uint32_t zenith_angle, uint64_t *zmm_rayleigh_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_ATMOSPHERE_RAYLEIGH_SKY_H */

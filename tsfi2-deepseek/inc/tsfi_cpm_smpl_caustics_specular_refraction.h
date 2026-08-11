/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_CAUSTICS_SPECULAR_REFRACTION_H
#define TSFI_CPM_SMPL_CAUSTICS_SPECULAR_REFRACTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_caustics_specular_refraction_init(uint32_t refraction_ray_count);
bool tsfi_cpm_smpl_caustics_specular_refraction_eval(uint64_t zmm_rig_latch, uint32_t refraction_ray_count, uint64_t *zmm_refraction_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_CAUSTICS_SPECULAR_REFRACTION_H */

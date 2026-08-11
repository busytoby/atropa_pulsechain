/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_MATERIAL_ANISOTROPIC_REFLECTANCE_H
#define TSFI_CPM_SMPL_MATERIAL_ANISOTROPIC_REFLECTANCE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_material_anisotropic_reflectance_init(uint32_t anisotropy_degree);
bool tsfi_cpm_smpl_material_anisotropic_reflectance_eval(uint64_t zmm_rig_latch, uint32_t anisotropy_degree, uint64_t *zmm_reflectance_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_MATERIAL_ANISOTROPIC_REFLECTANCE_H */

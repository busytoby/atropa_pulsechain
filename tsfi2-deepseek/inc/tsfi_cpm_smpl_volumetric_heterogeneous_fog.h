/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_VOLUMETRIC_HETEROGENEOUS_FOG_H
#define TSFI_CPM_SMPL_VOLUMETRIC_HETEROGENEOUS_FOG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_volumetric_heterogeneous_fog_init(uint32_t fog_grid_size);
bool tsfi_cpm_smpl_volumetric_heterogeneous_fog_eval(uint64_t zmm_rig_latch, uint32_t fog_grid_size, uint64_t *zmm_fog_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_VOLUMETRIC_HETEROGENEOUS_FOG_H */

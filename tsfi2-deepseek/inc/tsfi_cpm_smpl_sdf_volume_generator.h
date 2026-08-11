/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_SDF_VOLUME_GENERATOR_H
#define TSFI_CPM_SMPL_SDF_VOLUME_GENERATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_sdf_volume_generator_init(uint32_t grid_dim);
bool tsfi_cpm_smpl_sdf_volume_generator_eval(uint64_t zmm_rig_latch, uint32_t grid_dim, uint64_t *zmm_sdf_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_SDF_VOLUME_GENERATOR_H */

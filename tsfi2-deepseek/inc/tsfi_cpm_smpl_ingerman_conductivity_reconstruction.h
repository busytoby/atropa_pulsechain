/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_INGERMAN_CONDUCTIVITY_RECONSTRUCTION_H
#define TSFI_CPM_SMPL_INGERMAN_CONDUCTIVITY_RECONSTRUCTION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_ingerman_conductivity_reconstruction_init(uint32_t mesh_voxels);
bool tsfi_cpm_smpl_ingerman_conductivity_reconstruction_eval(uint64_t zmm_rig_latch, uint32_t mesh_voxels, uint64_t *zmm_conductivity_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_INGERMAN_CONDUCTIVITY_RECONSTRUCTION_H */

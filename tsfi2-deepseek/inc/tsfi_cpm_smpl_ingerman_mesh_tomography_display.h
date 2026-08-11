/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_INGERMAN_MESH_TOMOGRAPHY_DISPLAY_H
#define TSFI_CPM_SMPL_INGERMAN_MESH_TOMOGRAPHY_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_ingerman_mesh_tomography_display_init(uint32_t display_resolution);
bool tsfi_cpm_smpl_ingerman_mesh_tomography_display_eval(uint64_t zmm_rig_latch, uint32_t display_resolution, uint64_t *zmm_display_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_INGERMAN_MESH_TOMOGRAPHY_DISPLAY_H */

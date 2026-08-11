/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_INGERMAN_BOUNDARY_CURRENT_H
#define TSFI_CPM_SMPL_INGERMAN_BOUNDARY_CURRENT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_ingerman_boundary_current_init(uint32_t electrode_count);
bool tsfi_cpm_smpl_ingerman_boundary_current_eval(uint64_t zmm_rig_latch, uint32_t electrode_count, uint64_t *zmm_boundary_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_INGERMAN_BOUNDARY_CURRENT_H */

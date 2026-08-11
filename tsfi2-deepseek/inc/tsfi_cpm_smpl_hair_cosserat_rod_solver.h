/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_HAIR_COSSERAT_ROD_SOLVER_H
#define TSFI_CPM_SMPL_HAIR_COSSERAT_ROD_SOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_hair_cosserat_rod_solver_init(uint32_t rod_count);
bool tsfi_cpm_smpl_hair_cosserat_rod_solver_eval(uint64_t zmm_rig_latch, uint32_t rod_count, uint64_t *zmm_rod_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_HAIR_COSSERAT_ROD_SOLVER_H */

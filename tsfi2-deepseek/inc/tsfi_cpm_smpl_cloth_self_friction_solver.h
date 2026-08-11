/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_CLOTH_SELF_FRICTION_SOLVER_H
#define TSFI_CPM_SMPL_CLOTH_SELF_FRICTION_SOLVER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_cloth_self_friction_solver_init(uint32_t contact_pair_count);
bool tsfi_cpm_smpl_cloth_self_friction_solver_eval(uint64_t zmm_rig_latch, uint32_t contact_pair_count, uint64_t *zmm_friction_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_CLOTH_SELF_FRICTION_SOLVER_H */

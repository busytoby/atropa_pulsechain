/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_CLOTH_CONSTRAINT_SPRING_H
#define TSFI_CPM_SMPL_CLOTH_CONSTRAINT_SPRING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_cloth_constraint_spring_init(uint32_t spring_count);
bool tsfi_cpm_smpl_cloth_constraint_spring_eval(uint64_t zmm_rig_latch, uint32_t spring_count, uint64_t *zmm_spring_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_CLOTH_CONSTRAINT_SPRING_H */

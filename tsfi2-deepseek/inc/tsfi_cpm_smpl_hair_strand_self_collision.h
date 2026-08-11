/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_HAIR_STRAND_SELF_COLLISION_H
#define TSFI_CPM_SMPL_HAIR_STRAND_SELF_COLLISION_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_hair_strand_self_collision_init(uint32_t strand_pair_count);
bool tsfi_cpm_smpl_hair_strand_self_collision_eval(uint64_t zmm_rig_latch, uint32_t strand_pair_count, uint64_t *zmm_collision_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_HAIR_STRAND_SELF_COLLISION_H */

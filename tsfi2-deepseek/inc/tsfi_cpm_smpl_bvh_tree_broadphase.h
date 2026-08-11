/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_BVH_TREE_BROADPHASE_H
#define TSFI_CPM_SMPL_BVH_TREE_BROADPHASE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_bvh_tree_broadphase_init(uint32_t node_count);
bool tsfi_cpm_smpl_bvh_tree_broadphase_eval(uint64_t zmm_rig_latch, uint32_t node_count, uint64_t *zmm_bvh_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_BVH_TREE_BROADPHASE_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SMPL_JOINT_FK_IK_KINEMATICS_H
#define TSFI_CPM_SMPL_JOINT_FK_IK_KINEMATICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_joint_fk_ik_kinematics_init(uint32_t joint_count);
bool tsfi_cpm_smpl_joint_fk_ik_kinematics_eval(uint64_t zmm_rig_latch, uint32_t joint_count, uint64_t *zmm_joint_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SMPL_JOINT_FK_IK_KINEMATICS_H */

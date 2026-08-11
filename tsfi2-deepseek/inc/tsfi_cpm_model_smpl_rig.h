/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_RIG_H
#define TSFI_CPM_MODEL_SMPL_RIG_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_smpl_rig_init(uint32_t joint_count);
bool tsfi_cpm_smpl_rig_bind_weights(uint64_t weights_latch, uint64_t *zmm_rig_latch_out);
bool tsfi_cpm_smpl_rig_dispatch_renderman(uint64_t zmm_rig_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_RIG_H */

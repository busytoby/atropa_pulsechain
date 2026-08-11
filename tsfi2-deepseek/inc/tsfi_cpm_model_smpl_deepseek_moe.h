/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_DEEPSEEK_MOE_H
#define TSFI_CPM_MODEL_SMPL_DEEPSEEK_MOE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_deepseek_moe_init(uint32_t num_experts);
bool tsfi_cpm_deepseek_moe_route(uint64_t zmm_rig_latch, uint32_t expert_mask, uint64_t *zmm_moe_latch_out);
bool tsfi_cpm_deepseek_moe_render_renderman(uint64_t zmm_moe_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_DEEPSEEK_MOE_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_DEEPSEEK_EXPERT_AFFINITY_GATE_H
#define TSFI_CPM_DEEPSEEK_EXPERT_AFFINITY_GATE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_deepseek_expert_affinity_gate_init(uint32_t expert_id);
bool tsfi_cpm_deepseek_expert_affinity_gate_eval(uint64_t zmm_rig_latch, uint32_t expert_id, uint64_t *zmm_affinity_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_DEEPSEEK_EXPERT_AFFINITY_GATE_H */

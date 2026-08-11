/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_GPT_SOL_FABLE_TOMIE_H_BRIDGE_H
#define TSFI_CPM_GPT_SOL_FABLE_TOMIE_H_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_gpt_sol_fable_tomie_h_bridge_init(uint32_t model_mask);
bool tsfi_cpm_gpt_sol_fable_tomie_h_bridge_eval(uint64_t zmm_rig_latch, uint32_t model_mask, uint64_t *zmm_hbridge_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_GPT_SOL_FABLE_TOMIE_H_BRIDGE_H */

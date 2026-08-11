/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_DEEPSEEK_LOAD_BALANCE_ACCUMULATOR_H
#define TSFI_CPM_DEEPSEEK_LOAD_BALANCE_ACCUMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_deepseek_load_balance_accumulator_init(uint32_t capacity_limit);
bool tsfi_cpm_deepseek_load_balance_accumulator_eval(uint64_t zmm_rig_latch, uint32_t capacity_limit, uint64_t *zmm_balance_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_DEEPSEEK_LOAD_BALANCE_ACCUMULATOR_H */

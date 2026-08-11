/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_DYNAMIC_PLL_FREQUENCY_SCALER_H
#define TSFI_CPM_DYNAMIC_PLL_FREQUENCY_SCALER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_dynamic_pll_frequency_scaler_init(uint32_t multiplier);
bool tsfi_cpm_dynamic_pll_frequency_scaler_eval(uint64_t zmm_rig_latch, uint32_t multiplier, uint64_t *zmm_pll_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_DYNAMIC_PLL_FREQUENCY_SCALER_H */

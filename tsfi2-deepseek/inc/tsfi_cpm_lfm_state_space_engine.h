/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_STATE_SPACE_ENGINE_H
#define TSFI_CPM_LFM_STATE_SPACE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_lfm_state_space_engine_init(uint32_t state_dim);
bool tsfi_cpm_lfm_state_space_engine_eval(uint64_t zmm_rig_latch, uint32_t state_dim, uint64_t *zmm_lfm_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_STATE_SPACE_ENGINE_H */

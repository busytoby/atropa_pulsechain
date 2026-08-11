/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_VL_SANDBOX_DISPLAY_H
#define TSFI_CPM_LFM_VL_SANDBOX_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_lfm_vl_sandbox_display_init(uint32_t frame_id);
bool tsfi_cpm_lfm_vl_sandbox_display_eval(uint64_t zmm_rig_latch, uint32_t frame_id, uint64_t *zmm_vision_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_VL_SANDBOX_DISPLAY_H */

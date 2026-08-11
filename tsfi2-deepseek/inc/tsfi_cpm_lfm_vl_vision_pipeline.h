/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_VL_VISION_PIPELINE_H
#define TSFI_CPM_LFM_VL_VISION_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_lfm_vl_vision_pipeline_init(uint32_t frame_id);
bool tsfi_cpm_lfm_vl_vision_pipeline_eval(uint64_t zmm_rig_latch, uint32_t frame_id, uint64_t *zmm_vl_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_VL_VISION_PIPELINE_H */

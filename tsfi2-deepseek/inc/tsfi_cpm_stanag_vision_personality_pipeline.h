/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_STANAG_VISION_PERSONALITY_PIPELINE_H
#define TSFI_CPM_STANAG_VISION_PERSONALITY_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_stanag_vision_personality_pipeline_init(uint32_t stanag_key, uint32_t personality_id);
bool tsfi_cpm_stanag_vision_personality_pipeline_eval(uint64_t zmm_rig_latch, uint32_t camera_id, uint64_t *zmm_unified_latch_out);
bool tsfi_cpm_stanag_vision_personality_pipeline_render_renderman(uint64_t zmm_unified_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_STANAG_VISION_PERSONALITY_PIPELINE_H */

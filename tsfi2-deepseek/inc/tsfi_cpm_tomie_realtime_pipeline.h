/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_REALTIME_PIPELINE_H
#define TSFI_CPM_TOMIE_REALTIME_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_realtime_pipeline_init(uint32_t target_fps);
bool tsfi_cpm_tomie_realtime_pipeline_step_frame(uint32_t frame_index, uint64_t *zmm_pipeline_latch_out);
bool tsfi_cpm_tomie_realtime_pipeline_present_120fps(uint64_t zmm_pipeline_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_REALTIME_PIPELINE_H */

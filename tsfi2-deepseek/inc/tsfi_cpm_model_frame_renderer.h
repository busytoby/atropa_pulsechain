/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_FRAME_RENDERER_H
#define TSFI_CPM_MODEL_FRAME_RENDERER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_model_frame_renderer_init(const char *dat_bin_path);
bool tsfi_cpm_model_frame_renderer_project_weights(uint64_t weights_latch, uint64_t *zmm_frame_latch_out);
bool tsfi_cpm_model_frame_renderer_draw_renderman(uint64_t zmm_frame_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_FRAME_RENDERER_H */

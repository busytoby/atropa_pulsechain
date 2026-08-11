/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_RENDERMAN_MULTI_CAMERA_H
#define TSFI_CPM_MODEL_RENDERMAN_MULTI_CAMERA_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_multi_camera_init(uint32_t camera_count);
bool tsfi_cpm_multi_camera_switch(uint32_t camera_id, uint64_t *zmm_camera_latch_out);
bool tsfi_cpm_multi_camera_render_frame(uint64_t zmm_camera_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_RENDERMAN_MULTI_CAMERA_H */

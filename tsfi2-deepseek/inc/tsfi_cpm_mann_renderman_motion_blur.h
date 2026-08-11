/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_MOTION_BLUR_H
#define TSFI_CPM_MANN_RENDERMAN_MOTION_BLUR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_motion_blur_init(float shutter_open, float shutter_close);
bool tsfi_cpm_mann_motion_blur_sample_subframe(float shutter_time, uint64_t *zmm_motion_latch_out);
bool tsfi_cpm_mann_motion_blur_render_dof(uint64_t zmm_motion_latch, float aperture, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_MOTION_BLUR_H */

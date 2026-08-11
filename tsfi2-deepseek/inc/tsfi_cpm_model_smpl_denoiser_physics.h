/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_DENOISER_PHYSICS_H
#define TSFI_CPM_MODEL_SMPL_DENOISER_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_denoiser_physics_init(uint32_t denoise_passes);
bool tsfi_cpm_denoiser_physics_filter(uint64_t zmm_rig_latch, uint64_t *zmm_denoise_latch_out);
bool tsfi_cpm_denoiser_physics_render_renderman(uint64_t zmm_denoise_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_DENOISER_PHYSICS_H */

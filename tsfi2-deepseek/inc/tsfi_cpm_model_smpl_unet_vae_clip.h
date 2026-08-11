/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_UNET_VAE_CLIP_H
#define TSFI_CPM_MODEL_SMPL_UNET_VAE_CLIP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_unet_vae_clip_init(uint32_t latent_steps);
bool tsfi_cpm_unet_vae_clip_unwrap(uint64_t zmm_rig_latch, uint32_t clip_hash, uint64_t *zmm_sd_latch_out);
bool tsfi_cpm_unet_vae_clip_render_renderman(uint64_t zmm_sd_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_UNET_VAE_CLIP_H */

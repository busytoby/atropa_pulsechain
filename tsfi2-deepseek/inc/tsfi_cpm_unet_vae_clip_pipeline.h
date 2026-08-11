/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_UNET_VAE_CLIP_PIPELINE_H
#define TSFI_CPM_UNET_VAE_CLIP_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_unet_vae_clip_pipeline_init(uint32_t latent_dim);
bool tsfi_cpm_unet_vae_clip_pipeline_eval(uint64_t zmm_rig_latch, uint32_t prompt_tokens, uint64_t *zmm_unet_latch_out);
bool tsfi_cpm_unet_vae_clip_pipeline_render_renderman(uint64_t zmm_unet_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_UNET_VAE_CLIP_PIPELINE_H */

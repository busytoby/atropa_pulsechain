/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_DENOISER_H
#define TSFI_CPM_MANN_RENDERMAN_DENOISER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_denoiser_init(uint32_t model_id);
bool tsfi_cpm_mann_denoiser_denoise_frame(uint64_t noisy_vram_phys_addr, uint64_t *zmm_clean_latch_out);
bool tsfi_cpm_mann_denoiser_output_clean(uint64_t zmm_clean_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_DENOISER_H */

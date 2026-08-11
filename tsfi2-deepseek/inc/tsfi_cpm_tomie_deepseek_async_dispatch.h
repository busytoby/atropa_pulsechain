/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_ASYNC_DISPATCH_H
#define TSFI_CPM_TOMIE_DEEPSEEK_ASYNC_DISPATCH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_async_dispatch_grid(uint32_t ring_id, uint32_t grid_x, uint32_t grid_y, uint64_t *zmm_grid_hash_out);
bool tsfi_cpm_tomie_sd_async_dispatch_pipeline(uint32_t ring_id, uint32_t grid_x, uint32_t grid_y, uint32_t latent_steps, uint64_t *zmm_sd_grid_hash_out);
bool tsfi_cpm_tomie_deepseek_async_wait_fence(uint64_t fence_seq_id, bool *fence_complete_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_ASYNC_DISPATCH_H */

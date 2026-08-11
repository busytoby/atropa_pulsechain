/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_KV_CACHE_H
#define TSFI_CPM_TOMIE_DEEPSEEK_KV_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_kv_cache_alloc(uint32_t layer_idx, uint64_t vram_phys_addr, uint64_t *zmm_kv_state_out);
bool tsfi_cpm_tomie_deepseek_kv_cache_lookup(uint32_t layer_idx, uint32_t token_id, uint64_t *zmm_kv_entry_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_KV_CACHE_H */

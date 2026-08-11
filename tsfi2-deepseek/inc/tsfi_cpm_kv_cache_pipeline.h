/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_KV_CACHE_PIPELINE_H
#define TSFI_CPM_KV_CACHE_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_kv_cache_pipeline_init(uint32_t cache_id);
bool tsfi_cpm_kv_cache_pipeline_eval(uint64_t zmm_rig_latch, uint32_t token_pos, uint64_t *zmm_cache_latch_out);
bool tsfi_cpm_kv_cache_pipeline_render_renderman(uint64_t zmm_cache_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_KV_CACHE_PIPELINE_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_KV_CACHE_SYNAPSE_H
#define TSFI_CPM_LFM_KV_CACHE_SYNAPSE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_lfm_kv_cache_synapse_init(uint32_t cache_id);
bool tsfi_cpm_lfm_kv_cache_synapse_eval(uint64_t zmm_rig_latch, uint32_t cache_id, uint64_t *zmm_cache_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_KV_CACHE_SYNAPSE_H */

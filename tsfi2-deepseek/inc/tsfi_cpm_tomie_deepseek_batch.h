/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_BATCH_H
#define TSFI_CPM_TOMIE_DEEPSEEK_BATCH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_batch_infer(const uint32_t *tokens, size_t count, uint64_t *zmm_batch_hashes_out);
bool tsfi_cpm_tomie_deepseek_batch_render(const uint64_t *zmm_batch_hashes, size_t count, uint64_t base_vram_phys);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_BATCH_H */

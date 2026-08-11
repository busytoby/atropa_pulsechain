/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_DEEPSEEK_SPARSE_TOKEN_DISPATCHER_H
#define TSFI_CPM_DEEPSEEK_SPARSE_TOKEN_DISPATCHER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_deepseek_sparse_token_dispatcher_init(uint32_t token_count);
bool tsfi_cpm_deepseek_sparse_token_dispatcher_eval(uint64_t zmm_rig_latch, uint32_t token_count, uint64_t *zmm_dispatch_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_DEEPSEEK_SPARSE_TOKEN_DISPATCHER_H */

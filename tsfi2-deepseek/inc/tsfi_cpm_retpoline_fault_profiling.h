/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_RETPOLINE_FAULT_PROFILING_H
#define TSFI_CPM_RETPOLINE_FAULT_PROFILING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_retpoline_fault_profiling_init(uint32_t sample_count);
bool tsfi_cpm_retpoline_fault_profiling_eval(uint64_t zmm_rig_latch, uint32_t sample_count, uint64_t *zmm_profiling_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_RETPOLINE_FAULT_PROFILING_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_LFM_KERNEL_AGENT_SDK_H
#define TSFI_CPM_LFM_KERNEL_AGENT_SDK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_lfm_kernel_agent_sdk_init(uint32_t tool_id);
bool tsfi_cpm_lfm_kernel_agent_sdk_eval(uint64_t zmm_rig_latch, uint32_t tool_id, uint64_t *zmm_tool_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_LFM_KERNEL_AGENT_SDK_H */

/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_VULKAN_TIMELINE_SEMAPHORE_SYNC_H
#define TSFI_CPM_VULKAN_TIMELINE_SEMAPHORE_SYNC_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_vulkan_timeline_semaphore_sync_init(uint64_t initial_value);
bool tsfi_cpm_vulkan_timeline_semaphore_sync_eval(uint64_t zmm_rig_latch, uint64_t semaphore_value, uint64_t *signaled_value_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_VULKAN_TIMELINE_SEMAPHORE_SYNC_H */

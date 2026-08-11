/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_VULKAN_INTEROP_H
#define TSFI_CPM_TOMIE_VULKAN_INTEROP_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_vulkan_bind_smpl_pipeline(uint64_t vk_command_buffer, uint64_t vk_pipeline, uint64_t *zmm_vk_latch_out);
bool tsfi_cpm_tomie_vulkan_submit_smpl_draw(uint64_t vk_queue, uint32_t vertex_count, uint64_t *zmm_draw_fence_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_VULKAN_INTEROP_H */

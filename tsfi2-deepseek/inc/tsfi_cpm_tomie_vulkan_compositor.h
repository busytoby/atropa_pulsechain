/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_VULKAN_COMPOSITOR_H
#define TSFI_CPM_TOMIE_VULKAN_COMPOSITOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_vulkan_compositor_init(uint64_t vram_phys_addr);
bool tsfi_cpm_tomie_vulkan_compositor_present(uint64_t zmm_frame_latch, uint32_t width, uint32_t height);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_VULKAN_COMPOSITOR_H */

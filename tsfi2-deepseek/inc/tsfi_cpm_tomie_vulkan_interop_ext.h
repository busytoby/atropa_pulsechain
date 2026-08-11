/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_VULKAN_INTEROP_EXT_H
#define TSFI_CPM_TOMIE_VULKAN_INTEROP_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_vulkan_hardware_interop_init(uint32_t instance_flags);
bool tsfi_cpm_vulkan_hardware_interop_flip(uint64_t vram_phys_addr, uint32_t crtc_id, uint64_t *zmm_vk_latch_out);
bool tsfi_cpm_vulkan_hardware_interop_render_renderman(uint64_t zmm_vk_latch);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_VULKAN_INTEROP_EXT_H */

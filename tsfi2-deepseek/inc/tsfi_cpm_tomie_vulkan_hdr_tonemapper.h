/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_VULKAN_HDR_TONEMAPPER_H
#define TSFI_CPM_TOMIE_VULKAN_HDR_TONEMAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_hdr_tonemapper_init(uint32_t profile_id);
bool tsfi_cpm_tomie_hdr_tonemapper_map_aces(float exposure, uint64_t *zmm_hdr_latch_out);
bool tsfi_cpm_tomie_hdr_tonemapper_present_vulkan(uint64_t zmm_hdr_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_VULKAN_HDR_TONEMAPPER_H */

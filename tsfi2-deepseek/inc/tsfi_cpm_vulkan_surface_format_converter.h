/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_VULKAN_SURFACE_FORMAT_CONVERTER_H
#define TSFI_CPM_VULKAN_SURFACE_FORMAT_CONVERTER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_vulkan_surface_format_converter_init(uint32_t format_enum);
bool tsfi_cpm_vulkan_surface_format_converter_eval(uint64_t zmm_rig_latch, uint32_t format_enum, uint64_t *converted_format_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_VULKAN_SURFACE_FORMAT_CONVERTER_H */

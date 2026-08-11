/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_VULKAN_ZERO_COPY_PIPELINE_H
#define TSFI_CPM_VULKAN_ZERO_COPY_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_vulkan_zero_copy_pipeline_init(uint32_t pasid);
bool tsfi_cpm_vulkan_zero_copy_pipeline_eval(uint64_t zmm_rig_latch, uint32_t flip_seq, uint64_t *zmm_vulkan_latch_out);
bool tsfi_cpm_vulkan_zero_copy_pipeline_render_renderman(uint64_t zmm_vulkan_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_VULKAN_ZERO_COPY_PIPELINE_H */

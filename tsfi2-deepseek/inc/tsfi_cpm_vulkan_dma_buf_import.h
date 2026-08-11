/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_VULKAN_DMA_BUF_IMPORT_H
#define TSFI_CPM_VULKAN_DMA_BUF_IMPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_vulkan_dma_buf_import_init(uint32_t dma_buf_fd);
bool tsfi_cpm_vulkan_dma_buf_import_eval(uint64_t zmm_rig_latch, uint32_t dma_buf_fd, uint64_t *phys_page_addr_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_VULKAN_DMA_BUF_IMPORT_H */

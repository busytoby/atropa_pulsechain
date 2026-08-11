/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MANN_RENDERMAN_H
#define TSFI_CPM_MANN_RENDERMAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_mann_renderman_init(uint32_t ris_context_id);
bool tsfi_cpm_mann_renderman_dispatch_smpl(uint32_t vertex_count, uint64_t zmm_mann_memory_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MANN_RENDERMAN_H */

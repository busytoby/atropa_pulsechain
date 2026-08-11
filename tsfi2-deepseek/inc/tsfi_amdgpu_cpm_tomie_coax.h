/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_AMDGPU_CPM_TOMIE_COAX_H
#define TSFI_AMDGPU_CPM_TOMIE_COAX_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_amdgpu_cpm_tomie_coax_init(uint32_t ring_id);
bool tsfi_amdgpu_cpm_tomie_coax_submit(uint32_t ring_id, uint64_t zmm_latch, uint64_t vram_phys_addr, uint64_t *fence_seq_out);
bool tsfi_amdgpu_cpm_tomie_coax_wait_fence(uint64_t fence_seq);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_AMDGPU_CPM_TOMIE_COAX_H */

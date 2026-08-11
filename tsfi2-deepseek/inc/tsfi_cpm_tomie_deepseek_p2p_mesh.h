/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_DEEPSEEK_P2P_MESH_H
#define TSFI_CPM_TOMIE_DEEPSEEK_P2P_MESH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_deepseek_p2p_transfer(uint32_t src_gpu_id, uint32_t dst_gpu_id, uint64_t src_vram_phys, uint64_t dst_vram_phys, size_t size_bytes);
bool tsfi_cpm_tomie_deepseek_p2p_barrier_sync(uint32_t gpu_mask, uint64_t *zmm_mesh_barrier_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_DEEPSEEK_P2P_MESH_H */

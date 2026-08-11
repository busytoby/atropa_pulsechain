/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_PERSONALITY_ENGINE_H
#define TSFI_CPM_PERSONALITY_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_personality_init(uint32_t trait_mask);
bool tsfi_cpm_personality_map_token(uint32_t cpm_token_id, uint64_t *zmm_trait_latch_out);
bool tsfi_cpm_personality_render_pose(uint64_t zmm_trait_latch, uint64_t smpl_vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_PERSONALITY_ENGINE_H */

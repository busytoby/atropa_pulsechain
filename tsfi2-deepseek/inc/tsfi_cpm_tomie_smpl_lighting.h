/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOMIE_SMPL_LIGHTING_H
#define TSFI_CPM_TOMIE_SMPL_LIGHTING_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_tomie_lighting_init(uint32_t light_count);
bool tsfi_cpm_tomie_lighting_compute_sh(uint32_t light_id, uint64_t *zmm_sh_latch_out);
bool tsfi_cpm_tomie_lighting_render_smpl(uint64_t zmm_sh_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOMIE_SMPL_LIGHTING_H */

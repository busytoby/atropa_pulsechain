/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_GAUNTLET_COMFY_CHAIR_PIPELINE_H
#define TSFI_CPM_GAUNTLET_COMFY_CHAIR_PIPELINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_gauntlet_comfy_chair_pipeline_init(uint32_t chair_id);
bool tsfi_cpm_gauntlet_comfy_chair_pipeline_eval(uint64_t zmm_rig_latch, uint32_t roughness_pct, uint64_t *zmm_chair_latch_out);
bool tsfi_cpm_gauntlet_comfy_chair_pipeline_render_renderman(uint64_t zmm_chair_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_GAUNTLET_COMFY_CHAIR_PIPELINE_H */

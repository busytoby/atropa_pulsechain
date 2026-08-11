/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_MODEL_SMPL_INGERMAN_DISPLAY_H
#define TSFI_CPM_MODEL_SMPL_INGERMAN_DISPLAY_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_ingerman_display_init(uint32_t block_id);
bool tsfi_cpm_ingerman_display_push(uint64_t zmm_rig_latch, uint64_t *zmm_ingerman_latch_out);
bool tsfi_cpm_ingerman_display_render_renderman(uint64_t zmm_ingerman_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_MODEL_SMPL_INGERMAN_DISPLAY_H */

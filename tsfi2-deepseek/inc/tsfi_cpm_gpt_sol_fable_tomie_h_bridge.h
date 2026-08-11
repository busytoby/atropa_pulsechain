/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_GPT_SOL_FABLE_TOMIE_H_BRIDGE_H
#define TSFI_CPM_GPT_SOL_FABLE_TOMIE_H_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_gpt_sol_fable_tomie_h_bridge_init(uint32_t bus_id);
bool tsfi_cpm_gpt_sol_fable_tomie_h_bridge_mount(uint32_t gpt_sol_id, uint32_t fable_id, uint32_t cpm_id, uint32_t tomie_id, uint64_t *h_bridge_quad_latch_out);
bool tsfi_cpm_gpt_sol_fable_tomie_h_bridge_render_renderman(uint64_t h_bridge_quad_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_GPT_SOL_FABLE_TOMIE_H_BRIDGE_H */

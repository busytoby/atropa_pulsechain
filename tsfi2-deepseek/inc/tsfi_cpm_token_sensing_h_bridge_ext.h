/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_TOKEN_SENSING_H_BRIDGE_EXT_H
#define TSFI_CPM_TOKEN_SENSING_H_BRIDGE_EXT_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_token_sensing_h_bridge_ext_init(uint32_t sensor_bus_id);
bool tsfi_cpm_token_sensing_h_bridge_ext_eval(uint32_t gpt_sol_token, uint32_t fable_token, uint32_t cpm_token, uint32_t tomie_token, uint64_t *crosstalk_latch_out);
bool tsfi_cpm_token_sensing_h_bridge_ext_render_renderman(uint64_t crosstalk_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_TOKEN_SENSING_H_BRIDGE_EXT_H */

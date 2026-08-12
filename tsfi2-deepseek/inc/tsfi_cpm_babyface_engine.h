/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_BABYFACE_ENGINE_H
#define TSFI_CPM_BABYFACE_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_cpm_babyface_engine_init(uint32_t babyface_id);
bool tsfi_cpm_babyface_engine_eval(double eye_radius, double nose_radius, double smile_curve, uint64_t *zmm_babyface_latch_out);
bool tsfi_cpm_babyface_engine_render_vram(uint64_t zmm_babyface_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_BABYFACE_ENGINE_H */

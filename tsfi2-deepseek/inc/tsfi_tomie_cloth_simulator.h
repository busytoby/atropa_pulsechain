/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_TOMIE_CLOTH_SIMULATOR_H
#define TSFI_TOMIE_CLOTH_SIMULATOR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_tomie_cloth_init(uint32_t garment_mesh_id, uint32_t spring_count);
bool tsfi_tomie_cloth_step_fet_discharge(uint32_t fet_cycle_id, float stiffness, uint64_t *zmm_cloth_latch_out);
bool tsfi_tomie_cloth_render_smpl_garment(uint64_t zmm_cloth_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_TOMIE_CLOTH_SIMULATOR_H */

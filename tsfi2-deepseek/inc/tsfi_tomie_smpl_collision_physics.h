/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_TOMIE_SMPL_COLLISION_PHYSICS_H
#define TSFI_TOMIE_SMPL_COLLISION_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_tomie_collision_init(uint32_t max_bodies);
bool tsfi_tomie_collision_step_fet_discharge(uint32_t fet_cycle_id, uint32_t body_a, uint32_t body_b, uint64_t *zmm_collision_latch_out);
bool tsfi_tomie_collision_resolve(uint64_t zmm_collision_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_TOMIE_SMPL_COLLISION_PHYSICS_H */

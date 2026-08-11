/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_TOMIE_SMPL_DEFORMATION_PHYSICS_H
#define TSFI_TOMIE_SMPL_DEFORMATION_PHYSICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

bool tsfi_tomie_smpl_fet_discharge_solver(uint32_t fet_cycle_id, float power_cut_ratio, uint64_t *zmm_deform_latch_out);
bool tsfi_tomie_smpl_apply_soft_body_verlet(uint32_t vertex_count, uint64_t zmm_deform_latch, uint64_t vram_phys_addr);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_TOMIE_SMPL_DEFORMATION_PHYSICS_H */

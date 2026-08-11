/* SPDX-License-Identifier: GPL-2.0 */
#ifndef TSFI_CPM_SUBPIPELINES_H
#define TSFI_CPM_SUBPIPELINES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 1. tsfi_cpm_neural_mocap_rig: Skeletal retargeting, 24-joint SMPL skin binding, DeepSeek MoE routing */
bool tsfi_cpm_neural_mocap_rig_init(uint32_t rig_id);
bool tsfi_cpm_neural_mocap_rig_eval(uint32_t joint_mask, uint32_t expert_id, uint64_t *zmm_rig_latch_out);

/* 2. tsfi_cpm_fet_surface_physics: Cloth, collision, hair strands, and FET discharge solvers */
bool tsfi_cpm_fet_surface_physics_init(uint32_t solver_id);
bool tsfi_cpm_fet_surface_physics_eval(uint32_t surface_flags, uint64_t *zmm_fet_latch_out);

/* 3. tsfi_cpm_volumetric_optics: SSS, Rayleigh-Mie fog, photon caustics, Spherical Harmonics lighting */
bool tsfi_cpm_volumetric_optics_init(uint32_t optics_id);
bool tsfi_cpm_volumetric_optics_eval(uint32_t optics_flags, uint64_t *zmm_optics_latch_out);

/* 4. tsfi_cpm_pbr_renderman_pipeline: Disney BSDF, AI denoising, 3D motion blur/DOF, RenderMan RIS */
bool tsfi_cpm_pbr_renderman_pipeline_init(uint32_t pipeline_id);
bool tsfi_cpm_pbr_renderman_pipeline_eval(uint32_t pbr_flags, uint64_t *zmm_pbr_latch_out);
bool tsfi_cpm_pbr_renderman_pipeline_render(uint64_t zmm_pbr_latch, uint64_t vram_phys_addr);

/* 5. tsfi_cpm_algol_display_vector: 1961 VM ALGOL Display Vector stack frame scoping */
bool tsfi_cpm_algol_display_vector_init(uint32_t scope_level);
bool tsfi_cpm_algol_display_vector_eval(uint32_t stack_depth, uint64_t *zmm_ingerman_latch_out);

#ifdef __cplusplus
}
#endif

#endif /* TSFI_CPM_SUBPIPELINES_H */

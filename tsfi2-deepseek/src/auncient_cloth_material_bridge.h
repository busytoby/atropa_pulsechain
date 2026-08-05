#ifndef AUNCIENT_CLOTH_MATERIAL_BRIDGE_H
#define AUNCIENT_CLOTH_MATERIAL_BRIDGE_H

#include "cloth_simulator.h"
#include "auncient_vulkan_materials.h"

// Auncient Cloth Simulator to Vulkan Materials Integration Bridge

#ifdef __cplusplus
extern "C" {
#endif

// Maps the material template's color preset to the cloth mesh vertex colors dynamically
void auncient_bridge_material_to_cloth_color(const MaterialUniformBlock *mat_block, ClothVertex *vertices, int count);

// Links physics properties (wind vector force) to update instance uniforms
void auncient_bridge_cloth_to_instance(float wind_x, float wind_y, float wind_z, InstanceUniformBlock *inst_block);

// Runs the cloth simulation update step using parameters driven by the material template settings
void auncient_bridge_update_cloth_physics(const MaterialUniformBlock *mat_block, float global_time, float base_wind_x, float base_wind_y, float base_wind_z);

#include "../inc/tsfi_usdshade.h"

// Maps the teddy bear .dna material seed to a deterministic Social Security number and Area Lot creation site
void auncient_bridge_dna_to_ssa(const MaterialUniformBlock *mat_block, char *ssn_out, char *site_out, int max_len);

// Maps the teddy bear USD material footprint to a deterministic Social Security number and Area Lot creation site
void auncient_bridge_usd_to_ssa(const TSFiUsdShade *usd_shade, char *ssn_out, char *site_out, int max_len);

// Maps a singular entity name (e.g. TeddyBearSkelCharacter) to a deterministic Social Security number and Area Lot creation site
void auncient_bridge_entity_to_ssa(const char *entity_name, char *ssn_out, char *site_out, int max_len);

#ifdef __cplusplus
}
#endif

#endif // AUNCIENT_CLOTH_MATERIAL_BRIDGE_H

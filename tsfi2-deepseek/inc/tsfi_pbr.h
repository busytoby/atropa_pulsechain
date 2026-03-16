#ifndef TSFI_PBR_H
#define TSFI_PBR_H

#include <math.h>
#include <stdint.h>
#include "tsfi_c_math.h"

typedef struct { float x, y, z; } Vector3;
typedef struct { float r, g, b; } Color3;

// --- Physical Constants ---
#define TSFI_TAU (2 * TSFI_SECRET_CORE)

// --- PBR Look-Up Tables (LUTs) ---
// 64x64 grid for Roughness vs NdotV
#define TSFI_PBR_LUT_SIZE 64

typedef struct {
    float energy_comp[TSFI_PBR_LUT_SIZE][TSFI_PBR_LUT_SIZE]; // GGX Energy Compensation
    float fresnel_dfg[TSFI_PBR_LUT_SIZE][TSFI_PBR_LUT_SIZE]; // Precomputed DFG
} TSFiPbrLut;

// Global PBR Matrix
extern TSFiPbrLut *g_pbr_matrix;

void tsfi_pbr_init_luts(void);

// --- Unified PBR BRDF (Basic) ---
Color3 tsfi_pbr_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, Color3 albedo, float roughness, float metallic, float anisotropic);

// --- Ultra PBR BRDF (Multi-Lobe) ---
Color3 tsfi_ultra_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, Color3 albedo, float roughness, float metallic, float anisotropic, float sheen, float clearcoat, float clearcoat_roughness);

// --- Sovereign PBR BRDF (Energy Conserving + Iridescent) ---
// Additional Parameters:
// - iridescence: [0, 1] Thin-film interference shift
Color3 tsfi_sovereign_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, Color3 albedo, float roughness, float metallic, float anisotropic, float sheen, float clearcoat, float iridescence);

#endif // TSFI_PBR_H

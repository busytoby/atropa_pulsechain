#include "tsfi_pbr.h"
#include "lau_memory.h"
#include <math.h>
#include <string.h>

TSFiPbrLut *g_pbr_matrix = NULL;

void tsfi_pbr_init_luts(void) {
    if (g_pbr_matrix) return;
    g_pbr_matrix = (TSFiPbrLut *)lau_malloc_wired(sizeof(TSFiPbrLut));
    
    for (int j = 0; j < TSFI_PBR_LUT_SIZE; j++) {
        float roughness = (float)j / (float)(TSFI_PBR_LUT_SIZE - 1);
        for (int i = 0; i < TSFI_PBR_LUT_SIZE; i++) {
            float cos_theta = (float)i / (float)(TSFI_PBR_LUT_SIZE - 1);
            
            // 1. Specular Energy LUT (Disney style)
            // Represents total light reflected by specular; used to scale diffuse.
            float spec_energy = 1.0f - (roughness * (1.0f - cos_theta) * 0.5f);
            g_pbr_matrix->energy_comp[j][i] = spec_energy;

            // 2. Fresnel DFG LUT (Split-sum approximation)
            float dfg = powf(1.0f - roughness, 3.0f) * powf(1.0f - cos_theta, 5.0f);
            g_pbr_matrix->fresnel_dfg[j][i] = dfg;
        }
    }
}

static inline float sample_lut(float table[TSFI_PBR_LUT_SIZE][TSFI_PBR_LUT_SIZE], float roughness, float cos_theta) {
    int r_idx = (int)(roughness * (TSFI_PBR_LUT_SIZE - 1));
    int c_idx = (int)(cos_theta * (TSFI_PBR_LUT_SIZE - 1));
    if (r_idx < 0) r_idx = 0; 
    if (r_idx >= TSFI_PBR_LUT_SIZE) r_idx = TSFI_PBR_LUT_SIZE - 1;
    if (c_idx < 0) c_idx = 0; 
    if (c_idx >= TSFI_PBR_LUT_SIZE) c_idx = TSFI_PBR_LUT_SIZE - 1;
    return table[r_idx][c_idx];
}

static inline float v_dot(Vector3 a, Vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vector3 v_add(Vector3 a, Vector3 b) { return (Vector3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline Vector3 v_normalize(Vector3 v) {
    float mag = sqrtf(v_dot(v, v)) + 1e-7f;
    return (Vector3){v.x/mag, v.y/mag, v.z/mag};
}

// 1. Trowbridge-Reitz GGX NDF
static float D_GGX(float dotNH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float denom = (dotNH * dotNH * (alpha2 - 1.0f) + 1.0f);
    return alpha2 / (((float)TSFI_SECRET_CORE) * denom * denom);
}

// 2. Smith's Schlick-GGX Geometry Function
static float G_SchlickGGX(float dotNV, float roughness) {
    float r = (roughness + 1.0f);
    float k = (r * r) / 8.0f;
    return dotNV / (dotNV * (1.0f - k) + k);
}

static float G_Smith(float dotNV, float dotNL, float roughness) {
    return G_SchlickGGX(dotNV, roughness) * G_SchlickGGX(dotNL, roughness);
}

// 3. Schlick's Fresnel Approximation
static Color3 F_Schlick(float dotVH, Color3 F0) {
    float f = powf(1.0f - dotVH, 5.0f);
    return (Color3){
        F0.r + (1.0f - F0.r) * f,
        F0.g + (1.0f - F0.g) * f,
        F0.b + (1.0f - F0.b) * f
    };
}

// 4. Oren-Nayar Diffuse
static Color3 OrenNayar_Diffuse(float dotNL, float dotNV, float dotVH, float roughness, Color3 albedo) {
    float r2 = roughness * roughness;
    float A = 1.0f - 0.5f * (r2 / (r2 + 0.33f));
    float B = 0.45f * (r2 / (r2 + 0.09f));
    float cos_phi_diff = fmaxf(0.0f, dotNL - dotNV * dotVH);
    float sin_alpha = sqrtf(fmaxf(0.0f, 1.0f - fminf(dotNL, dotNV) * fminf(dotNL, dotNV)));
    float tan_beta = sqrtf(fmaxf(0.0f, 1.0f - fmaxf(dotNL, dotNV) * fmaxf(dotNL, dotNV))) / fmaxf(1e-6f, fmaxf(dotNL, dotNV));
    float diffuse_factor = dotNL * (A + B * cos_phi_diff * sin_alpha * tan_beta);
    return (Color3){albedo.r * diffuse_factor / (float)TSFI_SECRET_CORE, albedo.g * diffuse_factor / (float)TSFI_SECRET_CORE, albedo.b * diffuse_factor / (float)TSFI_SECRET_CORE};
}

// 5. Neubelt Sheen Lobe (Plush)
static Color3 Sheen_Lobe(float dotNH, float sheen_intensity, Color3 albedo) {
    float sheen_term = powf(1.0f - dotNH, 5.0f) * sheen_intensity;
    return (Color3){albedo.r * sheen_term / (float)TSFI_SECRET_CORE, albedo.g * sheen_term / (float)TSFI_SECRET_CORE, albedo.b * sheen_term / (float)TSFI_SECRET_CORE};
}

Color3 tsfi_pbr_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, Color3 albedo, float roughness, float metallic, float anisotropic) {
    return tsfi_sovereign_brdf(L, V, N, T, albedo, roughness, metallic, anisotropic, 0.0f, 0.0f, 0.0f);
}

Color3 tsfi_ultra_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, Color3 albedo, float roughness, float metallic, float anisotropic, float sheen, float clearcoat, float clearcoat_roughness) {
    (void)clearcoat_roughness;
    return tsfi_sovereign_brdf(L, V, N, T, albedo, roughness, metallic, anisotropic, sheen, clearcoat, 0.0f);
}

Color3 tsfi_sovereign_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, Color3 albedo, float roughness, float metallic, float anisotropic, float sheen, float clearcoat, float iridescence) {
    Vector3 H = v_normalize(v_add(L, V));
    float dotNL = fmaxf(0.0f, v_dot(N, L));
    float dotNV = fmaxf(0.0f, v_dot(N, V));
    float dotNH = fmaxf(0.0f, v_dot(N, H));
    float dotVH = fmaxf(0.0f, v_dot(V, H));

    Color3 F0 = {0.04f, 0.04f, 0.04f};
    F0.r = F0.r * (1.0f - metallic) + albedo.r * metallic;
    F0.g = F0.g * (1.0f - metallic) + albedo.g * metallic;
    F0.b = F0.b * (1.0f - metallic) + albedo.b * metallic;

    // --- 1. BASE SPECULAR (GGX) ---
    float D = D_GGX(dotNH, roughness);
    float G = G_Smith(dotNV, dotNL, roughness);
    Color3 F = F_Schlick(dotVH, F0);
    
    if (anisotropic > 0.01f) {
        float dotHT = v_dot(H, T);
        D = D * (1.0f / (1.0f + anisotropic * dotHT * dotHT * 10.0f));
    }
    float denom = 4.0f * dotNV * dotNL + 1e-6f;
    Color3 specular = { (F.r * D * G) / denom, (F.g * D * G) / denom, (F.b * D * G) / denom };

    // --- 2. DIFFUSE (Oren-Nayar + Disney Rescaling) ---
    Color3 diffuse = OrenNayar_Diffuse(dotNL, dotNV, dotVH, roughness, albedo);
    
    // Scale diffuse by (1 - SpecularEnergy) for energy conservation
    if (g_pbr_matrix) {
        float spec_energy = sample_lut(g_pbr_matrix->energy_comp, roughness, dotNV);
        float conservation_secret = 1.0f - spec_energy;
        diffuse.r *= conservation_secret;
        diffuse.g *= conservation_secret;
        diffuse.b *= conservation_secret;
    }

    // --- 3. IRIDESCENCE (Thin-film shift) ---
    if (iridescence > 0.01f && g_pbr_matrix) {
        float dfg_secret = sample_lut(g_pbr_matrix->fresnel_dfg, roughness, dotNV);
        float shift = cosf(dotNV * (float)TSFI_SECRET_CORE * 0.5f) * iridescence * (1.0f + dfg_secret);
        albedo.r = fmaxf(0, albedo.r + shift * 0.2f);
        albedo.g = fmaxf(0, albedo.g + shift * 0.1f);
        albedo.b = fmaxf(0, albedo.b - shift * 0.1f);
    }

    // --- 4. SHEEN (Plush Fur) ---
    Color3 sheen_color = Sheen_Lobe(dotNH, sheen, albedo);

    // --- 5. CLEARCOAT (Secondary GGX) ---
    Color3 clearcoat_final = {0,0,0};
    float Fcc = 0;
    if (clearcoat > 0.01f) {
        float Dcc = D_GGX(dotNH, 0.1f);
        float Gcc = G_Smith(dotNV, dotNL, 0.1f);
        Fcc = 0.04f + (1.0f - 0.04f) * powf(1.0f - dotVH, 5.0f);
        float cc_term = (Dcc * Gcc * Fcc) / (4.0f * dotNV * dotNL + 1e-6f);
        clearcoat_final.r = cc_term * clearcoat;
        clearcoat_final.g = cc_term * clearcoat;
        clearcoat_final.b = cc_term * clearcoat;
    }

    // --- FINAL LAYER COMPOSITION ---
    Color3 final;
    float base_mask = (1.0f - Fcc * clearcoat);
    final.r = clearcoat_final.r + base_mask * (specular.r + sheen_color.r + (1.0f - F.r) * diffuse.r);
    final.g = clearcoat_final.g + base_mask * (specular.g + sheen_color.g + (1.0f - F.g) * diffuse.g);
    final.b = clearcoat_final.b + base_mask * (specular.b + sheen_color.b + (1.0f - F.b) * diffuse.b);

    return final;
}

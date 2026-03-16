#include "tsfi_hair.h"
#include "tsfi_c_math.h"
#include <math.h>

static inline float v_dot(Vector3 a, Vector3 b) { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline Vector3 v_normalize(Vector3 v) {
    float mag = sqrtf(v_dot(v, v)) + 1e-7f;
    return (Vector3){v.x/mag, v.y/mag, v.z/mag};
}

// Marschner-style Longitudinal Scattering Function (M)
static float longitudinal_scattering(float sin_theta, float target_sin, float roughness) {
    float v = roughness * roughness;
    float diff = sin_theta - target_sin;
    return expf(-diff * diff / (2.0f * v + 1e-6f)) / (sqrtf((float)TSFI_TAU * v) + 1e-6f);
}

Color3 tsfi_hair_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, float roughness, Color3 albedo) {
    float sin_theta_l = v_dot(T, L);
    float sin_theta_v = v_dot(T, V);
    float cos_theta_l = sqrtf(fmaxf(0, 1.0f - sin_theta_l * sin_theta_l));

    // 1. R Lobe (Primary Reflection)
    float alpha_r = -0.05f; 
    float M_r = longitudinal_scattering(sin_theta_l + sin_theta_v, alpha_r, roughness);
    
    // 2. TT Lobe (Transmission)
    float alpha_tt = -alpha_r * 0.5f;
    float M_tt = longitudinal_scattering(sin_theta_l + sin_theta_v, alpha_tt, roughness * 0.5f);
    
    // 3. TRT Lobe (Internal Reflection)
    float alpha_trt = -alpha_r * 2.0f;
    float M_trt = longitudinal_scattering(sin_theta_l + sin_theta_v, alpha_trt, roughness * 2.0f);

    float cos_phi = v_dot(v_normalize(N), v_normalize(L));
    float N_dot_L = fmaxf(0.1f, cos_phi);

    float mfs_secret = 0.2f * (1.0f - N_dot_L) * albedo.r;

    Color3 final;
    float common_denom = fmaxf(1e-6f, cos_theta_l * cos_theta_l);
    float spec = (M_r + M_tt * 0.5f + M_trt * 0.2f) / common_denom;
    
    final.r = albedo.r * (spec * N_dot_L + mfs_secret);
    final.g = albedo.g * (spec * N_dot_L + mfs_secret);
    final.b = albedo.b * (spec * N_dot_L + mfs_secret);

    return final;
}

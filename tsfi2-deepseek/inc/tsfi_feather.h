#ifndef TSFI_FEATHER_H
#define TSFI_FEATHER_H

#include "tsfi_pbr.h"
#include "tsfi_vector_utils.h"

/**
 * --- TSFi Feather BRDF ---
 * Distinguishes Feathers from Fur via Anisotropic Specular Planes.
 * Implements Thin-Film Interference for the Crow's iridescent sheen.
 */
static inline Color3 tsfi_feather_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, float irid, float spec) {
    float NdotL = fmaxf(0.0f, v_dot(N, L));
    float NdotV = fmaxf(0.0f, v_dot(N, V));
    
    // Anisotropic Specular (Plated Reflection)
    Vector3 H = v_normalize((Vector3){L.x + V.x, L.y + V.y, L.z + V.z});
    float TdotH = v_dot(T, H);
    float anisotropic_spec = powf(sqrtf(1.0f - TdotH * TdotH), 64.0f);
    
    // Thin-Film Interference (Iridescence)
    float interference = sinf(NdotV * 10.0f + irid * (float)TSFI_TAU) * 0.5f + 0.5f;
    Color3 irid_color = {interference * 0.5f, 0.0f, interference * 1.0f}; // Violet/Cyan shift
    
    Color3 result;
    result.r = (0.01f + anisotropic_spec * spec) + irid_color.r * 0.2f;
    result.g = (0.01f + anisotropic_spec * spec) + irid_color.g * 0.2f;
    result.b = (0.02f + anisotropic_spec * spec) + irid_color.b * 0.4f;
    
    result.r *= NdotL; result.g *= NdotL; result.b *= NdotL;
    return result;
}

#endif

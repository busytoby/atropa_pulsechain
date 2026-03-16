#ifndef TSFI_HAIR_H
#define TSFI_HAIR_H

#include "tsfi_pbr.h"

// --- Marschner-Burley Hybrid Hair BRDF ---
// Tailored for photorealistic teddy bear plush.
Color3 tsfi_hair_brdf(Vector3 L, Vector3 V, Vector3 N, Vector3 T, float roughness, Color3 albedo);

#endif // TSFI_HAIR_H

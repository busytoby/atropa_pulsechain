#ifndef TSFI_SHADING_MANIFOLD_H
#define TSFI_SHADING_MANIFOLD_H

#include <stdint.h>
#include <stddef.h>
#include "tsfi_pbr.h"

// --- TSFi Packed Shading Manifold ---
// Optimized 256 KiB texture packing for minimal photorealism.
// R: Normal.x, G: Normal.y, B: Ambient Occlusion, A: Rim Light

#define SHADING_MAP_RES 256
#define SHADING_MAP_SIZE (SHADING_MAP_RES * SHADING_MAP_RES * 4)

typedef struct {
    uint8_t *data; // Packed RGBA8 buffer
    size_t size;
} TsfiShadingManifold;

#ifdef __cplusplus
extern "C" {
#endif

void tsfi_shading_manifold_init(void);
void tsfi_shading_manifold_teardown(void);

// Returns a normalized [0, 1] color sample from the packed manifold
Color3 tsfi_sample_manifold_rgb(float u, float v);
float  tsfi_sample_manifold_rim(float u, float v);

#ifdef __cplusplus
}
#endif

#endif

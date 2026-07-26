#ifndef TSFI_ENVMAP_H
#define TSFI_ENVMAP_H

#include <stdint.h>

typedef struct {
    int width;
    int height;
    const uint8_t *texture_data;
} TSFiEnvMap;

// Initializes the environment map with dimensions and backing texture data
void tsfi_envmap_init(TSFiEnvMap *env, int width, int height, const uint8_t *texture_data);

// Samples a texture value from the environment map using normal direction vector coordinates
uint8_t tsfi_envmap_sample(const TSFiEnvMap *env, double nx, double ny, double nz);

#endif // TSFI_ENVMAP_H

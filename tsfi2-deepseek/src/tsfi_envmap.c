#include "tsfi_envmap.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void tsfi_envmap_init(TSFiEnvMap *env, int width, int height, const uint8_t *texture_data) {
    if (!env) return;
    env->width = width > 0 ? width : 1;
    env->height = height > 0 ? height : 1;
    env->texture_data = texture_data;
}

uint8_t tsfi_envmap_sample(const TSFiEnvMap *env, double nx, double ny, double nz) {
    if (!env || !env->texture_data) return 0;
    
    // Normalize input vectors to protect against out-of-bounds math
    double length = sqrt(nx * nx + ny * ny + nz * nz);
    if (length < 1e-6) {
        nx = 0.0;
        ny = 1.0;
        nz = 0.0;
    } else {
        nx /= length;
        ny /= length;
        nz /= length;
    }
    
    // Calculate equirectangular U/V spherical coordinate reflections
    double u = 0.5 + atan2(ny, nx) / (2.0 * M_PI);
    double v = 0.5 - asin(nz) / M_PI;
    
    // Clamp to UV texture boundaries
    int tx = (int)(u * env->width);
    int ty = (int)(v * env->height);
    
    if (tx < 0) tx = 0;
    if (tx >= env->width) tx = env->width - 1;
    if (ty < 0) ty = 0;
    if (ty >= env->height) ty = env->height - 1;
    
    return env->texture_data[ty * env->width + tx];
}

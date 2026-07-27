#include "tsfi_fbm_landscape.h"
#include <math.h>

static uint8_t hash_noise(int x, int y, uint32_t seed) {
    uint32_t h = (uint32_t)x * 374761393U + (uint32_t)y * 668265263U + seed;
    h = (h ^ (h >> 13)) * 12741261U;
    return (uint8_t)(h & 0xFF);
}

static double noise_2d(double x, double y, uint32_t seed) {
    int x0 = (int)floor(x);
    int y0 = (int)floor(y);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    double dx = x - x0;
    double dy = y - y0;
    
    double v00 = (double)hash_noise(x0, y0, seed) / 255.0;
    double v10 = (double)hash_noise(x1, y0, seed) / 255.0;
    double v01 = (double)hash_noise(x0, y1, seed) / 255.0;
    double v11 = (double)hash_noise(x1, y1, seed) / 255.0;
    
    // Linear interpolation
    double i1 = v00 * (1.0 - dx) + v10 * dx;
    double i2 = v01 * (1.0 - dx) + v11 * dx;
    
    return i1 * (1.0 - dy) + i2 * dy;
}

void tsfi_fbm_landscape_init(TSFiFbmLandscape *land, int octaves, double lacunarity, double persistence, uint32_t seed) {
    if (!land) return;
    land->octaves = octaves;
    land->lacunarity = lacunarity;
    land->persistence = persistence;
    land->seed = seed;
}

double tsfi_fbm_landscape_eval(const TSFiFbmLandscape *land, double x, double y) {
    if (!land) return 0.0;
    
    double value = 0.0;
    double amplitude = 1.0;
    double frequency = 1.0;
    double max_amplitude = 0.0;
    
    for (int i = 0; i < land->octaves; i++) {
        value += noise_2d(x * frequency, y * frequency, land->seed + i * 100U) * amplitude;
        max_amplitude += amplitude;
        frequency *= land->lacunarity;
        amplitude *= land->persistence;
    }
    
    return value / max_amplitude;
}

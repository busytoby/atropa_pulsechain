#ifndef TSFI_TEXGEN_H
#define TSFI_TEXGEN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    char glyph;
} color_pixel_t;

// Standardized parameters under 200 bytes limit (Size: 68 bytes)
typedef struct {
    uint32_t seed;
    float phase_scale;
    float twirl_strength;
    uint8_t blend_mode; // 1: Multiply, 2: Difference, 3: Screen, 4: Add, 5: Subtract
    float light_x;
    float light_y;
    float light_z;
    float persistence;  // Noise octave amplitude scale
    float lacunarity;   // Noise octave frequency scale
    uint8_t octaves;    // Number of noise octaves
    float brightness;   // Color adjustments
    float contrast;     // Color adjustments
    uint8_t wave_type;  // 0: Sine, 1: Sawtooth, 2: Square
    float feedback_strength; // Recursive coordinate feedback scale
    uint8_t clut_preset;     // Color lookup table selector
    float vignette_strength; // Vignette boundary darkening scale
    float threshold_limit;   // Binarization limit (0 to 255, 0 to disable)
    float shell_ao;          // Shell depth ambient occlusion scale
    float anisotropy;        // Anisotropic highlight strength (0 to 1)
} tsfi_texgen_params_t;

void tsfi_texgen_init(void);
double tsfi_texgen_sin(double angle);
double tsfi_texgen_cos(double angle);
void tsfi_texgen_render(uint8_t *output_rgba, int width, int height, double phase, int mode, const tsfi_texgen_params_t *params);

#endif

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

typedef struct {
    uint32_t seed;
    float phase_scale;
    float twirl_strength;
    uint8_t blend_mode;
    float light_x;
    float light_y;
    float light_z;
} tsfi_texgen_params_t;

void tsfi_texgen_init(void);
double tsfi_texgen_sin(double angle);
double tsfi_texgen_cos(double angle);
void tsfi_texgen_render(uint8_t *output_rgb, int width, int height, double phase, int mode, const tsfi_texgen_params_t *params);

#endif

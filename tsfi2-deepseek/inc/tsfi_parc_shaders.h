#ifndef TSFI_PARC_SHADERS_H
#define TSFI_PARC_SHADERS_H

#include <stdint.h>

typedef struct {
    float viewport_width;
    float viewport_height;
    float cursor_x;
    float cursor_y;
    float out_color_r;
    float out_color_g;
    float out_color_b;
} tsfi_parc_shader_params_t;

/* Parse the Algol61 vertex shader representation and COBOL strategy fragment shader to load configuration */
int tsfi_parc_load_shaders(const char *algol_path, const char *strategy_path, tsfi_parc_shader_params_t *params);

#endif // TSFI_PARC_SHADERS_H

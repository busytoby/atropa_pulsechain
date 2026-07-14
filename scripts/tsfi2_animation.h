#ifndef TSFI2_ANIMATION_H
#define TSFI2_ANIMATION_H

#include <stdint.h>

// Canvas structure holding raw RGB24 buffer
typedef struct {
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} tsfi2_canvas_t;

// Vector 3D structure
typedef struct {
    float x;
    float y;
    float z;
} tsfi2_vec3_t;

// Vector 4D structure
typedef struct {
    float x;
    float y;
    float z;
    float w;
} tsfi2_vec4_t;

// Canvas Memory Management
tsfi2_canvas_t *tsfi2_canvas_create(uint32_t width, uint32_t height);
void tsfi2_canvas_clear(tsfi2_canvas_t *canvas, uint8_t r, uint8_t g, uint8_t b);
void tsfi2_canvas_free(tsfi2_canvas_t *canvas);

// Drawing Rasterization Functions
void tsfi2_draw_pixel(tsfi2_canvas_t *canvas, int x, int y, uint8_t r, uint8_t g, uint8_t b, float alpha);
void tsfi2_draw_line_aa(tsfi2_canvas_t *canvas, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b, float alpha);
void tsfi2_draw_line_glow(tsfi2_canvas_t *canvas, float x0, float y0, float x1, float y1, uint8_t r, uint8_t g, uint8_t b);
void tsfi2_draw_circle(tsfi2_canvas_t *canvas, int cx, int cy, int radius, uint8_t r, uint8_t g, uint8_t b, int fill);

// Easing Functions for Animation
float tsfi2_ease_linear(float t);
float tsfi2_ease_slow_in_out(float t);
float tsfi2_ease_elastic(float t);

// 3D/4D Projection Mathematics
void tsfi2_rotate_4d(tsfi2_vec4_t *v, float theta, float phi, tsfi2_vec4_t *out);
void tsfi2_project_4d_to_3d(const tsfi2_vec4_t *v, tsfi2_vec3_t *out);
void tsfi2_project_3d_to_2d(const tsfi2_vec3_t *v, float angle, float scale, uint32_t width, uint32_t height, int *sx, int *sy);

#endif // TSFI2_ANIMATION_H

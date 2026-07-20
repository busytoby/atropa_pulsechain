#ifndef TSFI_PAINT_H
#define TSFI_PAINT_H

#include "tsfi_layout.h"

// Paints the layout box tree onto the pixel buffer
void tsfi_paint_tree(const TsfiLayoutBox *box, uint32_t *pixels, int width, int height);

/* Quantel Paintbox: Pressure-sensitive Airbrushing with standard gaussian distribution */
int tsfi_quantel_paintbox_airbrush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color);

/* Quantel Paintbox: Dynamic stencil masking logic mapping vector stencil regions */
int tsfi_quantel_paintbox_apply_stencil(uint32_t *pixels, int w, int h, const uint8_t *stencil_mask, uint32_t color);

/* Quantel Mirage: 3D DVE transforms implementing page curls on image segments */
int tsfi_quantel_mirage_page_curl(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float curl_radius, float roll_percent);

/* Quantel Harry: Multi-layer chroma key extraction */
int tsfi_quantel_harry_chroma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint32_t key_color, float tolerance);

/* Quantel Harry: Dynamic timeline compositing blend modes */
int tsfi_quantel_harry_blend(const uint32_t *fg_pixels, const uint32_t *bg_pixels, uint32_t *dst_pixels, int w, int h, const uint8_t *mask, const char *blend_mode);

/* Quantel Paintbox: Textured brush stencils drawing custom patterns */
int tsfi_quantel_paintbox_custom_brush(uint32_t *pixels, int w, int h, int cx, int cy, const uint8_t *brush_tex, int brush_w, int brush_h, float opacity, uint32_t color);

/* Quantel Mirage: Sphere warp coordinate mapping */
int tsfi_quantel_mirage_sphere_wrap(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float sphere_radius);

/* Quantel Mirage: Flag ripple animation warp */
int tsfi_quantel_mirage_flag_ripple(const uint32_t *src_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float amplitude, float frequency, float phase);

/* Quantel Harry: Wipe transitions between two frames */
int tsfi_quantel_harry_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, const char *wipe_type);

/* Quantel Harry: Luma key extraction based on luminance thresholds */
int tsfi_quantel_harry_luma_key(const uint32_t *src_pixels, int w, int h, uint8_t *out_mask, uint8_t low_threshold, uint8_t high_threshold);

/* Quantel Harry: 3x3 Convolution filtering (blur, sharpen) */
int tsfi_quantel_harry_filter(const uint32_t *src, uint32_t *dst, int w, int h, const float kernel[9]);

/* Quantel Harry: HSL color correction (hue, sat, tint) */
int tsfi_quantel_harry_color_adjust(const uint32_t *src, uint32_t *dst, int w, int h, float hue_shift, float sat_scale, uint32_t tint_color, float tint_amount);

/* Quantel Mirage: 3D Page Peel transition from frame A to B */
int tsfi_quantel_mirage_page_peel_transition(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, float peel_radius);

/* Quantel Paintbox: Dynamic velocity-based brush stroke drawing */
int tsfi_quantel_paintbox_velocity_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, float max_radius, float pressure, uint32_t color);

/* Quantel Paintbox: Wet paint canvas smudge color mixing */
int tsfi_quantel_paintbox_wet_paint(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float smudge_rate, uint32_t *brush_color_in_out);

/* Quantel Mirage: 3D DVE perspective projection rotation */
int tsfi_quantel_mirage_perspective_rotate(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float rot_z, float fov);

/* Quantel Harry: Keyframe animation translation along Quadratic Bezier motion paths */
int tsfi_quantel_harry_bezier_animate(const uint32_t *fg, int fg_w, int fg_h, uint32_t *bg, int w, int h, float t, float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y);

#endif // TSFI_PAINT_H





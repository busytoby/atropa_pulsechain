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

/* Quantel Paintbox: Calligraphy brush with tablet pressure and flow-rate scale mapping */
int tsfi_quantel_paintbox_tablet_brush(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, float aspect_ratio, float angle, uint32_t color);

/* Quantel Paintbox: Stylus tilt & barrel rotation brush shearing */
int tsfi_quantel_paintbox_stylus_shear(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float tilt_x, float tilt_y, float rotation, uint32_t color);

/* Quantel Paintbox: Exponential smoothing filter for tablet coordinate jitter removal */
int tsfi_quantel_paintbox_filter_jitter(int raw_x, int raw_y, int prev_x, int prev_y, float alpha, int *smooth_x, int *smooth_y);

/* Quantel Paintbox: Tablet color modulation with pressure-to-hue drift */
int tsfi_quantel_paintbox_pressure_color_mod(uint32_t base_color, float pressure, float hue_drift, float sat_drift, uint32_t *out_color);

/* Quantel Paintbox: Dynamic cubic Bezier curve interpolation along tablet points */
int tsfi_quantel_paintbox_bezier_stroke(uint32_t *pixels, int w, int h, float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y, int radius, float pressure, uint32_t color);

/* Quantel Paintbox: Pressure-gated background canvas paper texture reveal */
int tsfi_quantel_paintbox_texture_reveal(uint32_t *pixels, int w, int h, int cx, int cy, int radius, const uint8_t *paper_tex, int tex_w, int tex_h, float pressure, uint32_t color);

/* Quantel Harry: Color difference keyer (G-B Ultimatte-style) and edge feathering */
int tsfi_quantel_harry_color_difference_key(const uint32_t *src, int w, int h, uint8_t *out_mask, float k_factor, float b_factor, float feather_radius);

/* Quantel Harry: 2D Keyframe dynamic scaling, rotation, and opacity transition */
int tsfi_quantel_harry_keyframe_transform(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float progress, float start_scale, float end_scale, float start_rot, float end_rot, float start_alpha, float end_alpha);

/* Quantel Harry: Matrix grid tile wipe transition (M x N block sweeps) */
int tsfi_quantel_harry_matrix_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, int grid_m, int grid_n);

/* Quantel Storyboard: Composite multiple frames into a grid layout sheet */
int tsfi_quantel_storyboard_grid(const uint32_t **frames, int frame_count, int frame_w, int frame_h, uint32_t *dst_sheet, int sheet_w, int sheet_h, int rows, int cols);

/* Quantel Storyboard: Overlay SMPTE timecode burn-in (HH:MM:SS:FF) */
int tsfi_quantel_storyboard_timecode_burn(uint32_t *pixels, int w, int h, int frame_number, float fps, uint32_t text_color);

/* Quantel Storyboard: Onion skinning compositor (blends prev/next frames) */
int tsfi_quantel_storyboard_onion_skin(const uint32_t *prev_frame, const uint32_t *next_frame, uint32_t *active_canvas, int w, int h, float opacity_prev, float opacity_next);

#endif // TSFI_PAINT_H









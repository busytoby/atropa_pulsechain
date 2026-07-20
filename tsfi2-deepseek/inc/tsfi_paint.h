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

/* Quantel Mirage: Dual-sided page curl with distinct backside texture */
int tsfi_quantel_mirage_dual_sided_page_curl(const uint32_t *front_pixels, const uint32_t *back_pixels, int src_w, int src_h, uint32_t *dst_pixels, int dst_w, int dst_h, float curl_radius, float roll_percent);

/* Quantel Paintbox: Speed-sensitive custom brush spacing dynamics along stylus path */
int tsfi_quantel_paintbox_spacing_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, const uint8_t *brush_tex, int brush_w, int brush_h, float base_spacing, float opacity, uint32_t color);

/* Quantel Harry: Ripple wave displacement wipe transition between two frames */
int tsfi_quantel_harry_displacement_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, float wave_amplitude, float wave_frequency);

/* Quantel Paintbox: Rubber stamp cloning tool painting pixels from a source offset */
int tsfi_quantel_paintbox_clone(uint32_t *pixels, int w, int h, int cx, int cy, int src_dx, int src_dy, int radius, float opacity);

/* Quantel Mirage: 3D Freeform grid mesh warp transformation using bilinear grid mapping */
int tsfi_quantel_mirage_mesh_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, const float *grid_points_x, const float *grid_points_y, int grid_cols, int grid_rows);

/* Quantel Harry: Temporal slow-motion frame blending compositor */
int tsfi_quantel_harry_temporal_blend(const uint32_t *frame_a, const uint32_t *frame_b, uint32_t *dst, int w, int h, float blend_factor);

/* Quantel Harry: Chroma key green/blue spill suppression */
int tsfi_quantel_harry_spill_suppress(uint32_t *pixels, int w, int h, const char *suppress_type, float amount);

/* Quantel Mirage: 3D Torus warp coordinate projection mapping */
int tsfi_quantel_mirage_torus_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float inner_r, float outer_r);

/* Quantel Paintbox: Draw calligraphic vector shapes (lines, ellipses) using flat chisel stamps */
int tsfi_quantel_paintbox_calligraphy_shape(uint32_t *pixels, int w, int h, int start_x, int start_y, int end_x, int end_y, int max_radius, float pressure, float aspect_ratio, float angle, uint32_t color);

/* Quantel Paintbox: Paint bucket flood fill tool with color similarity tolerance */
int tsfi_quantel_paintbox_flood_fill(uint32_t *pixels, int w, int h, int start_x, int start_y, uint32_t fill_color, float tolerance);

/* Quantel Mirage: 3D Cone warp coordinate projection mapping */
int tsfi_quantel_mirage_cone_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float cone_height, float cone_radius);

/* Quantel Harry: Posterize color filter to reduce channel bits */
int tsfi_quantel_harry_posterize(uint32_t *pixels, int w, int h, int levels);

/* Quantel Harry: Solarize color filter to invert highlights/midtones */
int tsfi_quantel_harry_solarize(uint32_t *pixels, int w, int h, float threshold);

/* Quantel Paintbox: Pressure-sensitive noise spray can */
int tsfi_quantel_paintbox_spray_can(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, uint32_t color);

/* Quantel Mirage: 3D Cylinder warp coordinate projection mapping */
int tsfi_quantel_mirage_cylinder_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float cylinder_radius);

/* Quantel Harry: Brightness, contrast, and threshold adjustment filter */
int tsfi_quantel_harry_contrast_adjust(uint32_t *pixels, int w, int h, float brightness, float contrast);

/* Quantel Paintbox: Linear and radial gradient fills */
int tsfi_quantel_paintbox_gradient(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, uint32_t color_start, uint32_t color_end, const char *gradient_type);

/* Quantel Mirage: 3D Pyramid warp coordinate projection mapping */
int tsfi_quantel_mirage_pyramid_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float height, float base_size);

/* Quantel Harry: Color negative inverter filter */
int tsfi_quantel_harry_invert(uint32_t *pixels, int w, int h);

/* Quantel Palette Color Cycling */
int tsfi_quantel_paintbox_palette_cycle(uint32_t *pixels, int w, int h, int shift_amount);

/* Quantel Typographer overlay */
int tsfi_quantel_paintbox_typographer(uint32_t *pixels, int w, int h, int x, int y, const char *text, uint32_t color, float size);

/* Paintbox Brush Flow Dynamics (speed-sensitive flow-rate modulation) */
int tsfi_quantel_paintbox_flow_brush(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, int radius, float pressure, uint32_t color);

/* Mirage 3D Height-Map Extrusion */
int tsfi_quantel_mirage_height_map_extrude(const uint32_t *src, int w, int h, uint32_t *dst, float max_height, float scale);

/* Quantel Harry Lift, Gamma, Gain Color Grading */
int tsfi_quantel_harry_lift_gamma_gain(uint32_t *pixels, int w, int h, float lift[3], float gamma[3], float gain[3]);

/* Mirage 3D Flowing Ribbon Warp */
int tsfi_quantel_mirage_ribbon_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float frequency, float speed, float t);

/* Harry Rotoscope Layer Blend */
int tsfi_quantel_harry_rotoscope_blend(const uint32_t *canvas, const uint32_t *reference_frame, uint32_t *dst, int w, int h, float reference_opacity);

/* Storyboard Sheet Exporter */
int tsfi_quantel_storyboard_export_sheet(const uint32_t *sheet_pixels, int w, int h, const char *output_path);

/* Quantel Brush Pressure Jitter */
int tsfi_quantel_paintbox_pressure_jitter(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amount, uint32_t color);

/* Mirage 3D Cube Mapping */
int tsfi_quantel_mirage_cube_map(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float size);

/* Harry Multi-Track Masked Dissolve */
int tsfi_quantel_harry_multitrack_dissolve(const uint32_t *src_a, const uint32_t *src_b, const uint32_t *src_c, uint32_t *dst, int w, int h, const float weights[3]);

/* Storyboard Captions Overlay */
int tsfi_quantel_storyboard_burn_captions(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *scene, const char *take, const char *desc, uint32_t text_color);

/* Quantel Palette Mixer Pad */
int tsfi_quantel_paintbox_palette_mixer(uint32_t *mixer_pixels, int w, int h, int x, int y, uint32_t brush_color, float mix_rate);

/* Mirage 3D Concentric Ripple Warp */
int tsfi_quantel_mirage_concentric_ripple(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float wavelength, float speed, float t, float cx, float cy);

/* Harry Advanced Color Difference Keyer */
int tsfi_quantel_harry_advanced_difference_key(const uint32_t *src, int w, int h, uint8_t *out_mask, uint32_t target_color, float range_min, float range_max);

/* Storyboard Delta Frame Overlay */
int tsfi_quantel_storyboard_delta_overlay(const uint32_t *prev_frame, const uint32_t *next_frame, uint32_t *dst, int w, int h);

/* Quantel Brush Edge Softness Profile */
int tsfi_quantel_paintbox_profile_brush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float softness, uint32_t color);

/* Mirage 3D Twirl Vortex Warp */
int tsfi_quantel_mirage_twirl(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius, float cx, float cy);

/* Harry Matte Edge Erosion / Dilation */
int tsfi_quantel_harry_erode_dilate(const uint8_t *src_mask, uint8_t *dst_mask, int w, int h, int radius, int erode);

/* Storyboard SMPTE Drop-Frame Configurator */
int tsfi_quantel_storyboard_drop_frame_timecode(int frame_number, float fps, char *timecode_out, int max_len);

/* Quantel Brush Shear Jitter */
int tsfi_quantel_paintbox_shear_jitter(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amount, float base_rotation, uint32_t color);

/* Mirage 3D Accordion Sheet Fold Transition */
int tsfi_quantel_mirage_accordion_fold(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress, int folds);

/* Harry Matte HSL Despill */
int tsfi_quantel_harry_hsl_despill(uint32_t *pixels, int w, int h, float threshold_hue, float suppression_amount);

/* Storyboard Aspect Ratio Guides */
int tsfi_quantel_storyboard_aspect_guides(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *ratio_str, uint32_t color);

/* Quantel Brush Saturation Buildup */
int tsfi_quantel_paintbox_saturation_buildup(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float dwell_time, uint32_t color);

/* Mirage 3D Sphere Map Warp */
int tsfi_quantel_mirage_sphere_map(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_x, float rot_y, float radius);

/* Harry Chronological Time-Slice Wipe */
int tsfi_quantel_harry_time_slice_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress);

/* Storyboard Burn-In Frame Indexer */
int tsfi_quantel_storyboard_burn_index(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int frame_idx, float fps, uint32_t text_color);

/* Quantel Multi-Stop Gradient Builder */
int tsfi_quantel_paintbox_multistop_gradient(uint32_t *pixels, int w, int h, int x0, int y0, int x1, int y1, const uint32_t *colors, const float *stops, int stop_count);

/* Mirage 3D Page Peel Shadow Matte */
int tsfi_quantel_mirage_peel_shadow(uint32_t *pixels, int w, int h, float progress, float shadow_intensity);

/* Harry Split Screen Matte Preview */
int tsfi_quantel_harry_split_matte_preview(const uint32_t *composite, const uint8_t *matte, uint32_t *dst, int w, int h, float split_x);

/* Storyboard Production Slate Block */
int tsfi_quantel_storyboard_production_slate(uint32_t *pixels, int w, int h, const char *director, const char *project, const char *date);

#endif // TSFI_PAINT_H















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

/* Quantel Paintbox Bristle Brush Simulation */
int tsfi_quantel_paintbox_bristle_brush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, int bristle_count, uint32_t color);

/* Mirage 3D Cylindrical Wrap */
int tsfi_quantel_mirage_cylindrical_wrap(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float rot_y, float radius);

/* Harry Keyframe In-Between Interpolator */
int tsfi_quantel_harry_interpolate_keyframe(float t, float start_val, float end_val, float *out_val);

/* Storyboard Film Frame Borders */
int tsfi_quantel_storyboard_film_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color);

/* Quantel Paintbox Chalk/Dry Brush Simulation */
int tsfi_quantel_paintbox_chalk_brush(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color);

/* Mirage 3D Perspective Plane Tilt Warp */
int tsfi_quantel_mirage_plane_tilt(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float pitch, float yaw);

/* Storyboard Page Divider Layout */
int tsfi_quantel_storyboard_page_divider(uint32_t *pixels, int w, int h, int y_coord, uint32_t line_color);

/* Quantel Paintbox Dynamic Smudge Brush */
int tsfi_quantel_paintbox_dynamic_smudge(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float smudge_dryness, uint32_t *brush_color);

/* Mirage 3D Dynamic Flag Wave Warp */
int tsfi_quantel_mirage_flag_wave(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float frequency, float speed, float t);

/* Harry SMPTE-C to Rec709 Color Space Converter */
int tsfi_quantel_harry_smptec_to_rec709(uint32_t *pixels, int w, int h);

/* Storyboard Frame Thumbnail Shadow Borders */
int tsfi_quantel_storyboard_thumbnail_shadows(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h);

/* Quantel Paintbox Spray Can Velocity Scatter */
int tsfi_quantel_paintbox_velocity_spray(uint32_t *pixels, int w, int h, int prev_x, int prev_y, int curr_x, int curr_y, int radius, float pressure, uint32_t color);

/* Mirage 3D Coordinate Shear Warp */
int tsfi_quantel_mirage_coordinate_shear(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float shear_x, float shear_y);

/* Harry Chronological Color Balance Sweep */
int tsfi_quantel_harry_color_balance_sweep(uint32_t *pixels, int w, int h, float red_bal, float green_bal, float blue_bal);

/* Storyboard PDF Custom Title Annotation */
int tsfi_quantel_storyboard_annotate_title(uint32_t *pixels, int w, int h, int x, int y, const char *title, uint32_t color);

/* Quantel Paintbox Palette Auto-Sort & Selection */
int tsfi_quantel_paintbox_sort_palette(uint32_t *palette, int count, int sort_by_luma);

/* Mirage 3D Perspective Wave Grid Deformer */
int tsfi_quantel_mirage_wave_grid(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase);

/* Harry Sub-Pixel Frame Translation Shift */
int tsfi_quantel_harry_subpixel_shift(const uint32_t *src, uint32_t *dst, int w, int h, float dx, float dy);

/* Storyboard Frame Outline Border */
int tsfi_quantel_storyboard_outline_cell(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t outline_color);

/* Quantel Paintbox Linear/Radial Blend Mixer */
int tsfi_quantel_paintbox_blend_mixer(uint32_t *pixels, int w, int h, const uint32_t *src_a, const uint32_t *src_b, float mix_factor);

/* Mirage 3D Dynamic Sphere Spherize Warp */
int tsfi_quantel_mirage_spherize(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amount);

/* Harry Linear Chroma Gain Balance */
int tsfi_quantel_harry_chroma_gain(uint32_t *pixels, int w, int h, float gain);

/* Storyboard Frame Thumbnail Burn-In Label Annotation */
int tsfi_quantel_storyboard_burn_label(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, const char *label, uint32_t color);

/* Quantel Paintbox Palette Interpolator Mixer */
int tsfi_quantel_paintbox_interpolate_palette(const uint32_t *palette_a, const uint32_t *palette_b, uint32_t *dst_palette, int count, float t);

/* Mirage 3D Dynamic Twirl/Swirl Warp */
int tsfi_quantel_mirage_twirl_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius);

/* Harry Chronological Frame Interlacing */
int tsfi_quantel_harry_interlace_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h);

/* Storyboard Frame Thumbnail Grid Margin Spacer */
int tsfi_quantel_storyboard_grid_spacers(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t bg_color);

/* Quantel Paintbox Dynamic Airbrush Flow Rate */
int tsfi_quantel_paintbox_dynamic_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float speed, uint32_t color);

/* Mirage 3D Barrel Distortion Fish-Eye Warp */
int tsfi_quantel_mirage_fisheye_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float strength);

/* Harry Chronological Frame Dissolve Transition */
int tsfi_quantel_harry_dissolve(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress);

/* Storyboard Frame Thumbnail Grid Border Overlay */
int tsfi_quantel_storyboard_cell_overlay(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color, float alpha);

/* Quantel Paintbox Brush Angle Lock Dynamics */
int tsfi_quantel_paintbox_angle_lock(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float angle_lock, uint32_t color);

/* Mirage 3D Dynamic Ripple Deformer */
int tsfi_quantel_mirage_ripple_deform(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float amplitude, float frequency, float phase);

/* Harry Chronological Film Grain Overlay */
int tsfi_quantel_harry_film_grain(uint32_t *pixels, int w, int h, float intensity);

/* Storyboard Frame Thumbnail Grid Border Margins */
int tsfi_quantel_storyboard_border_margins(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int margin_w, uint32_t color);

/* Quantel Paintbox Palette Complementary Matcher */
int tsfi_quantel_paintbox_complementary_color(uint32_t color, uint32_t *out_color);

/* Mirage 3D Orthographic Pinch/Squeeze Warp */
int tsfi_quantel_mirage_pinch_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float factor);

/* Harry Chroma-Key Edge Matte Choker */
int tsfi_quantel_harry_matte_choke(const uint8_t *src_mask, uint8_t *dst_mask, int w, int h, int choke_radius);

/* Storyboard Frame Thumbnail Grid Corner Markers */
int tsfi_quantel_storyboard_corner_marks(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int mark_len, uint32_t color);

/* Quantel Paintbox Palette Warm/Cool Color Filter */
int tsfi_quantel_paintbox_warm_cool_filter(uint32_t *pixels, int w, int h, float temp_adjust);

/* Mirage 3D Dynamic Twirl Zoom Distortion Warp */
int tsfi_quantel_mirage_twirl_zoom(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float zoom_factor);

/* Harry Chronological Frame Wipe Transition (Clock Wipe) */
int tsfi_quantel_harry_clock_wipe(const uint32_t *src_a, const uint32_t *src_b, uint32_t *dst, int w, int h, float progress);

/* Storyboard Frame Thumbnail Grid Crosshairs Overlay */
int tsfi_quantel_storyboard_crosshairs(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t color);

/* Quantel Paintbox Palette Saturation Sweep */
int tsfi_quantel_paintbox_saturation_sweep(uint32_t *pixels, int w, int h, float saturation_adjust);

/* Mirage 3D Double Twirl Symmetry Warp */
int tsfi_quantel_mirage_double_twirl(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius);

/* Harry Chronological Frame Interlaced Field Splitter */
int tsfi_quantel_harry_split_fields(const uint32_t *src, uint32_t *field_even, uint32_t *field_odd, int w, int h);

/* Storyboard Frame Thumbnail Grid Outer Borders */
int tsfi_quantel_storyboard_outer_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color);

/* Mirage 3D Perspective Wave Grid Shift */
int tsfi_quantel_mirage_wave_grid_shift(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase, float shift_amount);

/* Harry Chronological Frame Interlaced Scan Line Offset */
int tsfi_quantel_harry_scanline_offset(uint32_t *pixels, int w, int h, int offset);

/* Storyboard Frame Thumbnail Grid Inner Borders */
int tsfi_quantel_storyboard_inner_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color);

/* Quantel Paintbox Palette Contrast Adjust Filter */
int tsfi_quantel_paintbox_contrast_adjust(uint32_t *pixels, int w, int h, float contrast);

/* Mirage 3D Dynamic Polar Warp */
int tsfi_quantel_mirage_polar_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float radius);

/* Harry Chronological Film Grain Color Overlay */
int tsfi_quantel_harry_color_film_grain(uint32_t *pixels, int w, int h, float intensity);

/* Storyboard Frame Thumbnail Grid Corner Outlines */
int tsfi_quantel_storyboard_corner_outlines(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int outline_len, uint32_t color);

/* Quantel Paintbox Brush Pressure Radius Dynamics */
int tsfi_quantel_paintbox_pressure_radius(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float pressure, uint32_t color);

/* Mirage 3D Dynamic Polar Inverse Warp */
int tsfi_quantel_mirage_polar_inverse_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float radius);

/* Harry Chronological Film Grain Monochromatic Overlay */
int tsfi_quantel_harry_mono_film_grain(uint32_t *pixels, int w, int h, float intensity);

/* Storyboard Frame Thumbnail Grid Border Highlights */
int tsfi_quantel_storyboard_border_highlights(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t highlight_color);

/* Quantel Paintbox Brush Velocity Radius Dynamics */
int tsfi_quantel_paintbox_velocity_radius(uint32_t *pixels, int w, int h, int cx, int cy, int max_radius, float speed, uint32_t color);

/* Mirage 3D Dynamic Polar Swirl Warp */
int tsfi_quantel_mirage_polar_swirl(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float radius, float swirl_angle);

/* Harry Chronological Frame Interlaced Field Interpolator */
int tsfi_quantel_harry_interpolate_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h);

/* Storyboard Frame Thumbnail Grid Double Borders */
int tsfi_quantel_storyboard_double_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color);

/* Quantel Paintbox Brush Velocity Flow Dynamics */
int tsfi_quantel_paintbox_velocity_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, uint32_t color);

/* Mirage 3D Perspective Plane Rotation */
int tsfi_quantel_mirage_plane_rotation(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float pitch, float yaw, float roll);

/* Harry Chronological Frame Interlaced Field Shift Offset */
int tsfi_quantel_harry_field_shift(uint32_t *pixels, int w, int h, int shift_even, int shift_odd);

/* Storyboard Frame Thumbnail Grid Dotted Borders */
int tsfi_quantel_storyboard_dotted_borders(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int dot_spacing, uint32_t border_color);

/* Quantel Paintbox Brush Pressure Flow Dynamics */
int tsfi_quantel_paintbox_pressure_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, uint32_t color);

/* Mirage 3D Dynamic Twirl/Swirl Inverse Warp */
int tsfi_quantel_mirage_twirl_inverse_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius);

/* Harry Chronological Frame Interlaced Field Blend */
int tsfi_quantel_harry_blend_fields(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor);

/* Storyboard Frame Thumbnail Grid Corner Spacers */
int tsfi_quantel_storyboard_corner_spacers(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int spacer_w, uint32_t color);

/* Quantel Paintbox Palette Auto-Sort By Green */
int tsfi_quantel_paintbox_sort_palette_green(uint32_t *palette, int size);

/* Mirage 3D Dynamic Flag Wave Shift Warp */
int tsfi_quantel_mirage_flag_wave_shift(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase, float amplitude);

/* Harry Chronological Frame Interlaced Scan Line Blend */
int tsfi_quantel_harry_scanline_blend(uint32_t *pixels, int w, int h, float blend_factor);

/* Storyboard Frame Thumbnail Grid Outer Borders With Corners */
int tsfi_quantel_storyboard_outer_borders_corners(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, uint32_t border_color, uint32_t corner_color);

/* Quantel Paintbox Brush Pressure Jitter Amplitude Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_amp(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Perspective Wave Grid Phase Shift Warp */
int tsfi_quantel_mirage_wave_grid_phase(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float phase_x, float phase_y);

/* Harry Chronological Frame Interlaced Field Blend Offset */
int tsfi_quantel_harry_blend_fields_offset(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset);

/* Storyboard Frame Thumbnail Grid Inner Borders Offset */
int tsfi_quantel_storyboard_inner_borders_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t border_color);

/* Quantel Paintbox Brush Pressure Saturation Filter */
int tsfi_quantel_paintbox_pressure_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float saturation, uint32_t color);

/* Mirage 3D Dynamic Twirl/Swirl Double Symmetry Warp */
int tsfi_quantel_mirage_twirl_double_symmetry(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius);

/* Harry Chronological Frame Interlaced Field Blend Shift Offset */
int tsfi_quantel_harry_blend_fields_shift(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int shift_even, int shift_odd);

/* Storyboard Frame Thumbnail Grid Double Borders Offset */
int tsfi_quantel_storyboard_double_borders_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t border_color);

/* Quantel Paintbox Brush Velocity Saturation Filter */
int tsfi_quantel_paintbox_velocity_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, float saturation, uint32_t color);

/* Mirage 3D Perspective Plane Pitch Warp */
int tsfi_quantel_mirage_plane_pitch(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float pitch);

/* Harry Chronological Frame Interlaced Field Split Interpolator */
int tsfi_quantel_harry_split_interpolate(const uint32_t *src, uint32_t *dst_even, uint32_t *dst_odd, int w, int h);

/* Storyboard Frame Thumbnail Grid Border Highlights Offset */
int tsfi_quantel_storyboard_border_highlights_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t highlight_color);

/* Quantel Paintbox Brush Pressure Jitter Flow Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Perspective Plane Yaw Warp */
int tsfi_quantel_mirage_plane_yaw(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float yaw);

/* Harry Chronological Frame Interlaced Field Blend Jitter */
int tsfi_quantel_harry_blend_fields_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, float jitter_amp);

/* Storyboard Frame Thumbnail Grid Dotted Borders Offset */
int tsfi_quantel_storyboard_dotted_borders_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int dot_spacing, uint32_t border_color);

/* Quantel Paintbox Brush Velocity Jitter Flow Dynamics */
int tsfi_quantel_paintbox_velocity_jitter_flow(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, float jitter_amp, uint32_t color);

/* Mirage 3D Perspective Plane Rotation Warp */
int tsfi_quantel_mirage_plane_rotation_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float pitch, float yaw, float roll);

/* Harry Chronological Frame Interlaced Field Blend Shift Jitter */
int tsfi_quantel_harry_blend_fields_shift_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int shift_even, int shift_odd, float jitter_amp);

/* Storyboard Frame Thumbnail Grid Corner Outlines Offset */
int tsfi_quantel_storyboard_corner_outlines_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int outline_len, uint32_t color);

/* Quantel Paintbox Brush Pressure Jitter Opacity Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Dynamic Twirl Zoom Phase Shift Warp */
int tsfi_quantel_mirage_twirl_zoom_phase(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius, float phase);

/* Harry Chronological Frame Interlaced Scan Line Offset Jitter */
int tsfi_quantel_harry_scanline_offset_jitter(uint32_t *pixels, int w, int h, int offset, float jitter_amp);

/* Storyboard Frame Thumbnail Grid Corner Spacers Offset */
int tsfi_quantel_storyboard_corner_spacers_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int spacer_w, uint32_t color);

/* Quantel Paintbox Brush Velocity Jitter Opacity Dynamics */
int tsfi_quantel_paintbox_velocity_jitter_opacity(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float speed, float jitter_amp, uint32_t color);

/* Mirage 3D Dynamic Twirl Zoom Phase Shift Symmetry Warp */
int tsfi_quantel_mirage_twirl_zoom_phase_symmetry(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float angle, float radius, float phase);

/* Harry Chronological Frame Interlaced Field Blend Offset Jitter */
int tsfi_quantel_harry_blend_fields_offset_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, float jitter_amp);

/* Storyboard Frame Thumbnail Grid Border Highlights Offset Width */
int tsfi_quantel_storyboard_border_highlights_offset_width(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t highlight_color);

/* Quantel Paintbox Brush Pressure Jitter Size Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_size(uint32_t *pixels, int w, int h, int cx, int cy, int base_radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Sphere Mapping Distortion Bulge Warp */
int tsfi_quantel_mirage_sphere_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float bulge_factor, float radius);

/* Harry Chronological Frame Interlaced Field Split Jitter Shift */
int tsfi_quantel_harry_blend_fields_jitter_shift(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int shift, float jitter_amp);

/* Storyboard Frame Thumbnail Grid Outer Highlights Offset */
int tsfi_quantel_storyboard_outer_highlights_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t color);

/* Quantel Paintbox Brush Pressure Jitter Hue Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_hue(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Cylinder Mapping Warp */
int tsfi_quantel_mirage_cylinder_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curvature, float radius);

/* Harry Chronological Frame Interlaced Field Blend Directional Jitter */
int tsfi_quantel_harry_blend_fields_directional_jitter(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, float jitter_x, float jitter_y);

/* Storyboard Frame Thumbnail Grid Double Borders Offset With Color */
int tsfi_quantel_storyboard_double_borders_offset_color(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, uint32_t color1, uint32_t color2);

/* Quantel Paintbox Brush Pressure Jitter Saturation Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Spherical Coordinate Perspective Warp */
int tsfi_quantel_mirage_spherical_coordinate_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float scale_r, float scale_theta);

/* Harry Chronological Frame Interlaced Scan Line Offset Jitter Width */
int tsfi_quantel_harry_scanline_offset_jitter_width(uint32_t *pixels, int w, int h, int offset, int line_width, float jitter_amp);

/* Storyboard Frame Thumbnail Grid Border Highlights Offset Width Color */
int tsfi_quantel_storyboard_border_highlights_offset_width_color(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t color);

/* Quantel Paintbox Brush Pressure Jitter Value Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_value(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Spherical Coordinate Perspective Zoom Warp */
int tsfi_quantel_mirage_spherical_coordinate_zoom_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float scale_r, float scale_theta, float zoom);

/* Harry Chronological Frame Interlaced Field Blend Directional Jitter Offset */
int tsfi_quantel_harry_blend_fields_directional_jitter_offset(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, float jitter_x, float jitter_y, int offset);

/* Storyboard Frame Thumbnail Grid Border Highlights Offset Width Color Outer */
int tsfi_quantel_storyboard_border_highlights_offset_width_color_outer(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t color);

/* Quantel Paintbox Brush Pressure Jitter Opacity Value Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_value(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Spherical Coordinate Perspective Zoom Twist Warp */
int tsfi_quantel_mirage_spherical_coordinate_zoom_twist_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float scale_r, float scale_theta, float zoom, float twist);

/* Harry Chronological Frame Interlaced Scan Line Offset Jitter Width Directional */
int tsfi_quantel_harry_scanline_offset_jitter_width_directional(uint32_t *pixels, int w, int h, int offset, int line_width, float jitter_amp, float angle);

/* Storyboard Frame Thumbnail Grid Border Highlights Offset Width Color Outer Double */
int tsfi_quantel_storyboard_border_highlights_offset_width_color_outer_double(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, uint32_t color1, uint32_t color2);

/* Orchestrator & Paintbox Synthesizer + Red/Blue Box Integration */
int tsfi_quantel_orchestrator_paintbox_synth_link(uint32_t *pixels, int w, int h, int cx, int cy, float pressure, float speed, float *audio_buf, int audio_count);

/* Quantel Paintbox Chalk Pressure Texture Modulation */
int tsfi_quantel_paintbox_chalk_pressure_texture(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float texture_scale, uint32_t color);

/* Mirage 3D Ribbon / Page Wave Warp */
int tsfi_quantel_mirage_ribbon_wave_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float frequency, float amplitude);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color */
int tsfi_quantel_harry_blend_fields_color_offset(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric */
int tsfi_quantel_storyboard_border_highlights_concentric(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Spherical Page Curl Warp */
int tsfi_quantel_mirage_page_curl_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical */
int tsfi_quantel_harry_blend_fields_color_offset_vertical(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double */
int tsfi_quantel_storyboard_border_highlights_concentric_double(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Spherical Page Curl Perspective Warp */
int tsfi_quantel_mirage_page_curl_perspective_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Width Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color);

/* Mirage 3D Spherical Page Curl Perspective Rotation Warp */
int tsfi_quantel_mirage_page_curl_perspective_rotation_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale Aspect */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer Width */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Width Angle Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle);

/* Mirage 3D Spherical Page Curl Perspective Rotation Shear Warp */
int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale Aspect Rotation */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer Width Offset */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Width Angle Shape Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle, float shape_aspect);

/* Mirage 3D Spherical Page Curl Perspective Rotation Shear Center Warp */
int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale Aspect Rotation Center */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle, float center_x, float center_y);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer Width Offset Color */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Width Angle Shape Texture Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle, float shape_aspect, float texture_scale);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale Aspect Rotation Center Mirror */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center_mirror(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle, float center_x, float center_y, int mirror_h, int mirror_v);

/* Mirage 3D Spherical Page Curl Perspective Rotation Shear Center Light Warp */
int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer Width Offset Color Texture */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color, float texture_intensity);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Width Angle Shape Texture Path Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture_path(uint32_t *pixels, int w, int h, int cx, int cy, int radius, float pressure, float jitter_amp, uint32_t color, float angle, float shape_aspect, float texture_scale, float path_vel_x, float path_vel_y);

/* Mirage 3D Spherical Page Curl Perspective Rotation Shear Center Light Depth Warp */
int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_depth_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y, float depth_offset);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale Aspect Rotation Center Mirror Matte */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center_mirror_matte(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle, float center_x, float center_y, int mirror_h, int mirror_v, const uint8_t *matte_mask);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer Width Offset Color Texture Bevel */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture_bevel(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color, float texture_intensity, int bevel_thickness);

/* Quantel Paintbox Brush Pressure Jitter Opacity Saturation Value Width Angle Shape Texture Path Spline Dynamics */
int tsfi_quantel_paintbox_pressure_jitter_opacity_saturation_value_width_angle_shape_texture_path_spline(uint32_t *pixels, int w, int h, float p0_x, float p0_y, float p1_x, float p1_y, float p2_x, float p2_y, float p3_x, float p3_y, int radius, float pressure, float jitter_amp, uint32_t color, float angle, float shape_aspect, float texture_scale);

/* Mirage 3D Spherical Page Curl Perspective Rotation Shear Center Light Depth Displacement Warp */
int tsfi_quantel_mirage_page_curl_perspective_rotation_shear_center_light_depth_displacement_warp(const uint32_t *src, int src_w, int src_h, uint32_t *dst, int dst_w, int dst_h, float curl_radius, float angle, float zoom, float perspective, float rotation_angle, float shear_x, float shear_y, float center_x, float center_y, float light_x, float light_y, float depth_offset, float disp_amplitude, float disp_frequency);

/* Harry Chronological Frame Interlaced Field Split Jitter Blend Offset Color Vertical Scale Aspect Rotation Center Mirror Matte Chroma Keyer */
int tsfi_quantel_harry_blend_fields_color_offset_vertical_scale_aspect_rotation_center_mirror_matte_chroma(const uint32_t *field_even, const uint32_t *field_odd, uint32_t *dst, int w, int h, float blend_factor, int offset, uint32_t tint_color, float vert_displace, float scale, float aspect, float rotation_angle, float center_x, float center_y, int mirror_h, int mirror_v, const uint8_t *matte_mask, uint32_t chroma_key_color, float chroma_tolerance);

/* Storyboard Frame Thumbnail Grid Border Highlights Concentric Double Outer Width Offset Color Texture Bevel DropShadow */
int tsfi_quantel_storyboard_border_highlights_concentric_double_outer_width_offset_color_texture_bevel_shadow(uint32_t *pixels, int w, int h, int cell_x, int cell_y, int cell_w, int cell_h, int offset_w, int border_w, int count, uint32_t color1, uint32_t color2, int outer_margin, int highlight_thickness, int highlight_offset_x, int highlight_offset_y, uint32_t shadow_color, float texture_intensity, int bevel_thickness, int shadow_blur_radius);

#endif // TSFI_PAINT_H















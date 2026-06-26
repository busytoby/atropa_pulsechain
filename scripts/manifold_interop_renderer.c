#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "libantigravity_interop.h"

#define WIDTH 1280
#define HEIGHT 720
#define CHANNELS 3
#define FRAME_SIZE (WIDTH * HEIGHT * CHANNELS)

int main(int argc, char *argv[]) {
    if (argc < 9) {
        fprintf(stderr, "Usage: %s <eris_base_raw> <fomalhaute_base_raw> <fornax_base_raw> <lineart_raw> <depth_raw> <normal_raw> <seg_raw> <interop_registry_bin> [style: stylized|lineart|retro]\n", argv[0]);
        return 1;
    }

    int style = 0; // 0 = photorealistic, 1 = stylized, 2 = lineart, 3 = retro
    if (argc > 9) {
        if (strcmp(argv[9], "stylized") == 0) style = 1;
        else if (strcmp(argv[9], "lineart") == 0) style = 2;
        else if (strcmp(argv[9], "retro") == 0) style = 3;
    }

    FILE *f_eris = fopen(argv[1], "rb");
    FILE *f_fomal = fopen(argv[2], "rb");
    FILE *f_fornax = fopen(argv[3], "rb");
    FILE *f_line = fopen(argv[4], "rb");
    FILE *f_depth = fopen(argv[5], "rb");
    FILE *f_norm = fopen(argv[6], "rb");
    FILE *f_seg = fopen(argv[7], "rb");
    FILE *f_reg = fopen(argv[8], "rb"); // Binary stream of InteropRegistry state

    if (!f_eris || !f_fomal || !f_fornax || !f_line || !f_depth || !f_norm || !f_seg || !f_reg) {
        fprintf(stderr, "Error opening raw frame input pipes or registry file.\n");
        return 1;
    }

    uint8_t *buf_eris = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_fomal = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_fornax = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_line = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_depth = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_norm = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_seg = (uint8_t *)malloc(FRAME_SIZE);
    uint8_t *buf_out = (uint8_t *)malloc(FRAME_SIZE);

    if (!buf_eris || !buf_fomal || !buf_fornax || !buf_line || !buf_depth || !buf_norm || !buf_seg || !buf_out) {
        fprintf(stderr, "Memory allocation failure.\n");
        return 1;
    }

    InteropRegistry reg;
    memset(&reg, 0, sizeof(InteropRegistry));
    reg.frame_modulation_factor = 1.0f;

    // Process frame-by-frame until EOF
    int frame = 0;
    while (1) {
        size_t r1_e = fread(buf_eris, 1, FRAME_SIZE, f_eris);
        size_t r1_fm = fread(buf_fomal, 1, FRAME_SIZE, f_fomal);
        size_t r1_fn = fread(buf_fornax, 1, FRAME_SIZE, f_fornax);
        size_t r2 = fread(buf_line, 1, FRAME_SIZE, f_line);
        size_t r3 = fread(buf_depth, 1, FRAME_SIZE, f_depth);
        size_t r4 = fread(buf_norm, 1, FRAME_SIZE, f_norm);
        size_t r5 = fread(buf_seg, 1, FRAME_SIZE, f_seg);

        if (r1_e < FRAME_SIZE || r1_fm < FRAME_SIZE || r1_fn < FRAME_SIZE ||
            r2 < FRAME_SIZE || r3 < FRAME_SIZE || r4 < FRAME_SIZE || r5 < FRAME_SIZE) {
            break; // EOF reached
        }

        // Read active registry state for this frame if available
        if (fread(&reg, sizeof(InteropRegistry), 1, f_reg) < 1) {
            // Keep previous registry state if stream ends early
        }

        // Extract registers to modulate compositing factors
        // Check if there are active messages in the interop LUN network queue
        double mq_boost = 0.0;
        if (reg.network_lun.head < reg.network_lun.tail) {
            mq_boost = 0.25; // Boost brightness/gain when network packets are active in queue
        }

        double spec_power = 8.0 + ((reg.frame_modulation_factor + mq_boost) * 16.0); // Modulated specularity
        double edge_blend = 0.5 + ((reg.frame_modulation_factor + mq_boost) * 0.5);  // LineArt visibility mapping

        for (int i = 0; i < FRAME_SIZE; i += 3) {
            int px = (i / 3) % WIDTH;
            int py = (i / 3) / WIDTH;

            uint8_t lr = buf_line[i], lg = buf_line[i+1], lb = buf_line[i+2];
            uint8_t dr = buf_depth[i], dg = buf_depth[i+1], db = buf_depth[i+2];
            uint8_t nr = buf_norm[i], ng = buf_norm[i+1], nb = buf_norm[i+2];
            uint8_t sr = buf_seg[i], sg = buf_seg[i+1], sb = buf_seg[i+2];

            // Sample raw color based on target pixel mask routing
            uint8_t br = 0, bg = 0, bb = 0;
            int is_eris = (sb > 200 && sr < 50 && sg < 50);
            int is_fomal = (sr > 200 && sg > 200 && sb < 50);
            int is_fornax = (sr > 200 && sg < 50 && sb < 50);

            if (is_eris) {
                br = buf_eris[i]; bg = buf_eris[i+1]; bb = buf_eris[i+2];
            } else if (is_fomal) {
                br = buf_fomal[i]; bg = buf_fomal[i+1]; bb = buf_fomal[i+2];
            } else if (is_fornax) {
                br = buf_fornax[i]; bg = buf_fornax[i+1]; bb = buf_fornax[i+2];
            } else {
                // Background fallback to Fomalhaute base
                br = buf_fomal[i]; bg = buf_fomal[i+1]; bb = buf_fomal[i+2];
            }

            double d_val = dr / 255.0;
            double l_val = (255.0 - lr) / 255.0;

            double nx = (nr - 127.5) / 127.5;
            double ny = (ng - 127.5) / 127.5;
            double nz = (nb - 127.5) / 127.5;
            double spec = pow(fmax(0.0, nz), spec_power);

            // Compute proximity glow to Verlet particles for Fornax connections
            double fornax_glow = 0.0;
            if (is_fornax) {
                if (reg.active_verlet_count > 0 && reg.active_verlet_count <= 16) {
                    for (uint32_t k = 0; k < reg.active_verlet_count; k++) {
                        float p_x = (reg.verlet_x[k] + 500.0f) / 1000.0f * 1280.0f;
                        float p_y = (reg.verlet_y[k] + 300.0f) / 600.0f * 720.0f;
                        float dx = (float)px - p_x;
                        float dy = (float)py - p_y;
                        float dist = sqrtf(dx * dx + dy * dy);
                        if (dist < 60.0f) {
                            fornax_glow += (1.0f - dist / 60.0f);
                        }
                    }
                    if (fornax_glow > 1.0) fornax_glow = 1.0;
                } else {
                    fornax_glow = 0.3; // Default baseline fallback
                }
            }

            // Material routing based on rendering style selection
            double r_out = 0, g_out = 0, b_out = 0;
            if (style == 2) { // Lineart only
                r_out = lr; g_out = lg; b_out = lb;
            } else if (style == 1) { // Stylized / Flat Casings
                r_out = sr; g_out = sg; b_out = sb;
                if (is_fomal) { // Yellow casing
                    r_out = 230 + spec * 25;
                    g_out = 190 + spec * 25;
                    b_out = 30;
                } else if (is_eris) { // Blue casing
                    r_out = 30;
                    g_out = 140 + spec * 50;
                    b_out = 240 + spec * 15;
                } else if (is_fornax) { // Red connections
                    r_out = 250 * fornax_glow;
                    g_out = 40 * fornax_glow;
                    b_out = 40 * fornax_glow;
                }
            } else { // Photorealistic or Retro base
                r_out = br * 1.3; g_out = bg * 1.3; b_out = bb * 1.3; // Boost base brightness
                if (is_fomal) { // Yellow Mask: Casing B (Fomalhaute)
                    r_out = br * 1.1 + spec * (140 + reg.frame_modulation_factor * 60);
                    g_out = bg * 1.1 + spec * (140 + reg.frame_modulation_factor * 60);
                    b_out = bb * 0.8;
                } else if (is_eris) { // Blue Mask: Casing A (Eris)
                    r_out = br * 1.0 + spec * (160 + reg.frame_modulation_factor * 80);
                    g_out = bg * 1.1 + spec * (120 + reg.frame_modulation_factor * 60);
                    b_out = bb * 1.3 + spec * (100 + reg.frame_modulation_factor * 50);
                } else if (is_fornax) { // Red Mask: Verlet Particles / Connections (Boosted Fornax)
                    r_out = br * (1.8 + reg.frame_modulation_factor * 1.2) + 120.0 * fornax_glow + 80.0;
                    g_out = bg * (1.8 + reg.frame_modulation_factor * 0.8) + 80.0 * fornax_glow + 40.0;
                    b_out = bb * 1.2 + 50.0 * fornax_glow;
                }
            }

            // Apply LineArt borders (edges mask details but lighter)
            r_out = r_out * (1.0 - l_val * (edge_blend * 0.4));
            g_out = g_out * (1.0 - l_val * (edge_blend * 0.4));
            b_out = b_out * (1.0 - l_val * (edge_blend * 0.4));

            // Apply Lighter Volumetric Fog and Ambient Occlusion from Depth maps
            double fog_r = 30, fog_g = 35, fog_b = 55; // Lighter atmospheric background
            // Focus when the beat drops: reduce fog and boost clarity of the manifold geometry
            double depth_blend = 0.3 + (d_val * 0.7) + (reg.frame_modulation_factor * 0.25);
            if (depth_blend > 1.0) depth_blend = 1.0;
            r_out = r_out * depth_blend + fog_r * (1.0 - depth_blend);
            g_out = g_out * depth_blend + fog_g * (1.0 - depth_blend);
            b_out = b_out * depth_blend + fog_b * (1.0 - depth_blend);

            if (style == 3) { // Retro Amiga 4-bit quantization (16-color channel bits)
                r_out = ((int)(r_out / 16.0)) * 16.0;
                g_out = ((int)(g_out / 16.0)) * 16.0;
                b_out = ((int)(b_out / 16.0)) * 16.0;
            }

            buf_out[i]   = (uint8_t)fmin(255.0, fmax(0.0, r_out));
            buf_out[i+1] = (uint8_t)fmin(255.0, fmax(0.0, g_out));
            buf_out[i+2] = (uint8_t)fmin(255.0, fmax(0.0, b_out));
        }

        fwrite(buf_out, 1, FRAME_SIZE, stdout);
        frame++;
    }

    fclose(f_eris);
    fclose(f_fomal);
    fclose(f_fornax);
    fclose(f_line);
    fclose(f_depth);
    fclose(f_norm);
    fclose(f_seg);
    fclose(f_reg);

    free(buf_eris);
    free(buf_fomal);
    free(buf_fornax);
    free(buf_line);
    free(buf_depth);
    free(buf_norm);
    free(buf_seg);
    free(buf_out);

    return 0;
}

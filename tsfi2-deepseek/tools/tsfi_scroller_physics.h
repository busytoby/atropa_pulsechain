#ifndef TSFI_SCROLLER_PHYSICS_H
#define TSFI_SCROLLER_PHYSICS_H

#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_scroller_graphics.h"

typedef struct {
    float x, y, z;
    float px, py, pz;
} ScrollerPoppyJoint;

static ScrollerPoppyJoint poppy_joints[7];
static bool poppy_initialized = false;

#define MAX_POLLEN 16
typedef struct {
    float x, y;
    float vx, vy;
    float life;
} PollenParticle;

static PollenParticle pollen[MAX_POLLEN];
static bool pollen_initialized = false;

static void draw_usd_spline_ribbon(float time_val, float pulse, double usd_value, int color_scheme) {
    (void)color_scheme;

    // 1. Initialize Verlet joints: 1 central pod (0) + 6 radiating petals (1..6)
    if (!poppy_initialized) {
        poppy_joints[0] = (ScrollerPoppyJoint){ .x = 640.0f, .y = 300.0f, .z = 50.0f, .px = 640.0f, .py = 300.0f, .pz = 50.0f }; // central pod
        for (int i = 1; i <= 6; i++) {
            float ang = (i - 1) * (2.0f * M_PI / 6.0f);
            float px = 640.0f + 22.0f * cosf(ang);
            float py = 300.0f + 22.0f * sinf(ang);
            poppy_joints[i] = (ScrollerPoppyJoint){ .x = px, .y = py, .z = 50.0f, .px = px, .py = py, .pz = 50.0f };
        }
        poppy_initialized = true;
    }

    // 2. Sway the Central Pod gently and slowly in the wind (low frequency 0.6f)
    float stem_base_x = 640.0f;
    float stem_base_y = (float)HEIGHT - 80.0f;
    float wind_boost = (float)log10(usd_value + 1.0) * 0.8f;
    float wind_x = (5.0f + wind_boost) * sinf(time_val * 0.6f);

    poppy_joints[0].x = stem_base_x + wind_x * 2.5f;
    poppy_joints[0].y = stem_base_y - 250.0f; // Lift higher up on the screen

    // Petal rest length (bloom expansion) based on USD telemetry, beat pulse, and breathing cycle
    float breathe = 3.5f * sinf(time_val * 1.2f);
    float petal_rest_len = 24.0f + (float)fminf(18.0f, (float)log10(usd_value + 1.0) * 3.0f) + pulse * 4.0f + breathe;
    // Set high damping decay (0.55f) to prevent runaway gravity drift
    float decay = 0.55f;

    // 3. Update Petals using Verlet and pull tightly to their target radial angles around the Pod
    for (int i = 1; i <= 6; i++) {
        float tx = poppy_joints[i].x;
        float ty = poppy_joints[i].y;

        // Slow, gentle fluttering force
        float petal_wind_x = 1.5f * sinf(time_val * 1.5f + i) + wind_x * 0.05f;
        float petal_gravity_y = 0.15f;

        poppy_joints[i].x += (poppy_joints[i].x - poppy_joints[i].px) * decay + petal_wind_x * 0.05f;
        poppy_joints[i].y += (poppy_joints[i].y - poppy_joints[i].py) * decay + petal_gravity_y * 0.05f;

        poppy_joints[i].px = tx;
        poppy_joints[i].py = ty;

        // Angular targets: evenly spaced at 60 degree intervals, tilted based on stem deflection velocity
        float target_angle = (i - 1) * (2.0f * M_PI / 6.0f) + wind_x * 0.035f;
        float target_x = poppy_joints[0].x + petal_rest_len * cosf(target_angle);
        float target_y = poppy_joints[0].y + petal_rest_len * sinf(target_angle);

        // Strong stiffness (0.85f) to prevent petals from drifting away from the pod
        poppy_joints[i].x += (target_x - poppy_joints[i].x) * 0.85f;
        poppy_joints[i].y += (target_y - poppy_joints[i].y) * 0.85f;
    }

    // Curved Bezier Stem calculations (forest green with darker shading)
    float ctrl_x = (stem_base_x + poppy_joints[0].x) * 0.5f - wind_x * 2.0f;
    float ctrl_y = (stem_base_y + poppy_joints[0].y) * 0.5f;

    // Calculate mid-stem leaf node connection coordinates at t = 0.35 along the Bezier curve
    float t_node = 0.35f;
    float omt_node = 1.0f - t_node;
    int node_x = (int)(omt_node * omt_node * stem_base_x + 2.0f * omt_node * t_node * ctrl_x + t_node * t_node * poppy_joints[0].x);
    int node_y = (int)(omt_node * omt_node * stem_base_y + 2.0f * omt_node * t_node * ctrl_y + t_node * t_node * poppy_joints[0].y);

    // 4. Render the Verlet Poppy Flower components
    // Swaying feathery leaves influenced physically by wind deflection
    float leaf_sway = 0.05f * sinf(time_val * 1.5f) + wind_x * 0.015f;
    
    // Base tier leaves
    draw_gothic_leaf((int)stem_base_x, (int)stem_base_y, 80.0f, -M_PI * 0.8f + leaf_sway, time_val);
    draw_gothic_leaf((int)stem_base_x, (int)stem_base_y, 80.0f, -M_PI * 0.2f - leaf_sway, time_val);

    // Mid-stem tier leaves (slightly smaller, growing from node joint)
    draw_gothic_leaf(node_x, node_y, 55.0f, -M_PI * 0.85f + leaf_sway, time_val);
    draw_gothic_leaf(node_x, node_y, 55.0f, -M_PI * 0.15f - leaf_sway, time_val);

    int segments = 24;
    int prev_x = (int)stem_base_x;
    int prev_y = (int)stem_base_y;

    for (int j = 1; j <= segments; j++) {
        float t = (float)j / (float)segments;
        float omt = 1.0f - t;
        int sx = (int)(omt * omt * stem_base_x + 2.0f * omt * t * ctrl_x + t * t * poppy_joints[0].x);
        int sy = (int)(omt * omt * stem_base_y + 2.0f * omt * t * ctrl_y + t * t * poppy_joints[0].y);

        // Organic color shading: brown near the base to fresh green at the top
        uint8_t r_col = (uint8_t)(105 * (1.0f - t) + 55 * t);
        uint8_t g_col = (uint8_t)(85 * (1.0f - t) + 195 * t);
        uint8_t b_col = (uint8_t)(45 * (1.0f - t) + 65 * t);

        // 3D cylindrical chiascuro shading (left highlight, center body, right shadow)
        draw_line(prev_x - 1, prev_y, sx - 1, sy, (uint8_t)fminf(255, r_col * 1.3f), (uint8_t)fminf(255, g_col * 1.3f), (uint8_t)fminf(255, b_col * 1.3f)); // Left highlight
        draw_line(prev_x, prev_y, sx, sy, r_col, g_col, b_col); // Center body
        draw_line(prev_x + 1, prev_y, sx + 1, sy, (uint8_t)(r_col * 0.5f), (uint8_t)(g_col * 0.5f), (uint8_t)(b_col * 0.5f)); // Right shadow

        // Tiny dynamic curved downy hairs projecting from both sides of the stem (pilosity)
        if (j % 2 == 0) {
            float perp_x = -(float)(sy - prev_y);
            float perp_y = (float)(sx - prev_x);
            float len = sqrtf(perp_x*perp_x + perp_y*perp_y);
            if (len > 0.0f) {
                perp_x /= len;
                perp_y /= len;
                // Alternate left and right side of the Bezier spline
                float side = (j % 4 == 0) ? 1.0f : -1.0f;
                int hx = sx + (int)(side * perp_x * 5.0f + 1.2f * sinf(time_val * 3.0f + j));
                int hy = sy + (int)(side * perp_y * 5.0f);
                draw_line(sx, sy, hx, hy, 120, 195, 125); // Brighter downy green hair
            }
        }

        prev_x = sx;
        prev_y = sy;
    }

    // Calyx / Receptacle (forest green bud cup at the bottom base of the flower head)
    draw_glossy_bubble((int)poppy_joints[0].x, (int)poppy_joints[0].y + 12, 10, 46, 125, 50);


    // Multilayered Petals: Outer layer (1, 3, 5) is larger & darker; Inner layer (2, 4, 6) is smaller & brighter
    for (int i = 1; i <= 6; i += 2) {
        draw_flat_petal((int)poppy_joints[i].x, (int)poppy_joints[i].y, 38.0f, 195, 15, 50, (int)poppy_joints[0].x, (int)poppy_joints[0].y, time_val); // Outer
    }
    for (int i = 2; i <= 6; i += 2) {
        draw_flat_petal((int)poppy_joints[i].x, (int)poppy_joints[i].y, 32.0f, 235, 30, 70, (int)poppy_joints[0].x, (int)poppy_joints[0].y, time_val); // Inner
    }


    // Draw stamen filaments radiating from the center pod (black stems with gold tips)
    float stamen_count = 16.0f;
    for (int s = 0; s < (int)stamen_count; s++) {
        float angle = (s / stamen_count) * 2.0f * M_PI;
        float stamen_len = 16.0f + 2.0f * sinf(time_val * 6.0f + s);
        int sx = (int)(poppy_joints[0].x + stamen_len * cosf(angle));
        int sy = (int)(poppy_joints[0].y + stamen_len * sinf(angle));
        draw_line((int)poppy_joints[0].x, (int)poppy_joints[0].y, sx, sy, 30, 30, 30);
        draw_glossy_bubble(sx, sy, 2, 255, 215, 0); // Gold tips
    }

    // Central Pod (Deep Black/Gold core, drawn on top)
    draw_glossy_bubble((int)poppy_joints[0].x, (int)poppy_joints[0].y, 14, 20, 20, 20);
    draw_glossy_bubble((int)poppy_joints[0].x - 2, (int)poppy_joints[0].y - 2, 4, 255, 215, 0);

    // Star-shaped Poppy seed crown (6 dark spikes on top of central pod)
    for (int c = 0; c < 6; c++) {
        float c_ang = c * (2.0f * M_PI / 6.0f) + time_val * 0.1f;
        int csx = (int)(poppy_joints[0].x + 7.0f * cosf(c_ang));
        int csy = (int)(poppy_joints[0].y + 7.0f * sinf(c_ang));
        draw_line((int)poppy_joints[0].x, (int)poppy_joints[0].y, csx, csy, 5, 5, 5);
    }

    // 5. Update and render 16 dynamic gold pollen particles drifting in the wind
    if (!pollen_initialized) {
        for (int p = 0; p < MAX_POLLEN; p++) {
            pollen[p].life = 0.0f;
        }
        pollen_initialized = true;
    }

    for (int p = 0; p < MAX_POLLEN; p++) {
        pollen[p].life -= 0.012f;
        if (pollen[p].life <= 0.0f) {
            pollen[p].x = poppy_joints[0].x + (float)(rand() % 14 - 7);
            pollen[p].y = poppy_joints[0].y + (float)(rand() % 14 - 7);
            pollen[p].vx = (wind_x * 0.15f) + (float)(rand() % 100 - 50) / 120.0f;
            pollen[p].vy = 0.4f + (float)(rand() % 100) / 250.0f;
            pollen[p].life = 0.4f + (float)(rand() % 100) / 160.0f; // randomized lifetime
        }

        // Adding miniature swirling eddies turbulence
        float turbulence = 1.4f * cosf(time_val * 5.0f + p);
        pollen[p].x += pollen[p].vx + turbulence * 0.05f;
        pollen[p].y += pollen[p].vy;

        // Draw glowing gold particle tip
        if (pollen[p].x >= 0 && pollen[p].x < WIDTH && pollen[p].y >= 0 && pollen[p].y < HEIGHT) {
            uint8_t alpha = (uint8_t)(255.0f * pollen[p].life);
            // Blend gold color with current background pixels
            int p_idx = ((int)pollen[p].y * WIDTH + (int)pollen[p].x) * 3;
            frame_buffer[p_idx] = (uint8_t)fminf(255, frame_buffer[p_idx] + (255 * alpha / 255));
            frame_buffer[p_idx + 1] = (uint8_t)fminf(255, frame_buffer[p_idx + 1] + (215 * alpha / 255));
            frame_buffer[p_idx + 2] = (uint8_t)fminf(255, frame_buffer[p_idx + 2] + (0 * alpha / 255));
        }
    }

    // Share physical wind/sway deflection state with audio synthesizer process
    FILE *f_phys = fopen("/tmp/poppy_physics.bin", "wb");
    if (f_phys) {
        fwrite(&wind_x, sizeof(float), 1, f_phys);
        fclose(f_phys);
    }
}

#endif // TSFI_SCROLLER_PHYSICS_H

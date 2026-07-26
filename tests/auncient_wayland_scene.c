// Wayland Markdown Editor 3D scene and render delegate routines
#include <stdint.h>
#include <math.h>

static inline float sd_capsule(float px, float py, float pz, float ax, float ay, float az, float bx, float by, float bz, float r) {
    float pax = px - ax, pay = py - ay, paz = pz - az;
    float bax = bx - ax, bay = by - ay, baz = bz - az;
    float h = (pax*bax + pay*bay + paz*baz) / (bax*bax + bay*bay + baz*baz);
    if (h < 0.0f) h = 0.0f;
    if (h > 1.0f) h = 1.0f;
    float dx = pax - bax * h;
    float dy = pay - bay * h;
    float dz = paz - baz * h;
    return sqrtf(dx*dx + dy*dy + dz*dz) - r;
}

static inline float sd_cactus(float px, float py, float pz) {
    float d1 = sd_capsule(px, py, pz, 0.0f, -3.5f, 0.0f, 0.0f, 3.5f, 0.0f, 0.8f);
    float d2 = sd_capsule(px, py, pz, 0.0f, 0.5f, 0.0f, -1.8f, 0.5f, 0.0f, 0.6f);
    float d3 = sd_capsule(px, py, pz, -1.8f, 0.5f, 0.0f, -1.8f, 2.5f, 0.0f, 0.6f);
    float d4 = sd_capsule(px, py, pz, 0.0f, -0.5f, 0.0f, 1.8f, -0.5f, 0.0f, 0.6f);
    float d5 = sd_capsule(px, py, pz, 1.8f, -0.5f, 0.0f, 1.8f, 1.5f, 0.0f, 0.6f);
    float d = d1;
    if (d2 < d) d = d2;
    if (d3 < d) d = d3;
    if (d4 < d) d = d4;
    if (d5 < d) d = d5;
    return d;
}

static inline float sd_letter_t(float px, float py, float pz) {
    float d1 = sd_capsule(px, py, pz, -1.5f, 2.0f, 0.0f, 1.5f, 2.0f, 0.0f, 0.5f);
    float d2 = sd_capsule(px, py, pz, 0.0f, -2.0f, 0.0f, 0.0f, 2.0f, 0.0f, 0.5f);
    return (d1 < d2) ? d1 : d2;
}

static inline uint32_t blend_color_add(uint32_t base, uint32_t add) {
    uint8_t r1 = (base >> 16) & 0xFF;
    uint8_t g1 = (base >> 8) & 0xFF;
    uint8_t b1 = base & 0xFF;
    uint8_t r2 = (add >> 16) & 0xFF;
    uint8_t g2 = (add >> 8) & 0xFF;
    uint8_t b2 = add & 0xFF;
    int r = r1 + r2; if (r > 255) r = 255;
    int g = g1 + g2; if (g > 255) g = 255;
    int b = b1 + b2; if (b > 255) b = 255;
    return (0xFF000000 | (r << 16) | (g << 8) | b);
}

static void hd_storm_render(uint32_t *pixels, int w, int h, const CoaxialUBO *ubo) {
    (void)ubo;
    for (int i = 0; i < 15; i++) {
        if (starfield[i].z > 0.1f) {
            int px = w / 2 + (int)((starfield[i].x / starfield[i].z) * (w / 2)) + glitch_x;
            int py = h / 2 + (int)((starfield[i].y / starfield[i].z) * (h / 2)) + glitch_y;
            if (px >= 0 && px < w && py >= 0 && py < h) {
                uint32_t star_color = 0xFF884400;
                if (starfield[i].z < 3.0f) star_color = 0xFFFFCC00;
                else if (starfield[i].z < 6.0f) star_color = 0xFFAA6600;
                draw_char(pixels, w, h, px, py, starfield[i].glyph, star_color, 2);
            }
        }
    }
}

static void hd_embree_render(uint32_t *pixels, int w, int h, const CoaxialUBO *ubo) {
    float angle = ubo->rotation_angle;
    float cos_a = cosf(angle);
    float sin_a = sinf(angle);
    float ry = ubo->camera_y;
    int max_steps = 16;
    float abs_ry = fabsf(ry);
    if (abs_ry > 1.8f) max_steps = 6;
    else if (abs_ry > 1.0f) max_steps = 10;
    
    int resolved_variant = (int)ubo->material_variant;
    if (resolved_variant < 0 || resolved_variant > 2) {
        int fallbacks[3] = {1, 0, 2};
        for (int f = 0; f < 3; f++) {
            if (fallbacks[f] >= 0 && fallbacks[f] <= 2) {
                resolved_variant = fallbacks[f];
                break;
            }
        }
    }
    
    int vp_x = 700, vp_y = 120, vp_w = 160, vp_h = 160;
    for (int x = 0; x < vp_w; x++) {
        if (vp_y >= 0 && vp_y < h && (vp_x + x) < w) pixels[vp_y * w + vp_x + x] = 0xFF555555;
        if ((vp_y + vp_h) >= 0 && (vp_y + vp_h) < h && (vp_x + x) < w) pixels[(vp_y + vp_h) * w + vp_x + x] = 0xFF555555;
    }
    for (int y = 0; y <= vp_h; y++) {
        if ((vp_y + y) >= 0 && (vp_y + y) < h && vp_x < w) pixels[(vp_y + y) * w + vp_x] = 0xFF555555;
        if ((vp_y + y) >= 0 && (vp_y + y) < h && (vp_x + vp_w) < w) pixels[(vp_y + y) * w + vp_x + vp_w] = 0xFF555555;
    }
    draw_string(pixels, w, h, vp_x + 5, vp_y - 12, "HYDRA: HD_EMBREE", 0xFF888888, 1);

    for (int vy = 0; vy < 80; vy++) {
        for (int vx = 0; vx < 80; vx++) {
            float rx = (vx - 40.0f) * 0.11f;
            float ry_val = (vy - 40.0f) * 0.11f + ry;
            float rz = -5.0f;
            int steps = 0;
            
            while (steps < max_steps) {
                steps++;
                float rot_x = rx * cos_a - rz * sin_a;
                float rot_z = rx * sin_a + rz * cos_a;
                float dist;
                int model_to_render = ubo->active_model;
                if (resolved_variant == 2) {
                    model_to_render = 1;
                }
                if (model_to_render == 0) {
                    dist = sd_cactus(rot_x, ry_val, rot_z);
                } else if (model_to_render == 1) {
                    dist = sd_letter_t(rot_x, ry_val, rot_z);
                } else {
                    float d_c = sd_cactus(rot_x, ry_val, rot_z);
                    float d_l = sd_letter_t(rot_x, ry_val, rot_z);
                    dist = (d_c < d_l) ? d_c : d_l;
                }
                if (dist < 0.05f) break;
                rz += dist;
                if (rz > 15.0f) { steps = max_steps; break; }
            }
            
            if (steps < max_steps) {
                uint8_t r_col, g_col, b_col;
                float shade = 1.0f - ((float)steps / (float)max_steps);
                if (resolved_variant == 1) {
                    r_col = (uint8_t)(255 * shade);
                    g_col = (uint8_t)(85 * shade);
                    b_col = (uint8_t)(85 * shade);
                } else if (resolved_variant == 2) {
                    bool cross_hatch = ((vx / 2) % 2 == 0) ^ ((vy / 2) % 2 == 0);
                    float pattern_scale = cross_hatch ? 0.85f : 1.1f;
                    float cloth_shade = shade * pattern_scale;
                    if (cloth_shade > 1.0f) cloth_shade = 1.0f;
                    
                    r_col = (uint8_t)(85 * cloth_shade);
                    g_col = (uint8_t)(170 * cloth_shade);
                    b_col = (uint8_t)(85 * cloth_shade);
                } else {
                    r_col = (uint8_t)(255 * shade);
                    g_col = (uint8_t)(204 * shade);
                    b_col = 0;
                }
                uint32_t color = 0xFF000000 | (r_col << 16) | (g_col << 8) | b_col;
                for (int dy = 0; dy < 2; dy++) {
                    for (int dx = 0; dx < 2; dx++) {
                        int px = vp_x + vx * 2 + dx;
                        int py = vp_y + vy * 2 + dy;
                        if (px >= 0 && px < w && py >= 0 && py < h) {
                            pixels[py * w + px] = color;
                        }
                    }
                }
            }
        }
    }
}

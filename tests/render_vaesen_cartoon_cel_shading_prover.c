#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#define WIDTH 640
#define HEIGHT 480

typedef struct {
    uint8_t r, g, b;
} PixelRGB;

static inline double clamp(double v, double min_v, double max_v) {
    if (v < min_v) return min_v;
    if (v > max_v) return max_v;
    return v;
}

// 3D Vector Math
typedef struct { double x, y, z; } Vec3;
static inline Vec3 v_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static inline double v_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static inline Vec3 v_norm(Vec3 a) {
    double l = sqrt(v_dot(a, a));
    return (l > 1e-8) ? (Vec3){a.x / l, a.y / l, a.z / l} : (Vec3){0, 0, 0};
}

// Signed Distance Field (SDF) of Vaesen Teddy Bear Head with Ears and Snout
static double sdf_scene(Vec3 p) {
    // Head Sphere at (0, 0, 0), radius 1.0
    Vec3 head_c = {0.0, 0.0, 0.0};
    double d_head = sqrt(v_dot(v_sub(p, head_c), v_sub(p, head_c))) - 1.0;

    // Left Ear at (-0.75, 0.85, 0.0), radius 0.42
    Vec3 lear_c = {-0.75, 0.85, 0.0};
    double d_lear = sqrt(v_dot(v_sub(p, lear_c), v_sub(p, lear_c))) - 0.42;

    // Right Ear at (0.75, 0.85, 0.0), radius 0.42
    Vec3 rear_c = {0.75, 0.85, 0.0};
    double d_rear = sqrt(v_dot(v_sub(p, rear_c), v_sub(p, rear_c))) - 0.42;

    // Snout at (0.0, -0.2, 0.75), radius 0.45
    Vec3 snout_c = {0.0, -0.2, 0.75};
    double d_snout = sqrt(v_dot(v_sub(p, snout_c), v_sub(p, snout_c))) - 0.45;

    // Smooth Union
    double d = d_head;
    if (d_lear < d) d = d_lear;
    if (d_rear < d) d = d_rear;
    if (d_snout < d) d = d_snout;
    return d;
}

static Vec3 calc_normal(Vec3 p) {
    double e = 0.001;
    double dx = sdf_scene((Vec3){p.x + e, p.y, p.z}) - sdf_scene((Vec3){p.x - e, p.y, p.z});
    double dy = sdf_scene((Vec3){p.x, p.y + e, p.z}) - sdf_scene((Vec3){p.x, p.y - e, p.z});
    double dz = sdf_scene((Vec3){p.x, p.y, p.z + e}) - sdf_scene((Vec3){p.x, p.y, p.z - e});
    return v_norm((Vec3){dx, dy, dz});
}

int main(int argc, char **argv) {
    const char *out_ppm = (argc > 1) ? argv[1] : "vaesen_cartoon_teddy_bear_frame.ppm";

    printf("=================================================================\n");
    printf("CP/M-TOMIE PROCEDURAL CARTOON CEL SHADING RENDERER\n");
    printf("=================================================================\n");

    PixelRGB *framebuffer = (PixelRGB *)malloc(WIDTH * HEIGHT * sizeof(PixelRGB));
    if (!framebuffer) {
        fprintf(stderr, "Error: Failed to allocate framebuffer memory\n");
        return 1;
    }

    Vec3 light_dir = v_norm((Vec3){0.577, 0.577, -0.577});
    Vec3 cam_pos = {0.0, 0.0, -3.2};

    // Cartoon Cel-Shading Palette (Warm Golden TeddyBear Honey Tone)
    PixelRGB base_color = {245, 185, 95};
    PixelRGB shadow_color = {195, 120, 45};
    PixelRGB ink_outline = {25, 15, 10};
    PixelRGB bg_color = {225, 238, 248}; // Light pastel cyan sky

    for (int y = 0; y < HEIGHT; ++y) {
        for (int x = 0; x < WIDTH; ++x) {
            double u = (x - WIDTH * 0.5) / (HEIGHT * 0.5);
            double v = -(y - HEIGHT * 0.5) / (HEIGHT * 0.5);

            Vec3 ray_dir = v_norm((Vec3){u, v, 1.8});
            Vec3 p = cam_pos;
            double t = 0.0;
            bool hit = false;

            for (int step = 0; step < 64; ++step) {
                double dist = sdf_scene(p);
                if (dist < 0.001) {
                    hit = true;
                    break;
                }
                t += dist;
                if (t > 10.0) break;
                p = (Vec3){cam_pos.x + ray_dir.x * t, cam_pos.y + ray_dir.y * t, cam_pos.z + ray_dir.z * t};
            }

            int idx = y * WIDTH + x;
            if (hit) {
                Vec3 norm = calc_normal(p);
                Vec3 view_dir = v_norm((Vec3){-ray_dir.x, -ray_dir.y, -ray_dir.z});
                double ndotl = v_dot(norm, light_dir);
                double ndotv = v_dot(norm, view_dir);

                // Silhouette Ink Outline Thresholding (Sobel / Rim Silhouette Gating)
                if (ndotv < 0.28) {
                    framebuffer[idx] = ink_outline;
                } else {
                    // Discrete 3-Tier Cel-Shading Quantization
                    if (ndotl > 0.45) {
                        framebuffer[idx] = base_color;
                    } else if (ndotl > -0.1) {
                        framebuffer[idx] = shadow_color;
                    } else {
                        framebuffer[idx] = (PixelRGB){shadow_color.r * 0.75, shadow_color.g * 0.75, shadow_color.b * 0.75};
                    }
                }
            } else {
                framebuffer[idx] = bg_color;
            }
        }
    }

    FILE *f = fopen(out_ppm, "wb");
    if (!f) {
        perror("fopen");
        free(framebuffer);
        return 1;
    }
    fprintf(f, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
    fwrite(framebuffer, sizeof(PixelRGB), WIDTH * HEIGHT, f);
    fclose(f);
    free(framebuffer);

    printf(" Rendered Cartoon Frame:    %s (%dx%d RGB)\n", out_ppm, WIDTH, HEIGHT);
    printf(" Cel Shading Quantization:  3-Tier Discrete Radiance Bands\n");
    printf(" Silhouette Ink Outlining:  Rim Normal Thresholding (N.V < 0.28)\n");
    printf("=================================================================\n");
    return 0;
}

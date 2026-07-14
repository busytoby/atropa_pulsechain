#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include "tsfi2_animation.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define NUM_PARTICLES 60
#define WIDTH 640
#define HEIGHT 360
#define TOTAL_FRAMES 300 // 10 seconds at 30 fps

typedef struct {
    tsfi2_vec3_t curr;
    tsfi2_vec3_t prev;
} particle_t;

// Standard random range helper
float rand_range(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

int main(void) {
    printf("Compiling high-fidelity simulation from Reynolds (1987) & Lasseter (1987)...\n");
    srand(42);

    // 1. Initialize Particles (Reynolds Boids)
    particle_t particles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].curr.x = rand_range(-1.0f, 1.0f);
        particles[i].curr.y = rand_range(-0.4f, 0.4f);
        particles[i].curr.z = rand_range(-0.4f, 0.4f);
        
        // Give initial velocity along X axis
        particles[i].prev.x = particles[i].curr.x - 0.015f;
        particles[i].prev.y = particles[i].curr.y;
        particles[i].prev.z = particles[i].curr.z;
    }

    // Fixed Impurity Obstacles in FET Channel
    tsfi2_vec3_t impurities[3] = {
        {0.0f, 0.0f, 0.0f},
        {-0.5f, 0.18f, -0.1f},
        {0.5f, -0.18f, 0.1f}
    };

    // 2. Open stdout pipe to FFMPEG to compile video directly (No disk writes)
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -f image2pipe -vcodec ppm -framerate 30 -i - "
             "-c:v libx264 -pix_fmt yuv420p assets/papers_demo.mp4");
             
    FILE *ffmpeg = popen(cmd, "w");
    if (!ffmpeg) {
        fprintf(stderr, "Failed to start FFMPEG process.\n");
        return 1;
    }

    tsfi2_canvas_t *canvas = tsfi2_canvas_create(WIDTH, HEIGHT);
    
    // Springy visualizer bars state (Secondary Action & Follow Through)
    float bar_heights[16] = {0};

    printf("Piping frames directly to FFMPEG...\n");
    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        float t = frame / 30.0f;
        
        // Easing camera angle for smooth slow-in/out motion (Lasseter Principle 6)
        float camera_angle = tsfi2_ease_slow_in_out(t / (TOTAL_FRAMES / 30.0f)) * (float)M_PI * 2.0f;
        
        // Clear canvas
        tsfi2_canvas_clear(canvas, 15, 17, 26);

        // Header titles
        tsfi2_draw_line_aa(canvas, 20, 70, WIDTH - 20, 70, 40, 45, 65, 1.0f);
        
        // 3. Step Verlet Simulation (Straight Ahead Action - Lasseter Principle 4)
        float dt = 0.1f;
        for (int i = 0; i < NUM_PARTICLES; i++) {
            // Migratory urge (target voltage field pulling along X axis)
            float fx = 0.15f;
            float fy = 0.0f;
            float fz = 0.0f;

            // Obstacle Avoidance (Reynolds' Obstacles)
            for (int o = 0; o < 3; o++) {
                float dx_o = particles[i].curr.x - impurities[o].x;
                float dy_o = particles[i].curr.y - impurities[o].y;
                float dz_o = particles[i].curr.z - impurities[o].z;
                float dist_o = sqrtf(dx_o*dx_o + dy_o*dy_o + dz_o*dz_o);
                
                if (dist_o < 0.22f && dist_o > 0.001f) {
                    float avoid_mag = 0.08f / dist_o;
                    fx += (dx_o / dist_o) * avoid_mag;
                    fy += (dy_o / dist_o) * avoid_mag;
                    fz += (dz_o / dist_o) * avoid_mag;
                }
            }

            // Reynolds Flocking Steering Rules (Separation, Cohesion, Alignment)
            float ci_x = 0.0f, ci_y = 0.0f, ci_z = 0.0f;
            float vi_x = 0.0f, vi_y = 0.0f, vi_z = 0.0f;
            int neighbors = 0;

            for (int j = 0; j < NUM_PARTICLES; j++) {
                if (i == j) continue;
                float dx_p = particles[i].curr.x - particles[j].curr.x;
                float dy_p = particles[i].curr.y - particles[j].curr.y;
                float dz_p = particles[i].curr.z - particles[j].curr.z;
                float dist = sqrtf(dx_p*dx_p + dy_p*dy_p + dz_p*dz_p);

                if (dist < 0.35f && dist > 0.001f) {
                    // Separation
                    float repel = 0.02f / dist;
                    fx += (dx_p / dist) * repel;
                    fy += (dy_p / dist) * repel;
                    fz += (dz_p / dist) * repel;

                    // Cohesion accumulation
                    ci_x += particles[j].curr.x;
                    ci_y += particles[j].curr.y;
                    ci_z += particles[j].curr.z;

                    // Alignment accumulation (implicit velocity)
                    vi_x += particles[j].curr.x - particles[j].prev.x;
                    vi_y += particles[j].curr.y - particles[j].prev.y;
                    vi_z += particles[j].curr.z - particles[j].prev.z;

                    neighbors++;
                }
            }

            if (neighbors > 0) {
                ci_x /= neighbors; ci_y /= neighbors; ci_z /= neighbors;
                fx += (ci_x - particles[i].curr.x) * 0.08f;
                fy += (ci_y - particles[i].curr.y) * 0.08f;
                fz += (ci_z - particles[i].curr.z) * 0.08f;

                vi_x /= neighbors; vi_y /= neighbors; vi_z /= neighbors;
                float implicit_vx = particles[i].curr.x - particles[i].prev.x;
                float implicit_vy = particles[i].curr.y - particles[i].prev.y;
                float implicit_vz = particles[i].curr.z - particles[i].prev.z;
                fx += (vi_x - implicit_vx) * 0.1f;
                fy += (vi_y - implicit_vy) * 0.1f;
                fz += (vi_z - implicit_vz) * 0.1f;
            }

            // Verlet Integration Step
            float vx = particles[i].curr.x - particles[i].prev.x;
            float vy = particles[i].curr.y - particles[i].prev.y;
            float vz = particles[i].curr.z - particles[i].prev.z;

            float next_x = particles[i].curr.x + vx + fx * dt * dt;
            float next_y = particles[i].curr.y + vy + fy * dt * dt;
            float next_z = particles[i].curr.z + vz + fz * dt * dt;

            // Boundary Wrap-around
            if (next_x > 1.2f) {
                next_x = -1.2f;
                particles[i].prev.x = -1.21f;
            }

            particles[i].prev = particles[i].curr;
            particles[i].curr.x = next_x;
            particles[i].curr.y = next_y;
            particles[i].curr.z = next_z;
        }

        // 4. Project and Draw 3D FET Tube Rings (arcs of cylinder)
        for (int ring = -4; ring <= 4; ring++) {
            float rx = ring * 0.3f;
            for (int deg = 0; deg < 360; deg += 15) {
                float rad = (float)(deg * M_PI / 180.0);
                float cy = 0.45f * sinf(rad);
                float cz = 0.45f * cosf(rad);

                tsfi2_vec3_t ring_pt = {rx, cy, cz};
                int sx, sy;
                tsfi2_project_3d_to_2d(&ring_pt, camera_angle, 180.0f, WIDTH, HEIGHT, &sx, &sy);
                if (sx >= 0 && sx < WIDTH && sy >= 0 && sy < HEIGHT) {
                    tsfi2_draw_pixel(canvas, sx, sy, 40, 45, 65, 0.8f);
                }
            }
        }

        // 5. Draw Obstacles (Squash & Stretch on size)
        for (int o = 0; o < 3; o++) {
            int sx, sy;
            tsfi2_project_3d_to_2d(&impurities[o], camera_angle, 180.0f, WIDTH, HEIGHT, &sx, &sy);
            
            // Pulse size in sync with global beat (Lasseter Squash & Stretch)
            int radius = (int)(8 + fabsf(sinf(t * 4.0f)) * 2);
            tsfi2_draw_circle(canvas, sx, sy, radius, 155, 89, 182, 1);
            tsfi2_draw_circle(canvas, sx, sy, radius - 4, 255, 255, 255, 1);
        }

        // 6. Draw Local Perception Rays for Particle 0 (test agent)
        int sx0, sy0;
        tsfi2_project_3d_to_2d(&particles[0].curr, camera_angle, 180.0f, WIDTH, HEIGHT, &sx0, &sy0);
        for (int i = 1; i < NUM_PARTICLES; i++) {
            float dx = particles[0].curr.x - particles[i].curr.x;
            float dy = particles[0].curr.y - particles[i].curr.y;
            float dz = particles[0].curr.z - particles[i].curr.z;
            float dist = sqrtf(dx*dx + dy*dy + dz*dz);
            
            if (dist < 0.35f) {
                int sxn, syn;
                tsfi2_project_3d_to_2d(&particles[i].curr, camera_angle, 180.0f, WIDTH, HEIGHT, &sxn, &syn);
                tsfi2_draw_line_aa(canvas, sx0, sy0, sxn, syn, 47, 128, 237, 0.4f);
            }
        }

        // 7. Draw standard particles (green)
        for (int i = 1; i < NUM_PARTICLES; i++) {
            int sx, sy;
            tsfi2_project_3d_to_2d(&particles[i].curr, camera_angle, 180.0f, WIDTH, HEIGHT, &sx, &sy);
            tsfi2_draw_circle(canvas, sx, sy, 4, 46, 204, 113, 1);
            tsfi2_draw_circle(canvas, sx, sy, 2, 255, 255, 255, 1);
        }

        // Draw highlighted test agent (red)
        tsfi2_draw_circle(canvas, sx0, sy0, 7, 235, 87, 87, 1);
        tsfi2_draw_circle(canvas, sx0, sy0, 4, 255, 255, 255, 1);

        // 8. Draw springy background spectrum bars (Secondary Action & Follow Through)
        int num_bars = 16;
        int bar_width = 25;
        int gap = 10;
        int start_x = (WIDTH - (num_bars * bar_width + (num_bars - 1) * gap)) / 2;
        
        for (int b = 0; b < num_bars; b++) {
            float target_h = 20.0f + 40.0f * fabsf(sinf(t * 4.0f + b * 0.3f) * sinf(t * 12.0f + b * 0.5f));
            // Inertial decay: spring follows target with slight lag (Principle 5)
            bar_heights[b] = bar_heights[b] * 0.85f + target_h * 0.15f;
            
            int bx0 = start_x + b * (bar_width + gap);
            int by0 = HEIGHT - 10;
            int bx1 = bx0 + bar_width;
            int by1 = by0 - (int)bar_heights[b];
            
            for (int xb = bx0; xb < bx1; xb++) {
                tsfi2_draw_line_glow(canvas, xb, by0, xb, by1, 242, 100 + (int)bar_heights[b] * 2, 76);
            }
        }

        // Pipe frame to FFMPEG stdin
        fprintf(ffmpeg, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        fwrite(canvas->data, 1, WIDTH * HEIGHT * 3, ffmpeg);
    }

    tsfi2_canvas_free(canvas);
    pclose(ffmpeg);
    printf("✓ Successfully compiled papers verification video to: assets/papers_demo.mp4\n");
    return 0;
}

#include "../src/auncient_turtle_usd.h"
#include "../src/cloth_simulator.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WIDTH 320
#define HEIGHT 240
#define FPS 24
#define DURATION_SEC 120
#define TOTAL_FRAMES (FPS * DURATION_SEC) // 2880 frames
#define NUM_THREADS 4

// Frame buffers
static uint8_t raw_fb[WIDTH * HEIGHT * 3];
static uint8_t final_fb[WIDTH * HEIGHT * 3];

// 3D Point and Vector definitions
typedef struct {
    double x, y, z;
} Point3D;

// Pure 3D Turtle Graphics State
typedef struct {
    Point3D pos;
    double yaw;   // Angle in X-Z plane
    double pitch; // Angle relative to X-Z plane
} Turtle3D;

// Thread arguments for parallel Super 8 post-processing
typedef struct {
    int thread_id;
    double flicker;
} ThreadArgs;

// Verlet node structure for physics-based turtle graphics
typedef struct {
    Point3D pos;
    Point3D prev;
    int anchored;
    Point3D target;
} VerletNode;

// Helper to calculate distance
static double dist_3d(Point3D a, Point3D b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    double dz = a.z - b.z;
    return sqrt(dx*dx + dy*dy + dz*dz);
}

// Global Verlet systems for the Bear and the Moth
static VerletNode bear_nodes[7]; // 0: pelvis, 1: heart, 2: head, 3: l_hand, 4: r_hand, 5: l_foot, 6: r_foot
static VerletNode moth_nodes[14]; // 0: root, 1: head, 2..7: joint fibers, 8..13: tip fibers
static int physics_initialized = 0;

// Initialize Verlet nodes
static void init_verlet_physics(void) {
    memset(bear_nodes, 0, sizeof(bear_nodes));
    memset(moth_nodes, 0, sizeof(moth_nodes));
    physics_initialized = 1;
}

// Turtle graphics 3D command state mutators
static void turtle_forward(Turtle3D *t, double dist, Point3D *out_p) {
    double rad_yaw = t->yaw * M_PI / 180.0;
    double rad_pitch = t->pitch * M_PI / 180.0;
    t->pos.x += dist * cos(rad_pitch) * cos(rad_yaw);
    t->pos.y += dist * sin(rad_pitch);
    t->pos.z += dist * cos(rad_pitch) * sin(rad_yaw);
    if (out_p) {
        *out_p = t->pos;
    }
}

static void turtle_turn_yaw(Turtle3D *t, double deg) {
    t->yaw += deg;
}

static void turtle_turn_pitch(Turtle3D *t, double deg) {
    t->pitch += deg;
}

// Cohen-Sutherland clipping codes
#define INSIDE 0 // 0000
#define LEFT 1   // 0001
#define RIGHT 2  // 0010
#define BOTTOM 4 // 0100
#define TOP 8    // 1000

static int compute_out_code(double x, double y) {
    int code = INSIDE;
    if (x < 0)           code |= LEFT;
    else if (x >= WIDTH)  code |= RIGHT;
    if (y < 0)           code |= BOTTOM;
    else if (y >= HEIGHT) code |= TOP;
    return code;
}

static bool clip_line(int *x0, int *y0, int *x1, int *y1) {
    double x0_d = *x0, y0_d = *y0, x1_d = *x1, y1_d = *y1;
    int code0 = compute_out_code(x0_d, y0_d);
    int code1 = compute_out_code(x1_d, y1_d);
    bool accept = false;

    while (true) {
        if ((code0 | code1) == 0) {
            accept = true;
            break;
        } else if (code0 & code1) {
            break;
        } else {
            double x = 0, y = 0;
            int outcode_out = code0 ? code0 : code1;

            if (outcode_out & TOP) {
                x = x0_d + (x1_d - x0_d) * (HEIGHT - 1 - y0_d) / (y1_d - y0_d);
                y = HEIGHT - 1;
            } else if (outcode_out & BOTTOM) {
                x = x0_d + (x1_d - x0_d) * (0 - y0_d) / (y1_d - y0_d);
                y = 0;
            } else if (outcode_out & RIGHT) {
                y = y0_d + (y1_d - y0_d) * (WIDTH - 1 - x0_d) / (x1_d - x0_d);
                x = WIDTH - 1;
            } else if (outcode_out & LEFT) {
                y = y0_d + (y1_d - y0_d) * (0 - x0_d) / (x1_d - x0_d);
                x = 0;
            }

            if (outcode_out == code0) {
                x0_d = x;
                y0_d = y;
                code0 = compute_out_code(x0_d, y0_d);
            } else {
                x1_d = x;
                y1_d = y;
                code1 = compute_out_code(x1_d, y1_d);
            }
        }
    }
    if (accept) {
        *x0 = (int)x0_d;
        *y0 = (int)y0_d;
        *x1 = (int)x1_d;
        *y1 = (int)y1_d;
        return true;
    }
    return false;
}

// Simple Bresenham line drawer with variable width and opacity for blur simulation
static void draw_line_blur(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, int thickness, float opacity) {
    if (!clip_line(&x0, &y0, &x1, &y1)) {
        return; // Line is entirely offscreen
    }

    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    int safety_limit = 1000;
    while (safety_limit-- > 0) {
        for (int tx = -thickness / 2; tx <= thickness / 2; tx++) {
            for (int ty = -thickness / 2; ty <= thickness / 2; ty++) {
                int px = x0 + tx;
                int py = y0 + ty;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) {
                    int idx = (py * WIDTH + px) * 3;
                    raw_fb[idx] = (uint8_t)(raw_fb[idx] * (1.0f - opacity) + r * opacity);
                    raw_fb[idx + 1] = (uint8_t)(raw_fb[idx + 1] * (1.0f - opacity) + g * opacity);
                    raw_fb[idx + 2] = (uint8_t)(raw_fb[idx + 2] * (1.0f - opacity) + b * opacity);
                }
            }
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

// 3D Point projection helper with depth guard to prevent division by zero
static bool project_point(Point3D p, double cam_x, double cam_y, double cam_z, double time_val, int *sx, int *sy) {
    double depth = cam_z - p.z;
    if (depth < 10.0) {
        *sx = -9999;
        *sy = -9999;
        return false;
    }
    double scale = (250.0 + sin(time_val * 3.0) * 10.0) / depth;
    *sx = (int)(160.0 + (p.x - cam_x) * scale);
    *sy = (int)(120.0 + (p.y - cam_y) * scale);
    return true;
}

// Draw a wireframe 3D sphere using pure 3D turtle commands
static void draw_sphere_turtle(Point3D center, double radius, double cam_x, double cam_y, double cam_z, double time_val, uint8_t r, uint8_t g, uint8_t b) {
    int steps = 6;
    for (int lat = 0; lat < steps; lat++) {
        double angle_lat = (double)lat * 180.0 / steps - 90.0;
        Turtle3D t = { center, 0.0, angle_lat };
        Point3D prev = t.pos;
        for (int lon = 0; lon <= steps * 2; lon++) {
            turtle_turn_yaw(&t, 360.0 / (steps * 2));
            Point3D next;
            turtle_forward(&t, radius * sin(180.0 / (steps * 2) * M_PI / 180.0), &next);
            
            int sx0, sy0, sx1, sy1;
            if (project_point(prev, cam_x, cam_y, cam_z, time_val, &sx0, &sy0) &&
                project_point(next, cam_x, cam_y, cam_z, time_val, &sx1, &sy1)) {
                draw_line_blur(sx0, sy0, sx1, sy1, r, g, b, 1, 0.5f);
            }
            prev = next;
        }
    }
}

// Multi-threaded Super 8 camera post-processing worker with Kodak film sepia tinting
static void *super8_thread_worker(void *arg) {
    ThreadArgs *t_args = (ThreadArgs *)arg;
    int chunk = HEIGHT / NUM_THREADS;
    int start_y = t_args->thread_id * chunk;
    int end_y = (t_args->thread_id == NUM_THREADS - 1) ? HEIGHT : start_y + chunk;
    double flicker = t_args->flicker;

    for (int y = start_y; y < end_y; y++) {
        for (int x = 0; x < WIDTH; x++) {
            int out_idx = (y * WIDTH + x) * 3;

            // Vignette shading
            double dx_v = (x - WIDTH * 0.5) / (WIDTH * 0.5);
            double dy_v = (y - HEIGHT * 0.5) / (HEIGHT * 0.5);
            double vignette = fmax(0.0, 1.0 - (dx_v*dx_v + dy_v*dy_v) * 0.58);

            // Chromatic aberration color fringe shift
            int shift = (int)((dx_v*dx_v + dy_v*dy_v) * 4.5);
            int r_x = x - shift; if (r_x < 0) r_x = 0;
            int b_x = x + shift; if (b_x >= WIDTH) b_x = WIDTH - 1;

            int r_idx = (y * WIDTH + r_x) * 3;
            int b_idx = (y * WIDTH + b_x) * 3;

            uint8_t r_val = raw_fb[r_idx];
            uint8_t g_val = raw_fb[out_idx + 1];
            uint8_t b_val = raw_fb[b_idx + 2];

            // Coarse grain structure
            double grain = 1.0 + ((double)rand() / RAND_MAX - 0.5) * 0.12;

            int r_fin = (int)(r_val * flicker * vignette * grain);
            int g_fin = (int)(g_val * flicker * vignette * grain);
            int b_fin = (int)(b_val * flicker * vignette * grain);

            // Apply Sepia Warm Film Tinting (boosting Red/Green, lowering Blue)
            r_fin = (int)(r_fin * 1.18);
            g_fin = (int)(g_fin * 0.98);
            b_fin = (int)(b_fin * 0.74);

            if (r_fin > 255) r_fin = 255;
            if (g_fin > 255) g_fin = 255;
            if (b_fin > 255) b_fin = 255;

            final_fb[out_idx] = (uint8_t)r_fin;
            final_fb[out_idx + 1] = (uint8_t)g_fin;
            final_fb[out_idx + 2] = (uint8_t)b_fin;
        }
    }
    return NULL;
}

// Verlet update helper
static void update_verlet_node(VerletNode *node, double dt, double stiffness, double damping) {
    if (node->anchored) {
        node->pos = node->target;
        node->prev = node->target;
    } else {
        Point3D temp = node->pos;
        // Hooke's Law spring acceleration pulling toward cinematic target position
        double ax = (node->target.x - node->pos.x) * stiffness;
        double ay = (node->target.y - node->pos.y) * stiffness - 9.8; // Minor gravity pull
        double az = (node->target.z - node->pos.z) * stiffness;

        node->pos.x += (node->pos.x - node->prev.x) * damping + ax * dt * dt;
        node->pos.y += (node->pos.y - node->prev.y) * damping + ay * dt * dt;
        node->pos.z += (node->pos.z - node->prev.z) * damping + az * dt * dt;
        node->prev = temp;
    }
}

// Satisfy constraint helper
static void satisfy_distance(VerletNode *n1, VerletNode *n2, double rest_len) {
    double d = dist_3d(n1->pos, n2->pos);
    if (d < 0.001) return;
    double diff = (rest_len - d) / d * 0.5;
    Point3D offset = { (n1->pos.x - n2->pos.x) * diff, (n1->pos.y - n2->pos.y) * diff, (n1->pos.z - n2->pos.z) * diff };

    if (!n1->anchored) {
        n1->pos.x += offset.x;
        n1->pos.y += offset.y;
        n1->pos.z += offset.z;
    }
    if (!n2->anchored) {
        n2->pos.x -= offset.x;
        n2->pos.y -= offset.y;
        n2->pos.z -= offset.z;
    }
}

int main(void) {
    // Disable stdout buffering immediately so low-level fd redirection works cleanly without leaks
    setvbuf(stdout, NULL, _IONBF, 0);

    // Redirect stdout to stderr temporarily to prevent library printfs from polluting the raw video pipe
    int real_stdout = dup(1);
    dup2(2, 1);

    TSFiRiInterface ri;
    tsfi_riinterface_init(&ri);
    auncient_turtle_generate_usd_with_ri(&ri, "F", 1, 10.0f, 90.0f, "tests/render_temp.usda");

    const TurtlePoint3D *points_buf = auncient_turtle_get_points_buffer();
    uint32_t curve_count = auncient_turtle_get_curve_count();
    const uint32_t *curve_lengths = auncient_turtle_get_curve_lengths();

    (void)points_buf;
    (void)curve_count;
    (void)curve_lengths;

    cloth_init();
    init_verlet_physics();

    // Restore stdout back to the pipe for clean raw video streaming
    fflush(stdout);
    dup2(real_stdout, 1);
    close(real_stdout);

    static const int midi_lead[128] = {
        69, 71, 72, 76, 74, 72, 71, 69, 68, 71, 64, 68, 71, 74, 72, 71,
        69, 71, 72, 76, 81, 79, 77, 76, 74, 77, 71, 74, 72, 71, 69, 0,
        72, 74, 76, 79, 77, 76, 74, 72, 71, 72, 74, 77, 76, 74, 72, 71,
        69, 71, 72, 76, 81, 79, 77, 76, 74, 77, 71, 74, 72, 71, 69, 0,
        69, 71, 72, 76, 74, 72, 71, 69, 68, 71, 64, 68, 71, 74, 72, 71,
        69, 71, 72, 76, 81, 79, 77, 76, 74, 77, 71, 74, 72, 71, 69, 0,
        69, 71, 72, 76, 81, 84, 88, 93, 93, 0, 93, 0, 93, 0, 93, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    int last_note_idx = -1;

    // Render loop
    for (int frame = 0; frame < TOTAL_FRAMES; frame++) {
        memset(raw_fb, 14, sizeof(raw_fb));

        double time_val = (double)frame / FPS;
        int note_idx = (frame / (TOTAL_FRAMES / 128)) % 128;
        int active_note_pitch = midi_lead[note_idx];

        // Centered stable camera with random gate-weave jitter to simulate film shake
        double weave_x = ((double)rand() / RAND_MAX - 0.5) * 1.4;
        double weave_y = ((double)rand() / RAND_MAX - 0.5) * 1.4;

        // Perfectly center composition at visual Y center (15.0) and X center (0.0)
        double cam_x = 0.0 + weave_x;
        double cam_y = 15.0 + weave_y;
        double cam_z = 240.0;

        // Animated character target coordinates (Stuffed Teddy Bear model)
        double swing = sin(time_val * 4.0) * 8.0;
        Point3D root_pos = { 0.0, -20.0, 0.0 };

        // Turtle base state
        Turtle3D t_base = { root_pos, 90.0, 90.0 };

        // Spine targets
        Point3D pelvis_target = t_base.pos;
        Point3D heart_target;
        turtle_forward(&t_base, 25.0, &heart_target);
        Point3D head_target;
        turtle_forward(&t_base, 25.0, &head_target);

        // Limbs targets using Push/Pop Turtle States
        Turtle3D t_larm = t_base;
        t_larm.pos = heart_target;
        turtle_turn_yaw(&t_larm, -90.0);
        turtle_turn_pitch(&t_larm, swing);
        Point3D l_hand_target;
        turtle_forward(&t_larm, 25.0, &l_hand_target);

        Turtle3D t_rarm = t_base;
        t_rarm.pos = heart_target;
        turtle_turn_yaw(&t_rarm, 90.0);
        turtle_turn_pitch(&t_rarm, -swing);
        Point3D r_hand_target;
        turtle_forward(&t_rarm, 25.0, &r_hand_target);

        Turtle3D t_lleg = { pelvis_target, 90.0, -90.0 };
        turtle_turn_yaw(&t_lleg, -30.0);
        turtle_turn_pitch(&t_lleg, -swing);
        Point3D l_foot_target;
        turtle_forward(&t_lleg, 25.0, &l_foot_target);

        Turtle3D t_rleg = { pelvis_target, 90.0, -90.0 };
        turtle_turn_yaw(&t_rleg, 30.0);
        turtle_turn_pitch(&t_rleg, swing);
        Point3D r_foot_target;
        turtle_forward(&t_rleg, 25.0, &r_foot_target);

        // Set targets and anchor state for Bear Verlet Nodes
        bear_nodes[0].target = pelvis_target; bear_nodes[0].anchored = 1;
        bear_nodes[1].target = heart_target;  bear_nodes[1].anchored = 1;
        bear_nodes[2].target = head_target;   bear_nodes[2].anchored = 1;
        bear_nodes[3].target = l_hand_target; bear_nodes[3].anchored = 0;
        bear_nodes[4].target = r_hand_target; bear_nodes[4].anchored = 0;
        bear_nodes[5].target = l_foot_target; bear_nodes[5].anchored = 0;
        bear_nodes[6].target = r_foot_target; bear_nodes[6].anchored = 0;

        if (frame == 0) {
            for (int k = 0; k < 7; k++) {
                bear_nodes[k].pos = bear_nodes[k].target;
                bear_nodes[k].prev = bear_nodes[k].target;
            }
        }

        // Apply Verlet updates to Bear Nodes
        double dt = 0.04;
        for (int k = 0; k < 7; k++) {
            update_verlet_node(&bear_nodes[k], dt, 120.0, 0.88); // Soft rubbery spring
        }

        // Resolve distance constraints for bear skeleton limbs
        for (int iteration = 0; iteration < 4; iteration++) {
            satisfy_distance(&bear_nodes[0], &bear_nodes[1], 25.0);
            satisfy_distance(&bear_nodes[1], &bear_nodes[2], 25.0);
            satisfy_distance(&bear_nodes[1], &bear_nodes[3], 25.0);
            satisfy_distance(&bear_nodes[1], &bear_nodes[4], 25.0);
            satisfy_distance(&bear_nodes[0], &bear_nodes[5], 25.0);
            satisfy_distance(&bear_nodes[0], &bear_nodes[6], 25.0);
        }

        // Project and Draw bear limbs from Verlet nodes (Guarded against behind-camera errors)
        int sh_x, sh_y, sp_x, sp_y, slh_x, slh_y, srh_x, srh_y, slf_x, slf_y, srf_x, srf_y;
        bool b_head = project_point(bear_nodes[2].pos, cam_x, cam_y, cam_z, time_val, &sh_x, &sh_y);
        bool b_pelvis = project_point(bear_nodes[0].pos, cam_x, cam_y, cam_z, time_val, &sp_x, &sp_y);
        bool b_lh = project_point(bear_nodes[3].pos, cam_x, cam_y, cam_z, time_val, &slh_x, &slh_y);
        bool b_rh = project_point(bear_nodes[4].pos, cam_x, cam_y, cam_z, time_val, &srh_x, &srh_y);
        bool b_lf = project_point(bear_nodes[5].pos, cam_x, cam_y, cam_z, time_val, &slf_x, &slf_y);
        bool b_rf = project_point(bear_nodes[6].pos, cam_x, cam_y, cam_z, time_val, &srf_x, &srf_y);

        // Draw Head and Ears/Eyes centered on Verlet head pos
        if (b_head) {
            draw_sphere_turtle(bear_nodes[2].pos, 18.0, cam_x, cam_y, cam_z, time_val, 200, 160, 100);

            Point3D ear_l = { bear_nodes[2].pos.x - 15.0, bear_nodes[2].pos.y + 12.0, bear_nodes[2].pos.z - 2.0 };
            Point3D ear_r = { bear_nodes[2].pos.x + 15.0, bear_nodes[2].pos.y + 12.0, bear_nodes[2].pos.z - 2.0 };
            draw_sphere_turtle(ear_l, 6.0, cam_x, cam_y, cam_z, time_val, 200, 160, 100);
            draw_sphere_turtle(ear_r, 6.0, cam_x, cam_y, cam_z, time_val, 200, 160, 100);

            Point3D eye_l = { bear_nodes[2].pos.x - 6.0, bear_nodes[2].pos.y + 2.0, bear_nodes[2].pos.z + 16.0 };
            Point3D eye_r = { bear_nodes[2].pos.x + 6.0, bear_nodes[2].pos.y + 2.0, bear_nodes[2].pos.z + 16.0 };
            draw_sphere_turtle(eye_l, 2.0, cam_x, cam_y, cam_z, time_val, 10, 10, 10);
            draw_sphere_turtle(eye_r, 2.0, cam_x, cam_y, cam_z, time_val, 10, 10, 10);
        }

        if (b_head && b_pelvis) {
            draw_line_blur(sh_x, sh_y, sp_x, sp_y, 180, 120, 80, 4, 0.8f);
        }
        if (b_head && b_lh) {
            draw_line_blur(sh_x - 10, sh_y + 10, slh_x, slh_y, 180, 120, 80, 2, 0.8f);
        }
        if (b_head && b_rh) {
            draw_line_blur(sh_x + 10, sh_y + 10, srh_x, srh_y, 180, 120, 80, 2, 0.8f);
        }
        if (b_pelvis && b_lf) {
            draw_line_blur(sp_x, sp_y, slf_x, slf_y, 180, 120, 80, 2, 0.8f);
        }
        if (b_pelvis && b_rf) {
            draw_line_blur(sp_x, sp_y, srf_x, srf_y, 180, 120, 80, 2, 0.8f);
        }

        // Sewn Heart (pulsing)
        int sheart_x, sheart_y;
        if (project_point(bear_nodes[1].pos, cam_x, cam_y, cam_z, time_val, &sheart_x, &sheart_y)) {
            int pulse_thick = 4 + (active_note_pitch % 4);
            draw_line_blur(sheart_x, sheart_y - 2, sheart_x - 5, sheart_y + 5, 255, 20, 40, pulse_thick, 0.95f);
            draw_line_blur(sheart_x, sheart_y - 2, sheart_x + 5, sheart_y + 5, 255, 20, 40, pulse_thick, 0.95f);
        }

        // 2. Draw NIH 3D Public Domain DNA Double Helix Model (Red Ginseng Root - Left)
        double dna_radius = 16.0;
        int dna_steps = 45;
        double dna_y_start = -50.0;
        double dna_y_end = 80.0;
        double dna_y_step = (dna_y_end - dna_y_start) / dna_steps;
        double dna_rot_speed = 0.25;

        double dna_cx = -45.0;
        double dna_cy = 0.0;
        double dna_cz = 0.0;

        for (int i = 0; i < dna_steps; i++) {
            double angle = (double)i * 0.45 + time_val * dna_rot_speed;
            Point3D dna_p1 = { dna_cx + dna_radius * cos(angle), dna_cy + dna_y_start + i * dna_y_step, dna_cz + dna_radius * sin(angle) };
            Point3D dna_p2 = { dna_cx + dna_radius * cos(angle + M_PI), dna_cy + dna_y_start + i * dna_y_step, dna_cz + dna_radius * sin(angle + M_PI) };

            int sx_dna1, sy_dna1, sx_dna2, sy_dna2;
            bool v1 = project_point(dna_p1, cam_x, cam_y, cam_z, time_val, &sx_dna1, &sy_dna1);
            bool v2 = project_point(dna_p2, cam_x, cam_y, cam_z, time_val, &sx_dna2, &sy_dna2);

            if (v1 && v2 && i % 2 == 0) {
                draw_line_blur(sx_dna1, sy_dna1, sx_dna2, sy_dna2, 255, 50, 50, 1, 0.7f); // Red ginseng rung
            }

            if (i > 0) {
                double prev_angle = (double)(i - 1) * 0.45 + time_val * dna_rot_speed;
                Point3D prev_dna_p1 = { dna_cx + dna_radius * cos(prev_angle), dna_cy + dna_y_start + (i - 1) * dna_y_step, dna_cz + dna_radius * sin(prev_angle) };
                Point3D prev_dna_p2 = { dna_cx + dna_radius * cos(prev_angle + M_PI), dna_cy + dna_y_start + (i - 1) * dna_y_step, dna_cz + dna_radius * sin(prev_angle + M_PI) };

                int prev_sx1, prev_sy1, prev_sx2, prev_sy2;
                if (project_point(prev_dna_p1, cam_x, cam_y, cam_z, time_val, &prev_sx1, &prev_sy1) && v1) {
                    draw_line_blur(prev_sx1, prev_sy1, sx_dna1, sy_dna1, 255, 30, 30, 2, 0.85f); // Red backbone
                }
                if (project_point(prev_dna_p2, cam_x, cam_y, cam_z, time_val, &prev_sx2, &prev_sy2) && v2) {
                    draw_line_blur(prev_sx2, prev_sy2, sx_dna2, sy_dna2, 255, 30, 30, 2, 0.85f); // Red backbone
                }
            }
        }

        // Draw Red DNA Root (on the Right, X = 45.0) - stationary root
        for (int i = 0; i < dna_steps; i++) {
            double angle = (double)i * 0.45 + time_val * dna_rot_speed;
            Point3D dna_p1 = { 45.0 + dna_radius * cos(angle), dna_y_start + i * dna_y_step, dna_radius * sin(angle) };
            Point3D dna_p2 = { 45.0 + dna_radius * cos(angle + M_PI), dna_y_start + i * dna_y_step, dna_radius * sin(angle + M_PI) };

            int sx_dna1, sy_dna1, sx_dna2, sy_dna2;
            bool v1 = project_point(dna_p1, cam_x, cam_y, cam_z, time_val, &sx_dna1, &sy_dna1);
            bool v2 = project_point(dna_p2, cam_x, cam_y, cam_z, time_val, &sx_dna2, &sy_dna2);

            if (v1 && v2 && i % 2 == 0) {
                draw_line_blur(sx_dna1, sy_dna1, sx_dna2, sy_dna2, 255, 50, 50, 1, 0.7f); // Red root rung
            }

            if (i > 0) {
                double prev_angle = (double)(i - 1) * 0.45 + time_val * dna_rot_speed;
                Point3D prev_dna_p1 = { 45.0 + dna_radius * cos(prev_angle), dna_y_start + (i - 1) * dna_y_step, dna_radius * sin(prev_angle) };
                Point3D prev_dna_p2 = { 45.0 + dna_radius * cos(prev_angle + M_PI), dna_y_start + (i - 1) * dna_y_step, dna_radius * sin(prev_angle + M_PI) };

                int prev_sx1, prev_sy1, prev_sx2, prev_sy2;
                if (project_point(prev_dna_p1, cam_x, cam_y, cam_z, time_val, &prev_sx1, &prev_sy1) && v1) {
                    draw_line_blur(prev_sx1, prev_sy1, sx_dna1, sy_dna1, 255, 30, 30, 2, 0.85f); // Red backbone
                }
                if (project_point(prev_dna_p2, cam_x, cam_y, cam_z, time_val, &prev_sx2, &prev_sy2) && v2) {
                    draw_line_blur(prev_sx2, prev_sy2, sx_dna2, sy_dna2, 255, 30, 30, 2, 0.85f); // Red backbone
                }
            }
        }

        // 3. Draw Red Moth (Delicate small moth model - NO phage/root elements)
        double takeoff_duration = 5.0; // Take off over 5 seconds
        double takeoff_factor = time_val / takeoff_duration;
        if (takeoff_factor > 1.0) takeoff_factor = 1.0;

        // Resting coordinates on the Purple Root (middle Y)
        double ph_x_rest = 45.0 + dna_radius * cos(0.0);
        double ph_y_rest = 0.0;
        double ph_z_rest = dna_radius * sin(0.0);

        // Flight coordinates (dynamic 3D Lissajous sweeps all around the screen)
        double ph_x_fly = 85.0 * sin(time_val * 0.65);
        double ph_y_fly = 15.0 + sin(time_val * 1.3) * 30.0;
        double ph_z_fly = 75.0 * cos(time_val * 0.45);

        // Smoothly interpolate coordinate states
        double ph_x = ph_x_rest * (1.0 - takeoff_factor) + ph_x_fly * takeoff_factor;
        double ph_y = ph_y_rest * (1.0 - takeoff_factor) + ph_y_fly * takeoff_factor;
        double ph_z = ph_z_rest * (1.0 - takeoff_factor) + ph_z_fly * takeoff_factor;

        // Path derivatives (velocity vector)
        double vx = (ph_x_fly - ph_x_rest) / takeoff_duration;
        double vz = (ph_z_fly - ph_z_rest) / takeoff_duration;
        double vy = (ph_y_fly - ph_y_rest) / takeoff_duration;
        if (takeoff_factor >= 1.0) {
            vx = 85.0 * 0.65 * cos(time_val * 0.65);
            vy = 30.0 * 1.3 * cos(time_val * 1.3);
            vz = -75.0 * 0.45 * sin(time_val * 0.45);
        }

        double v_len = sqrt(vx*vx + vy*vy + vz*vz);
        Point3D ph_root_target = { ph_x, ph_y, ph_z };
        Point3D ph_head_target;

        if (v_len > 0.001) {
            ph_head_target.x = ph_x + (vx / v_len) * 6.0;
            ph_head_target.y = ph_y + 15.0 + (vy / v_len) * 2.0;
            ph_head_target.z = ph_z + (vz / v_len) * 6.0;
        } else {
            ph_head_target = (Point3D){ ph_x, ph_y + 15.0, ph_z };
        }

        // Set targets and anchor state for Moth Verlet Nodes
        moth_nodes[0].target = ph_root_target; moth_nodes[0].anchored = 1;
        moth_nodes[1].target = ph_head_target; moth_nodes[1].anchored = 1;

        if (frame == 0) {
            for (int k = 0; k < 2; k++) {
                moth_nodes[k].pos = moth_nodes[k].target;
                moth_nodes[k].prev = moth_nodes[k].target;
            }
        }

        // Apply Verlet updates to Moth Nodes
        for (int k = 0; k < 2; k++) {
            update_verlet_node(&moth_nodes[k], dt, 140.0, 0.85);
        }

        // Resolve distance constraints for Moth skeleton limbs
        for (int iteration = 0; iteration < 4; iteration++) {
            satisfy_distance(&moth_nodes[0], &moth_nodes[1], 15.0);
        }

        // Draw small red head on Verlet head node (Guarded)
        bool m_head_ok = false;
        int m_hx = 0, m_hy = 0;
        if (project_point(moth_nodes[1].pos, cam_x, cam_y, cam_z, time_val, &m_hx, &m_hy)) {
            draw_sphere_turtle(moth_nodes[1].pos, 5.0, cam_x, cam_y, cam_z, time_val, 255, 40, 40); // Red Capsid
            m_head_ok = true;
        }

        // Project and Draw small Moth body line from Verlet nodes (Guarded)
        int sh_base_x, sh_base_y;
        bool m_root_ok = project_point(moth_nodes[0].pos, cam_x, cam_y, cam_z, time_val, &sh_base_x, &sh_base_y);
        if (m_root_ok && m_head_ok) {
            draw_line_blur(sh_base_x, sh_base_y, m_hx, m_hy, 255, 40, 40, 2, 0.8f); // Red body
        }

        // Draw small flapping wings (ensuring it is visually identified as a small moth)
        if (m_root_ok) {
            double wing_span = 14.0;
            double flap_y = sin(time_val * 24.0) * 8.0; // Flapping frequency
            Point3D wing_l_tip = { moth_nodes[0].pos.x - wing_span, moth_nodes[0].pos.y + flap_y, moth_nodes[0].pos.z - 3.0 };
            Point3D wing_r_tip = { moth_nodes[0].pos.x + wing_span, moth_nodes[0].pos.y + flap_y, moth_nodes[0].pos.z - 3.0 };
            int wl_x, wl_y, wr_x, wr_y;
            if (project_point(wing_l_tip, cam_x, cam_y, cam_z, time_val, &wl_x, &wl_y)) {
                draw_line_blur(sh_base_x, sh_base_y, wl_x, wl_y, 255, 40, 40, 1, 0.75f);
            }
            if (project_point(wing_r_tip, cam_x, cam_y, cam_z, time_val, &wr_x, &wr_y)) {
                draw_line_blur(sh_base_x, sh_base_y, wr_x, wr_y, 255, 40, 40, 1, 0.75f);
            }
        }

        // T4 Phage moth removed entirely as requested



        // Verlet Soft-Body Physics tethered to the pelvis
        float perturb_x = (float)(bear_nodes[0].pos.x + sin(time_val * 6.0) * 3.0);
        float perturb_z = (float)(bear_nodes[0].pos.z + cos(time_val * 6.0) * 3.0);
        if (note_idx != last_note_idx) {
            double freq = 440.0 * pow(2.0, (midi_lead[note_idx] - 69) / 12.0);
            perturb_x += (float)(sin(freq * 0.15) * 4.0f);
            perturb_z += (float)(cos(freq * 0.15) * 4.0f);
            last_note_idx = note_idx;
        }
        cloth_set_anchor(0, CLOTH_HEIGHT - 1, perturb_x, (float)bear_nodes[0].pos.y + 10.0f, perturb_z);
        cloth_update(0.02f * perturb_x, -0.01f, 0.02f * perturb_z);

        // 4. Draw Floor Plane Grid (Fully guarded and Cohen-Sutherland clipped inside draw_line_blur)
        for (int fx = -5; fx <= 5; fx++) {
            Point3D start_line_x = { fx * 30.0, -50.0, -150.0 };
            Point3D end_line_x = { fx * 30.0, -50.0, 150.0 };
            int sx0, sy0, sx1, sy1;
            if (project_point(start_line_x, cam_x, cam_y, cam_z, time_val, &sx0, &sy0) &&
                project_point(end_line_x, cam_x, cam_y, cam_z, time_val, &sx1, &sy1)) {
                draw_line_blur(sx0, sy0, sx1, sy1, 40, 70, 90, 1, 0.35f);
            }
        }
        for (int fz = -5; fz <= 5; fz++) {
            Point3D start_line_z = { -150.0, -50.0, fz * 30.0 };
            Point3D end_line_z = { 150.0, -50.0, fz * 30.0 };
            int sx0, sy0, sx1, sy1;
            if (project_point(start_line_z, cam_x, cam_y, cam_z, time_val, &sx0, &sy0) &&
                project_point(end_line_z, cam_x, cam_y, cam_z, time_val, &sx1, &sy1)) {
                draw_line_blur(sx0, sy0, sx1, sy1, 40, 70, 90, 1, 0.35f);
            }
        }

        // 5. Draw Physical Verlet Cloth (Cape) mesh trailing the bear (Guarded and clipped)
        static ClothVertex cloth_vertices[CLOTH_WIDTH * CLOTH_HEIGHT];
        int indices[CLOTH_WIDTH * CLOTH_HEIGHT * 6];
        int vc = 0, ic = 0;
        cloth_generate_mesh(cloth_vertices, indices, &vc, &ic);

        for (int cy = 0; cy < CLOTH_HEIGHT; cy++) {
            for (int cx = 0; cx < CLOTH_WIDTH; cx++) {
                int c_idx = cy * CLOTH_WIDTH + cx;
                double cz = cloth_vertices[c_idx].z * 12.0 + bear_nodes[0].pos.z - 40.0; 
                double scale = 250.0 / (cam_z - cz);

                double cloth_x = cloth_vertices[c_idx].x * 50.0 - 25.0 + bear_nodes[0].pos.x;
                double cloth_y = cloth_vertices[c_idx].y * 50.0 - 15.0 + bear_nodes[0].pos.y;

                int sx = (int)(160.0 + (cloth_x - cam_x) * scale);
                int sy = (int)(120.0 + (cloth_y - cam_y) * scale);

                if (cx < CLOTH_WIDTH - 1) {
                    int nidx = cy * CLOTH_WIDTH + (cx + 1);
                    double ncz = cloth_vertices[nidx].z * 12.0 + bear_nodes[0].pos.z - 40.0;
                    double nscale = 250.0 / (cam_z - ncz);
                    double ncloth_x = cloth_vertices[nidx].x * 50.0 - 25.0 + bear_nodes[0].pos.x;
                    double ncloth_y = cloth_vertices[nidx].y * 50.0 - 15.0 + bear_nodes[0].pos.y;
                    int nsx = (int)(160.0 + (ncloth_x - cam_x) * nscale);
                    int nsy = (int)(120.0 + (ncloth_y - cam_y) * nscale);
                    
                    draw_line_blur(sx, sy, nsx, nsy, 255, 100, 120, 1, 0.6f);
                }
                if (cy < CLOTH_HEIGHT - 1) {
                    int nidx = (cy + 1) * CLOTH_WIDTH + cx;
                    double ncz = cloth_vertices[nidx].z * 12.0 + bear_nodes[0].pos.z - 40.0;
                    double nscale = 250.0 / (cam_z - ncz);
                    double ncloth_x = cloth_vertices[nidx].x * 50.0 - 25.0 + bear_nodes[0].pos.x;
                    double ncloth_y = cloth_vertices[nidx].y * 50.0 - 15.0 + bear_nodes[0].pos.y;
                    int nsx = (int)(160.0 + (ncloth_x - cam_x) * nscale);
                    int nsy = (int)(120.0 + (ncloth_y - cam_y) * nscale);
                    
                    draw_line_blur(sx, sy, nsx, nsy, 255, 100, 120, 1, 0.6f);
                }
            }
        }

        // 6. Multi-Threaded post-processing pipeline
        pthread_t threads[NUM_THREADS];
        ThreadArgs t_args[NUM_THREADS];
        double flicker = 0.88 + ((double)rand() / RAND_MAX * 0.18) + sin(frame * 1.8) * 0.04;

        for (int i = 0; i < NUM_THREADS; i++) {
            t_args[i].thread_id = i;
            t_args[i].flicker = flicker;
            pthread_create(&threads[i], NULL, super8_thread_worker, &t_args[i]);
        }
        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(threads[i], NULL);
        }

        // Film gate dust and scratches (Increased density)
        if (rand() % 10 == 0) {
            int scratch_x = rand() % WIDTH;
            int scratch_len = 40 + rand() % 60;
            int scratch_y_start = rand() % (HEIGHT - scratch_len);
            for (int sy = scratch_y_start; sy < scratch_y_start + scratch_len; sy++) {
                int idx = (sy * WIDTH + scratch_x) * 3;
                final_fb[idx] = 6;
                final_fb[idx + 1] = 6;
                final_fb[idx + 2] = 8;
            }
        }

        if (rand() % 20 == 0) {
            int hair_x = 20 + rand() % (WIDTH - 40);
            int hair_y = 20 + rand() % (HEIGHT - 40);
            for (int h = -6; h <= 6; h++) {
                int idx = ((hair_y + h) * WIDTH + (hair_x + (int)(sin(h * 1.5) * 3.0))) * 3;
                if (idx >= 0 && idx < WIDTH * HEIGHT * 3) {
                    final_fb[idx] = 4;
                    final_fb[idx + 1] = 4;
                    final_fb[idx + 2] = 4;
                }
            }
        }

        fwrite(final_fb, 1, sizeof(final_fb), stdout);
    }

    return 0;
}

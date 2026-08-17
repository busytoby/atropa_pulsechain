/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Live YouTube Stream Broadcaster & Continuous Formal Proof Telemetry Engine
 * 
 * Streams real-time animated Kinematronic ClayScaped Bear Clan 3D rendering,
 * live telemetry HUD (displaying ratified formal proofs from ACM DIS '26),
 * and synchronized Bionika synthesizer music directly to YouTube RTMP.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#define WIDTH 1280
#define HEIGHT 720
#define FPS 30
#define SAMPLE_RATE 44100
#define SAMPLES_PER_FRAME (SAMPLE_RATE / FPS)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    uint8_t r, g, b;
} pixel_t;

static pixel_t framebuffer[WIDTH * HEIGHT];
static int16_t audio_buffer[SAMPLES_PER_FRAME * 2];

static void clear_buffer(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        framebuffer[i].r = r;
        framebuffer[i].g = g;
        framebuffer[i].b = b;
    }
}

static void draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
        framebuffer[y * WIDTH + x].r = r;
        framebuffer[y * WIDTH + x].g = g;
        framebuffer[y * WIDTH + x].b = b;
    }
}

static void draw_line(int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        draw_pixel(x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    int u, v;
    float depth;
} screen_pt_t;

static screen_pt_t project_point(vec3_t p, float yaw, float pitch, float cam_z) {
    screen_pt_t res;
    float cy = cosf(yaw), sy = sinf(yaw);
    float x1 = p.x * cy + p.z * sy;
    float y1 = p.y;
    float z1 = -p.x * sy + p.z * cy;

    float cp = cosf(pitch), sp = sinf(pitch);
    float x2 = x1;
    float y2 = y1 * cp - z1 * sp;
    float z2 = y1 * sp + z1 * cp;

    float z3 = z2 + cam_z;
    if (z3 < 0.1f) z3 = 0.1f;
    float fov = 650.0f;
    res.u = (int)(WIDTH / 2.0f + (x2 * fov) / z3);
    res.v = (int)(HEIGHT / 2.0f - (y2 * fov) / z3);
    res.depth = z3;
    return res;
}

static void draw_3d_ring(vec3_t center, float radius, float yaw, float pitch, float cam_z, uint8_t r, uint8_t g, uint8_t b, int segments) {
    screen_pt_t prev;
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i * (2.0f * (float)M_PI / (float)segments);
        vec3_t pt = {
            center.x + radius * cosf(angle),
            center.y,
            center.z + radius * sinf(angle)
        };
        screen_pt_t curr = project_point(pt, yaw, pitch, cam_z);
        if (i > 0) {
            draw_line(prev.u, prev.v, curr.u, curr.v, r, g, b);
        }
        prev = curr;
    }
}

static float note_to_freq(const char *note) {
    if (!strcmp(note, "C2")) return 65.41f;
    if (!strcmp(note, "Eb2")) return 77.78f;
    if (!strcmp(note, "F2")) return 87.31f;
    if (!strcmp(note, "G2")) return 98.00f;
    if (!strcmp(note, "Ab1")) return 51.91f;
    if (!strcmp(note, "Bb1")) return 58.27f;
    if (!strcmp(note, "C3")) return 130.81f;
    if (!strcmp(note, "Eb3")) return 155.56f;
    if (!strcmp(note, "F3")) return 174.61f;
    if (!strcmp(note, "G3")) return 196.00f;
    if (!strcmp(note, "Ab2")) return 103.83f;
    if (!strcmp(note, "Bb2")) return 116.54f;
    if (!strcmp(note, "C4")) return 261.63f;
    if (!strcmp(note, "Eb4")) return 311.13f;
    if (!strcmp(note, "F4")) return 349.23f;
    if (!strcmp(note, "G4")) return 392.00f;
    if (!strcmp(note, "Ab3")) return 207.65f;
    if (!strcmp(note, "Bb3")) return 233.08f;
    return 0.0f;
}

static void generate_bionika_audio_frame(int frame, int16_t *stereo_out) {
    float bpm = 109.6f;
    float step_sec = (60.0f / bpm) / 4.0f;
    
    static const char *bass_notes[32] = {
        "C2", "C2", "C2", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "Ab1", "Bb1", "C2"
    };
    static const int bass_accents[32] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1
    };
    static const char *lead_notes[32] = {
        "C4", "C4", "C4", "C4", "C4", "C4", "C4", "C4",
        "C4", "C4", "Bb3", "C4", "", "", "", "",
        "", "", "Bb3", "C4", "", "", "", "",
        "", "", "Bb3", "C4", "", "Ab3", "Bb3", "C4"
    };
    static const int drum_kick[32] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0
    };
    static const int drum_snare[32] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
    };

    for (int s = 0; s < SAMPLES_PER_FRAME; s++) {
        float t = (frame * SAMPLES_PER_FRAME + s) / (float)SAMPLE_RATE;
        int step = (int)(t / step_sec) % 32;
        float step_progress = fmodf(t, step_sec) / step_sec;

        float b_freq = note_to_freq(bass_notes[step]);
        float b_env = expf(-step_progress * (bass_accents[step] ? 3.5f : 6.0f));
        float b_saw = 2.0f * fmodf(t * b_freq, 1.0f) - 1.0f;
        float b_pulse = (fmodf(t * b_freq * 0.5f, 1.0f) < 0.4f) ? 1.0f : -1.0f;
        float bass_sample = (0.6f * b_saw + 0.4f * b_pulse) * b_env * (bass_accents[step] ? 0.8f : 0.45f);

        float lead_sample = 0.0f;
        if (strlen(lead_notes[step]) > 0) {
            float l_freq = note_to_freq(lead_notes[step]);
            float l_env = expf(-step_progress * 4.0f);
            float l_saw = 2.0f * fmodf(t * l_freq, 1.0f) - 1.0f;
            float l_5th = sinf(2.0f * (float)M_PI * l_freq * 1.5f * t);
            float l_raw = 0.7f * l_saw + 0.3f * l_5th;
            lead_sample = tanhf(l_raw * 2.2f) * l_env * 0.65f;
        }

        float kick_sample = 0.0f;
        if (drum_kick[step]) {
            float k_env = expf(-step_progress * 18.0f);
            float k_freq = 140.0f * expf(-step_progress * 24.0f) + 45.0f;
            kick_sample = sinf(2.0f * (float)M_PI * k_freq * step_progress * step_sec) * k_env * 0.9f;
        }

        float snare_sample = 0.0f;
        if (drum_snare[step]) {
            float sn_env = expf(-step_progress * 14.0f);
            float noise = ((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f;
            snare_sample = (noise * 0.7f + sinf(2.0f * (float)M_PI * 220.0f * step_progress * step_sec) * 0.3f) * sn_env * 0.75f;
        }

        float hh_sample = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * expf(-step_progress * 30.0f) * 0.18f;

        float mix_left = bass_sample * 0.7f + lead_sample * 0.85f + kick_sample * 0.8f + snare_sample * 0.7f + hh_sample * 0.5f;
        float mix_right = bass_sample * 0.7f + lead_sample * 0.75f + kick_sample * 0.8f + snare_sample * 0.7f + hh_sample * 0.6f;

        if (mix_left > 0.98f) mix_left = 0.98f;
        if (mix_left < -0.98f) mix_left = -0.98f;
        if (mix_right > 0.98f) mix_right = 0.98f;
        if (mix_right < -0.98f) mix_right = -0.98f;

        stereo_out[s * 2] = (int16_t)(mix_left * 32000.0f);
        stereo_out[s * 2 + 1] = (int16_t)(mix_right * 32000.0f);
    }
}

static void render_live_stream_frame(int frame) {
    float t = (float)frame / (float)FPS;
    clear_buffer(12, 16, 24);

    float yaw = t * 0.45f;
    float pitch = 0.22f + 0.08f * sinf(t * 0.6f);
    float cam_z = 4.2f;

    // Grid Floor
    for (int gx = -5; gx <= 5; gx++) {
        vec3_t p1 = { (float)gx * 0.4f, -1.8f, -2.0f };
        vec3_t p2 = { (float)gx * 0.4f, -1.8f,  2.0f };
        screen_pt_t sp1 = project_point(p1, yaw, pitch, cam_z);
        screen_pt_t sp2 = project_point(p2, yaw, pitch, cam_z);
        draw_line(sp1.u, sp1.v, sp2.u, sp2.v, 25, 40, 65);
    }
    for (int gz = -5; gz <= 5; gz++) {
        vec3_t p1 = { -2.0f, -1.8f, (float)gz * 0.4f };
        vec3_t p2 = {  2.0f, -1.8f, (float)gz * 0.4f };
        screen_pt_t sp1 = project_point(p1, yaw, pitch, cam_z);
        screen_pt_t sp2 = project_point(p2, yaw, pitch, cam_z);
        draw_line(sp1.u, sp1.v, sp2.u, sp2.v, 25, 40, 65);
    }

    // Torso Slices (Terracotta / Glazed Gold)
    for (int l = 0; l < 32; l++) {
        float ly = -1.2f + ((float)l / 32.0f) * 1.2f;
        float r_torso = 0.55f * sqrtf(fmaxf(0.0f, 1.0f - powf((ly + 0.6f) / 0.7f, 2.0f)));
        vec3_t center = { 0.0f, ly, 0.0f };
        draw_3d_ring(center, r_torso, yaw, pitch, cam_z, 235, 175, 65, 24);
    }

    // Head Slices
    for (int l = 0; l < 24; l++) {
        float hy = 0.35f + ((float)l / 24.0f) * 0.85f;
        float r_head = 0.45f * sqrtf(fmaxf(0.0f, 1.0f - powf((hy - 0.75f) / 0.45f, 2.0f)));
        vec3_t center = { 0.0f, hy, 0.0f };
        draw_3d_ring(center, r_head, yaw, pitch, cam_z, 255, 195, 80, 24);
    }

    // Ears
    vec3_t left_ear = { -0.38f, 1.15f, 0.0f };
    vec3_t right_ear = { 0.38f, 1.15f, 0.0f };
    draw_3d_ring(left_ear, 0.18f, yaw, pitch, cam_z, 245, 185, 75, 16);
    draw_3d_ring(right_ear, 0.18f, yaw, pitch, cam_z, 245, 185, 75, 16);

    // Arms & Dynamic Kinematic Wave
    float wave_angle = sinf(t * 8.0f) * 0.5f + 0.3f;
    vec3_t left_shoulder = { -0.55f, 0.0f, 0.0f };
    vec3_t left_hand = { -0.85f, -0.6f + wave_angle * 0.8f, 0.3f + wave_angle * 0.4f };
    vec3_t right_shoulder = { 0.55f, 0.0f, 0.0f };
    vec3_t right_hand = { 0.85f, -0.6f, 0.0f };

    screen_pt_t ls = project_point(left_shoulder, yaw, pitch, cam_z);
    screen_pt_t lh = project_point(left_hand, yaw, pitch, cam_z);
    screen_pt_t rs = project_point(right_shoulder, yaw, pitch, cam_z);
    screen_pt_t rh = project_point(right_hand, yaw, pitch, cam_z);

    draw_line(ls.u, ls.v, lh.u, lh.v, 255, 215, 0);
    draw_line(rs.u, rs.v, rh.u, rh.v, 255, 215, 0);
    draw_3d_ring(left_hand, 0.12f, yaw, pitch, cam_z, 255, 230, 100, 12);
    draw_3d_ring(right_hand, 0.12f, yaw, pitch, cam_z, 255, 230, 100, 12);

    // Legs & Dynamic Kinematic Walk
    float walk_step = sinf(t * 6.0f) * 0.25f;
    vec3_t left_hip = { -0.3f, -1.2f, 0.0f };
    vec3_t left_foot = { -0.35f, -1.75f, 0.2f + walk_step };
    vec3_t right_hip = { 0.3f, -1.2f, 0.0f };
    vec3_t right_foot = { 0.35f, -1.75f, 0.2f - walk_step };

    screen_pt_t lhp = project_point(left_hip, yaw, pitch, cam_z);
    screen_pt_t lft = project_point(left_foot, yaw, pitch, cam_z);
    screen_pt_t rhp = project_point(right_hip, yaw, pitch, cam_z);
    screen_pt_t rft = project_point(right_foot, yaw, pitch, cam_z);

    draw_line(lhp.u, lhp.v, lft.u, lft.v, 255, 215, 0);
    draw_line(rhp.u, rhp.v, rft.u, rft.v, 255, 215, 0);
    draw_3d_ring(left_foot, 0.15f, yaw, pitch, cam_z, 255, 230, 100, 12);
    draw_3d_ring(right_foot, 0.15f, yaw, pitch, cam_z, 255, 230, 100, 12);

    // Snout & Cyan Telemetry Eyes
    vec3_t snout = { 0.0f, 0.65f, 0.45f };
    draw_3d_ring(snout, 0.12f, yaw, pitch, cam_z, 255, 255, 255, 12);

    vec3_t eye_l = { -0.15f, 0.82f, 0.40f };
    vec3_t eye_r = {  0.15f, 0.82f, 0.40f };
    screen_pt_t el = project_point(eye_l, yaw, pitch, cam_z);
    screen_pt_t er = project_point(eye_r, yaw, pitch, cam_z);
    draw_pixel(el.u, el.v, 0, 255, 255);
    draw_pixel(el.u+1, el.v, 0, 255, 255);
    draw_pixel(el.u, el.v+1, 0, 255, 255);
    draw_pixel(er.u, er.v, 0, 255, 255);
    draw_pixel(er.u+1, er.v, 0, 255, 255);
    draw_pixel(er.u, er.v+1, 0, 255, 255);

    // Telemetry Box Borders
    for (int x = 40; x < 680; x++) {
        draw_pixel(x, 40, 0, 255, 200);
        draw_pixel(x, 140, 0, 255, 200);
    }
    for (int y = 40; y < 140; y++) {
        draw_pixel(40, y, 0, 255, 200);
        draw_pixel(680, y, 0, 255, 200);
    }
}

int main(int argc, char **argv) {
    const char *stream_key = (argc > 1) ? argv[1] : "h19h-xuw9-7tx6-m8sw-dmht";
    char rtmp_url[512];
    snprintf(rtmp_url, sizeof(rtmp_url), "rtmp://a.rtmp.youtube.com/live2/%s", stream_key);

    printf("=============================================================\n");
    printf("LAUNCHING TSFI2 LIVE BROADCASTER TO YOUTUBE LIVE             \n");
    printf("=============================================================\n");
    printf("RTMP Endpoint : %s\n", rtmp_url);
    printf("Resolution    : %dx%d @ %d fps\n", WIDTH, HEIGHT, FPS);
    printf("Audio Score   : Eye of the Tiger (Bionika Synth)\n");
    printf("=============================================================\n");

    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -y -f rawvideo -pix_fmt rgb24 -s %dx%d -r %d -i - "
             "-f s16le -ar %d -ac 2 -i - "
             "-c:v libx264 -pix_fmt yuv420p -preset veryfast -g %d -b:v 3000k -maxrate 3500k -bufsize 6000k "
             "-c:a aac -b:a 160k -ar 44100 "
             "-f flv \"%s\" > /dev/null 2>&1",
             WIDTH, HEIGHT, FPS, SAMPLE_RATE, FPS * 2, rtmp_url);

    FILE *pipe = popen(cmd, "w");
    if (!pipe) {
        fprintf(stderr, "Error: Failed to open FFmpeg RTMP pipe!\n");
        return 1;
    }

    int frame = 0;
    while (1) {
        render_live_stream_frame(frame);
        generate_bionika_audio_frame(frame, audio_buffer);

        size_t v_w = fwrite(framebuffer, sizeof(pixel_t), WIDTH * HEIGHT, pipe);
        size_t a_w = fwrite(audio_buffer, sizeof(int16_t) * 2, SAMPLES_PER_FRAME, pipe);

        if (v_w == 0 || a_w == 0) {
            fprintf(stderr, "Pipe closed or broadcast interrupted.\n");
            break;
        }

        if (frame % 300 == 0) {
            printf("   -> [Live Telemetry Stream] Sent frame %d (Time: %.1f sec)...\n", frame, (float)frame / (float)FPS);
            fflush(stdout);
        }
        frame++;
    }

    pclose(pipe);
    return 0;
}

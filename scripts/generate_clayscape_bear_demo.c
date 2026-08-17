/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Kinematronic ClayScaped Bear Clan 90-Second Demo Generator
 * 
 * Renders an animated 3D ClayScape Bear Clan participant being assembled
 * layer-by-layer (clay extrusion slicing toolpaths, articulated monobone sockets,
 * ear arches, ceramic glazing shaders, and kinematic walk-and-wave) set to the
 * energetic modular Bionika synthesizer score of "Eye of the Tiger".
 *
 * Direct pipe to FFmpeg generates /home/mariarahel/src/tsfi2/atropa_pulsechain/clayscape_bear_demo_90s.mp4
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>

#define WIDTH 1280
#define HEIGHT 720
#define FPS 30
#define DURATION_SEC 90
#define TOTAL_FRAMES (FPS * DURATION_SEC)
#define SAMPLE_RATE 44100
#define SAMPLES_PER_FRAME (SAMPLE_RATE / FPS)

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    uint8_t r, g, b;
} pixel_t;

static pixel_t framebuffer[WIDTH * HEIGHT];
static int16_t audio_buffer[SAMPLES_PER_FRAME * 2]; // Stereo S16LE

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

/* 3D Wireframe / Mesh Transform & Projection */
typedef struct {
    float x, y, z;
} vec3_t;

typedef struct {
    int u, v;
    float depth;
} screen_pt_t;

static screen_pt_t project_point(vec3_t p, float yaw, float pitch, float roll, float cam_z, float scale) {
    screen_pt_t res;
    // Rotate Y
    float cy = cosf(yaw), sy = sinf(yaw);
    float x1 = p.x * cy + p.z * sy;
    float y1 = p.y;
    float z1 = -p.x * sy + p.z * cy;

    // Rotate X
    float cp = cosf(pitch), sp = sinf(pitch);
    float x2 = x1;
    float y2 = y1 * cp - z1 * sp;
    float z2 = y1 * sp + z1 * cp;

    // Rotate Z
    float cr = cosf(roll), sr = sinf(roll);
    float x3 = x2 * cr - y2 * sr;
    float y3 = x2 * sr + y2 * cr;
    float z3 = z2 + cam_z;

    if (z3 < 0.1f) z3 = 0.1f;
    float fov = 650.0f;
    res.u = (int)(WIDTH / 2.0f + (x3 * fov * scale) / z3);
    res.v = (int)(HEIGHT / 2.0f - (y3 * fov * scale) / z3);
    res.depth = z3;
    return res;
}

/* Draw a 3D Circle / Ring (Extrusion Layer) */
static void draw_3d_ring(vec3_t center, float radius, float yaw, float pitch, float cam_z, uint8_t r, uint8_t g, uint8_t b, int segments) {
    screen_pt_t prev;
    for (int i = 0; i <= segments; i++) {
        float angle = (float)i * (2.0f * (float)M_PI / (float)segments);
        vec3_t pt = {
            center.x + radius * cosf(angle),
            center.y,
            center.z + radius * sinf(angle)
        };
        screen_pt_t curr = project_point(pt, yaw, pitch, 0.0f, cam_z, 1.0f);
        if (i > 0) {
            draw_line(prev.u, prev.v, curr.u, curr.v, r, g, b);
        }
        prev = curr;
    }
}

/* Bionika Note Frequency Helper */
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

/* Audio Synthesis from Eye of the Tiger Arrangement */
static void generate_bionika_audio_frame(int frame, int16_t *stereo_out) {
    float bpm = 109.6f;
    float step_sec = (60.0f / bpm) / 4.0f; // 16th note step
    
    // 32-step Intro Riff pattern definitions from eye_of_the_tiger.bio
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

        // 1. Bass Synth (Pulse + Sub Saw)
        float b_freq = note_to_freq(bass_notes[step]);
        float b_env = expf(-step_progress * (bass_accents[step] ? 3.5f : 6.0f));
        float b_saw = 2.0f * fmodf(t * b_freq, 1.0f) - 1.0f;
        float b_pulse = (fmodf(t * b_freq * 0.5f, 1.0f) < 0.4f) ? 1.0f : -1.0f;
        float bass_sample = (0.6f * b_saw + 0.4f * b_pulse) * b_env * (bass_accents[step] ? 0.8f : 0.45f);

        // 2. Lead Guitar Synth (Overdrive + 5th Harmonic)
        float lead_sample = 0.0f;
        if (strlen(lead_notes[step]) > 0) {
            float l_freq = note_to_freq(lead_notes[step]);
            float l_env = expf(-step_progress * 4.0f);
            float l_saw = 2.0f * fmodf(t * l_freq, 1.0f) - 1.0f;
            float l_5th = sinf(2.0f * (float)M_PI * l_freq * 1.5f * t);
            float l_raw = 0.7f * l_saw + 0.3f * l_5th;
            lead_sample = tanhf(l_raw * 2.2f) * l_env * 0.65f;
        }

        // 3. Drums (Punchy 90s Kick + Snare + Hi-Hat)
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

        // Limiter / Clamping
        if (mix_left > 0.98f) mix_left = 0.98f;
        if (mix_left < -0.98f) mix_left = -0.98f;
        if (mix_right > 0.98f) mix_right = 0.98f;
        if (mix_right < -0.98f) mix_right = -0.98f;

        stereo_out[s * 2] = (int16_t)(mix_left * 32000.0f);
        stereo_out[s * 2 + 1] = (int16_t)(mix_right * 32000.0f);
    }
}

/* Render ClayScape Teddy Bear Frame */
static void render_clayscape_bear_frame(int frame) {
    float t = (float)frame / (float)FPS;
    clear_buffer(15, 18, 28); // Deep Cyber Ceramic Navy

    // Stage progression over 90 seconds:
    // 0..20s : Phase 1 - 3D Clay Slicing Toolpath Extrusion (Torso + Head Layer Slices)
    // 20..45s: Phase 2 - Articulated Ceramic Monobone Socket Assembly (Arms, Legs, Ears)
    // 45..70s: Phase 3 - Ceramic Glazing Shader & Surface Sintering (Gloss, Highlight Wireframe)
    // 70..90s: Phase 4 - Full Kinematronic Walk-and-Wave & Bear Clan Mastery Celebration

    float yaw = t * 0.35f;
    float pitch = 0.2f + 0.08f * sinf(t * 0.5f);
    float cam_z = 4.2f;

    // Grid Floor
    for (int gx = -5; gx <= 5; gx++) {
        vec3_t p1 = { (float)gx * 0.4f, -1.8f, -2.0f };
        vec3_t p2 = { (float)gx * 0.4f, -1.8f,  2.0f };
        screen_pt_t sp1 = project_point(p1, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t sp2 = project_point(p2, yaw, pitch, 0.0f, cam_z, 1.0f);
        draw_line(sp1.u, sp1.v, sp2.u, sp2.v, 30, 45, 70);
    }
    for (int gz = -5; gz <= 5; gz++) {
        vec3_t p1 = { -2.0f, -1.8f, (float)gz * 0.4f };
        vec3_t p2 = {  2.0f, -1.8f, (float)gz * 0.4f };
        screen_pt_t sp1 = project_point(p1, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t sp2 = project_point(p2, yaw, pitch, 0.0f, cam_z, 1.0f);
        draw_line(sp1.u, sp1.v, sp2.u, sp2.v, 30, 45, 70);
    }

    // Assembly Layer Heights
    int max_layers = 32;
    int visible_layers = (t < 20.0f) ? (int)((t / 20.0f) * max_layers) + 1 : max_layers;
    if (visible_layers > max_layers) visible_layers = max_layers;

    // 1. Torso Slices (Terracotta / Glazed Ceramic Gold)
    uint8_t clay_r = (t >= 45.0f) ? 230 : 210;
    uint8_t clay_g = (t >= 45.0f) ? 170 : 120;
    uint8_t clay_b = (t >= 45.0f) ? 60  : 70;

    for (int l = 0; l < visible_layers; l++) {
        float ly = -1.2f + ((float)l / (float)max_layers) * 1.2f;
        float r_torso = 0.55f * sqrtf(fmaxf(0.0f, 1.0f - powf((ly + 0.6f) / 0.7f, 2.0f)));
        vec3_t center = { 0.0f, ly, 0.0f };
        draw_3d_ring(center, r_torso, yaw, pitch, cam_z, clay_r, clay_g, clay_b, 24);
    }

    // 2. Head Slices (Starts at layer 16 onwards)
    if (t >= 8.0f) {
        int head_layers = (t < 20.0f) ? (int)(((t - 8.0f) / 12.0f) * 24) + 1 : 24;
        for (int l = 0; l < head_layers; l++) {
            float hy = 0.35f + ((float)l / 24.0f) * 0.85f;
            float r_head = 0.45f * sqrtf(fmaxf(0.0f, 1.0f - powf((hy - 0.75f) / 0.45f, 2.0f)));
            vec3_t center = { 0.0f, hy, 0.0f };
            draw_3d_ring(center, r_head, yaw, pitch, cam_z, clay_r + 20, clay_g + 20, clay_b + 10, 24);
        }
    }

    // 3. Ears (Phase 2: > 20s)
    if (t >= 20.0f) {
        float ear_progress = (t < 30.0f) ? (t - 20.0f) / 10.0f : 1.0f;
        float ear_r = 0.18f * ear_progress;
        vec3_t left_ear = { -0.38f * ear_progress, 1.15f, 0.0f };
        vec3_t right_ear = { 0.38f * ear_progress, 1.15f, 0.0f };
        draw_3d_ring(left_ear, ear_r, yaw, pitch, cam_z, 240, 190, 80, 16);
        draw_3d_ring(right_ear, ear_r, yaw, pitch, cam_z, 240, 190, 80, 16);
    }

    // 4. Arms & Kinematronic Wave (Phase 2 & 4)
    if (t >= 25.0f) {
        float wave_angle = (t >= 70.0f) ? sinf(t * 8.0f) * 0.5f + 0.3f : 0.0f;
        vec3_t left_shoulder = { -0.55f, 0.0f, 0.0f };
        vec3_t left_hand = { -0.85f, -0.6f + wave_angle * 0.8f, 0.3f + wave_angle * 0.4f };
        vec3_t right_shoulder = { 0.55f, 0.0f, 0.0f };
        vec3_t right_hand = { 0.85f, -0.6f, 0.0f };

        screen_pt_t ls = project_point(left_shoulder, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t lh = project_point(left_hand, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t rs = project_point(right_shoulder, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t rh = project_point(right_hand, yaw, pitch, 0.0f, cam_z, 1.0f);

        draw_line(ls.u, ls.v, lh.u, lh.v, 255, 215, 0);
        draw_line(rs.u, rs.v, rh.u, rh.v, 255, 215, 0);

        draw_3d_ring(left_hand, 0.12f, yaw, pitch, cam_z, 255, 230, 100, 12);
        draw_3d_ring(right_hand, 0.12f, yaw, pitch, cam_z, 255, 230, 100, 12);
    }

    // 5. Legs & Feet
    if (t >= 32.0f) {
        float walk_step = (t >= 70.0f) ? sinf(t * 6.0f) * 0.25f : 0.0f;
        vec3_t left_hip = { -0.3f, -1.2f, 0.0f };
        vec3_t left_foot = { -0.35f, -1.75f, 0.2f + walk_step };
        vec3_t right_hip = { 0.3f, -1.2f, 0.0f };
        vec3_t right_foot = { 0.35f, -1.75f, 0.2f - walk_step };

        screen_pt_t lhp = project_point(left_hip, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t lft = project_point(left_foot, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t rhp = project_point(right_hip, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t rft = project_point(right_foot, yaw, pitch, 0.0f, cam_z, 1.0f);

        draw_line(lhp.u, lhp.v, lft.u, lft.v, 255, 215, 0);
        draw_line(rhp.u, rhp.v, rft.u, rft.v, 255, 215, 0);

        draw_3d_ring(left_foot, 0.15f, yaw, pitch, cam_z, 255, 230, 100, 12);
        draw_3d_ring(right_foot, 0.15f, yaw, pitch, cam_z, 255, 230, 100, 12);
    }

    // 6. Bear Snout & Eyes
    if (t >= 15.0f) {
        vec3_t snout = { 0.0f, 0.65f, 0.45f };
        draw_3d_ring(snout, 0.12f, yaw, pitch, cam_z, 255, 255, 255, 12);

        vec3_t eye_l = { -0.15f, 0.82f, 0.40f };
        vec3_t eye_r = {  0.15f, 0.82f, 0.40f };
        screen_pt_t el = project_point(eye_l, yaw, pitch, 0.0f, cam_z, 1.0f);
        screen_pt_t er = project_point(eye_r, yaw, pitch, 0.0f, cam_z, 1.0f);
        draw_pixel(el.u, el.v, 0, 255, 255);
        draw_pixel(el.u+1, el.v, 0, 255, 255);
        draw_pixel(el.u, el.v+1, 0, 255, 255);
        draw_pixel(er.u, er.v, 0, 255, 255);
        draw_pixel(er.u+1, er.v, 0, 255, 255);
        draw_pixel(er.u, er.v+1, 0, 255, 255);
    }

    // On-screen Telemetry Banner
    // Simple ASCII Box Overlay
    for (int x = 40; x < 600; x++) {
        draw_pixel(x, 40, 0, 255, 200);
        draw_pixel(x, 140, 0, 255, 200);
    }
    for (int y = 40; y < 140; y++) {
        draw_pixel(40, y, 0, 255, 200);
        draw_pixel(600, y, 0, 255, 200);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("GENERATING 90-SECOND KINEMATRONIC CLAYSCAPE BEAR DEMO MP4    \n");
    printf("=============================================================\n");
    printf("Total Frames  : %d frames @ %d fps\n", TOTAL_FRAMES, FPS);
    printf("Audio Score   : Eye of the Tiger (Bionika Synth 109.6 BPM)\n");
    printf("Target File   : clayscape_bear_demo_90s.mp4\n");
    printf("=============================================================\n");

    const char *cmd = "ffmpeg -y -f rawvideo -pix_fmt rgb24 -s 1280x720 -r 30 -i - "
                      "-f s16le -ar 44100 -ac 2 -i - "
                      "-c:v libx264 -pix_fmt yuv420p -preset fast -crf 20 "
                      "-c:a aac -b:a 192k clayscape_bear_demo_90s.mp4 > /dev/null 2>&1";

    FILE *pipe = popen(cmd, "w");
    if (!pipe) {
        fprintf(stderr, "Error: Failed to open FFmpeg pipe!\n");
        return 1;
    }

    for (int f = 0; f < TOTAL_FRAMES; f++) {
        render_clayscape_bear_frame(f);
        generate_bionika_audio_frame(f, audio_buffer);

        fwrite(framebuffer, sizeof(pixel_t), WIDTH * HEIGHT, pipe);
        fwrite(audio_buffer, sizeof(int16_t) * 2, SAMPLES_PER_FRAME, pipe);

        if (f % 300 == 0) {
            printf("   -> Rendered %d/%d frames (%.1f%% complete, Time: %.1f sec)...\n",
                   f, TOTAL_FRAMES, (float)f / (float)TOTAL_FRAMES * 100.0f, (float)f / 30.0f);
            fflush(stdout);
        }
    }

    pclose(pipe);
    printf("=============================================================\n");
    printf("✓ DEMO GENERATION COMPLETE: clayscape_bear_demo_90s.mp4      \n");
    printf("=============================================================\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "auncient_teddy_bear_personality.h"
#include "tsfi_displacementshader.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define FPS 30
#define WIDTH 640
#define HEIGHT 360

static const char* intro_lead[32] = {
    "REST", "REST", "REST", "REST", "REST", "REST", "REST", "REST",
    "REST", "REST", "REST", "REST", "REST", "REST", "REST", "REST",
    "Bb3", "REST", "C4", "D4", "REST", "C4", "Bb3", "C4",
    "REST", "REST", "REST", "REST", "REST", "REST", "REST", "REST"
};
static const char* verse_lead[32] = {
    "Bb3", "REST", "C4", "D4", "REST", "C4", "Bb3", "C4",
    "REST", "REST", "REST", "REST", "REST", "REST", "REST", "REST",
    "A3", "REST", "Bb3", "C4", "REST", "Bb3", "A3", "Bb3",
    "REST", "REST", "REST", "REST", "REST", "REST", "REST", "REST"
};
static const char* chorus_lead[32] = {
    "F4", "REST", "REST", "D4", "Eb4", "D4", "C4", "Bb3",
    "G3", "REST", "REST", "REST", "REST", "REST", "REST", "REST",
    "Eb4", "REST", "REST", "C4", "D4", "C4", "Bb3", "A3",
    "F3", "REST", "REST", "REST", "REST", "REST", "REST", "REST"
};

static const char* bass_seq[32] = {
    "Bb1", "REST", "Bb1", "F1", "Bb1", "REST", "D2", "Eb2",
    "Eb2", "REST", "Eb2", "Bb1", "Eb2", "REST", "F2", "Bb1",
    "Cm2", "REST", "Cm2", "G1", "Cm2", "REST", "Eb2", "F2",
    "F2", "REST", "F2", "C2", "F2", "REST", "F2", "F1"
};

static const int kick_seq[32] = {
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
    1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0
};
static const int snare_seq[32] = {
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
};

typedef struct {
    char name[32];
    double angle;
    uint8_t color[3];
    int x, y;
    teddy_bear_geometry_t geom;
} bear_node_t;

static double note_to_freq(const char* note) {
    if (strcmp(note, "REST") == 0) return 0.0;
    char name[8];
    int octave = 4;
    if (strlen(note) == 3) {
        sprintf(name, "%c%c", note[0], note[1]);
        octave = note[2] - '0';
    } else {
        sprintf(name, "%c", note[0]);
        octave = note[1] - '0';
    }
    const char* names[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    int semis = -1;
    for (int i = 0; i < 12; i++) {
        if (strcmp(name, names[i]) == 0) {
            semis = i;
            break;
        }
    }
    if (semis == -1) return 0.0;
    int midi = 12 * (octave + 1) + semis;
    return 440.0 * pow(2.0, (midi - 69) / 12.0);
}

static void draw_line(uint8_t* pixels, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, int alpha) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
            int idx = (y0 * WIDTH + x0) * 3;
            pixels[idx] = (pixels[idx] * (255 - alpha) + r * alpha) / 255;
            pixels[idx+1] = (pixels[idx+1] * (255 - alpha) + g * alpha) / 255;
            pixels[idx+2] = (pixels[idx+2] * (255 - alpha) + b * alpha) / 255;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

static void draw_dashed_line(uint8_t* pixels, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b, int alpha, int dash_len) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    int count = 0;
    while (1) {
        if ((count / dash_len) % 2 == 0) {
            if (x0 >= 0 && x0 < WIDTH && y0 >= 0 && y0 < HEIGHT) {
                int idx = (y0 * WIDTH + x0) * 3;
                pixels[idx] = (pixels[idx] * (255 - alpha) + r * alpha) / 255;
                pixels[idx+1] = (pixels[idx+1] * (255 - alpha) + g * alpha) / 255;
                pixels[idx+2] = (pixels[idx+2] * (255 - alpha) + b * alpha) / 255;
            }
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
        count++;
    }
}

static void draw_ellipse(uint8_t* pixels, int cx, int cy, int rx, int ry, uint8_t r, uint8_t g, uint8_t b, int alpha, bool fill, uint8_t outline_r, uint8_t outline_g, uint8_t outline_b) {
    for (int y = cy - ry; y <= cy + ry; y++) {
        for (int x = cx - rx; x <= cx + rx; x++) {
            if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                double dx = (double)(x - cx) / rx;
                double dy = (double)(y - cy) / ry;
                double dist = dx*dx + dy*dy;
                if (fill && dist <= 1.0) {
                    int idx = (y * WIDTH + x) * 3;
                    pixels[idx] = (pixels[idx] * (255 - alpha) + r * alpha) / 255;
                    pixels[idx+1] = (pixels[idx+1] * (255 - alpha) + g * alpha) / 255;
                    pixels[idx+2] = (pixels[idx+2] * (255 - alpha) + b * alpha) / 255;
                }
                if (!fill && dist >= 0.9 && dist <= 1.1) {
                    int idx = (y * WIDTH + x) * 3;
                    pixels[idx] = outline_r;
                    pixels[idx+1] = outline_g;
                    pixels[idx+2] = outline_b;
                }
            }
        }
    }
}

int main() {
    printf("[C Engine] Initializing DSP synthesizer and frame renderer...\n");
    
    // WinchesterMQ or wm rule linkage: initialize DisplacementShader instance
    TSFiDisplacementShader ds_wmq;
    tsfi_displacementshader_init(&ds_wmq, 1.0, 1.0);
    
    bear_node_t bears[5];
    memset(bears, 0, sizeof(bears));
    
    strcpy(bears[0].name, "Trusty");
    bears[0].angle = 0.0 * M_PI;
    bears[0].color[0] = 50; bears[0].color[1] = 160; bears[0].color[2] = 255;
    
    strcpy(bears[1].name, "Aggro");
    bears[1].angle = 0.4 * M_PI;
    bears[1].color[0] = 255; bears[1].color[1] = 30; bears[1].color[2] = 80;
    
    strcpy(bears[2].name, "Skeptic");
    bears[2].angle = 0.8 * M_PI;
    bears[2].color[0] = 255; bears[2].color[1] = 210; bears[2].color[2] = 10;
    
    strcpy(bears[3].name, "Eerie");
    bears[3].angle = 1.2 * M_PI;
    bears[3].color[0] = 180; bears[3].color[1] = 40; bears[3].color[2] = 255;
    
    strcpy(bears[4].name, "Coop");
    bears[4].angle = 1.6 * M_PI;
    bears[4].color[0] = 30; bears[4].color[1] = 220; bears[4].color[2] = 100;
    
    resolve_teddy_bear_geometry(PERSONALITY_TRUSTWORTHY, &bears[0].geom);
    resolve_teddy_bear_geometry(PERSONALITY_AGGRESSIVE, &bears[1].geom);
    resolve_teddy_bear_geometry(PERSONALITY_SKEPTICAL, &bears[2].geom);
    resolve_teddy_bear_geometry(PERSONALITY_EERIE, &bears[3].geom);
    resolve_teddy_bear_geometry(PERSONALITY_TRUSTWORTHY, &bears[4].geom);
    bears[4].geom.symmetry = 0.95;
    
    int cX = WIDTH / 2, cY = HEIGHT / 2, rad = 140;
    for (int i = 0; i < 5; i++) {
        bears[i].x = cX + (int)(rad * cos(bears[i].angle));
        bears[i].y = cY + (int)(rad * sin(bears[i].angle));
    }
    
    const int num_patterns = 12;
    const double seconds_per_pattern = 6.0;
    const double duration = num_patterns * seconds_per_pattern;
    const int total_samples = (int)(duration * SAMPLE_RATE);
    const int total_frames = (int)(duration * FPS);
    
    float* audio_data = (float*)calloc(total_samples * 2, sizeof(float));
    assert(audio_data != NULL);
    
    printf("[C Engine] Synthesizing audio tracks...\n");
    double phase_l = 0.0, phase_b = 0.0;
    
    for (int p = 0; p < num_patterns; p++) {
        const char** lead_pat = (p % 4 == 0) ? intro_lead : ((p % 2 == 0) ? verse_lead : chorus_lead);
        double p_start_sec = p * seconds_per_pattern;
        
        for (int step = 0; step < 32; step++) {
            double step_start_sec = p_start_sec + step * (seconds_per_pattern / 32.0);
            int start_idx = (int)(step_start_sec * SAMPLE_RATE);
            int end_idx = (int)((step_start_sec + (seconds_per_pattern / 32.0)) * SAMPLE_RATE);
            
            const char* lead_note = lead_pat[step];
            const char* bass_note = bass_seq[step];
            int kick = kick_seq[step];
            int snare = snare_seq[step];
            
            double f_lead = note_to_freq(lead_note);
            double f_bass = note_to_freq(bass_note);
            
            for (int s = start_idx; s < end_idx && s < total_samples; s++) {
                double t_local = (double)(s - start_idx) / SAMPLE_RATE;
                double env = 1.0;
                if (t_local < 0.01) env = t_local / 0.01;
                if ((end_idx - s) / (double)SAMPLE_RATE < 0.01) env = ((end_idx - s) / (double)SAMPLE_RATE) / 0.01;
                
                if (f_lead > 0.0) {
                    phase_l += 2.0 * M_PI * f_lead / SAMPLE_RATE;
                    double wave_val = sin(phase_l) + 0.3 * sin(3.0 * phase_l);
                    double clipped = tanh(wave_val * 1.5) * 0.15 * env;
                    audio_data[s * 2] += clipped;
                    audio_data[s * 2 + 1] += clipped;
                }
                if (f_bass > 0.0) {
                    phase_b += 2.0 * M_PI * f_bass / SAMPLE_RATE;
                    double bass_val = sin(phase_b) * 0.25 * env;
                    audio_data[s * 2] += bass_val;
                    audio_data[s * 2 + 1] += bass_val;
                }
                if (kick && t_local < 0.15) {
                    double sweep = 120.0 * exp(-t_local * 40.0) + 40.0;
                    double k_val = sin(2.0 * M_PI * sweep * t_local) * 0.4 * exp(-t_local * 8.0);
                    audio_data[s * 2] += k_val;
                    audio_data[s * 2 + 1] += k_val;
                }
                if (snare && t_local < 0.2) {
                    double noise_val = (((double)rand() / RAND_MAX) * 2.0 - 1.0) * 0.18 * exp(-t_local * 12.0);
                    audio_data[s * 2] += noise_val;
                    audio_data[s * 2 + 1] += noise_val;
                }
            }
        }
    }
    
    float max_audio = 0.0f;
    for (int i = 0; i < total_samples * 2; i++) {
        if (fabs(audio_data[i]) > max_audio) max_audio = fabs(audio_data[i]);
    }
    if (max_audio > 0.0f) {
        for (int i = 0; i < total_samples * 2; i++) audio_data[i] = (audio_data[i] / max_audio) * 0.9f;
    }
    
    FILE* wf_out = fopen("bear_chorus_track.wav", "wb");
    assert(wf_out != NULL);
    uint32_t chunk_size = 36 + total_samples * 4;
    uint32_t subchunk1_size = 16;
    uint16_t audio_fmt = 1;
    uint16_t num_chans = 2;
    uint32_t s_rate = SAMPLE_RATE;
    uint32_t byte_rate = SAMPLE_RATE * 4;
    uint16_t block_align = 4;
    uint16_t bits_per_sample = 16;
    uint32_t subchunk2_size = total_samples * 4;
    
    fwrite("RIFF", 1, 4, wf_out);
    fwrite(&chunk_size, 4, 1, wf_out);
    fwrite("WAVE", 1, 4, wf_out);
    fwrite("fmt ", 1, 4, wf_out);
    fwrite(&subchunk1_size, 4, 1, wf_out);
    fwrite(&audio_fmt, 2, 1, wf_out);
    fwrite(&num_chans, 2, 1, wf_out);
    fwrite(&s_rate, 4, 1, wf_out);
    fwrite(&byte_rate, 4, 1, wf_out);
    fwrite(&block_align, 2, 1, wf_out);
    fwrite(&bits_per_sample, 2, 1, wf_out);
    fwrite("data", 1, 4, wf_out);
    fwrite(&subchunk2_size, 4, 1, wf_out);
    
    for (int i = 0; i < total_samples * 2; i++) {
        int16_t pcm = (int16_t)(audio_data[i] * 32767.0f);
        fwrite(&pcm, 2, 1, wf_out);
    }
    fclose(wf_out);
    
    printf("[C Engine] Rendering frames to raw PPMs...\n");
    uint8_t* pixels = (uint8_t*)malloc(WIDTH * HEIGHT * 3);
    assert(pixels != NULL);
    system("mkdir -p rendered_frames");
    
    double prev_pitch = 220.0;
    
    for (int f = 0; f < total_frames; f++) {
        memset(pixels, 6, WIDTH * HEIGHT * 3);
        
        double t_sec = (double)f / FPS;
        int pat_idx = (int)(t_sec / seconds_per_pattern);
        if (pat_idx >= num_patterns) pat_idx = num_patterns - 1;
        const char** lead_pat = (pat_idx % 4 == 0) ? intro_lead : ((pat_idx % 2 == 0) ? verse_lead : chorus_lead);
        
        double local_sec = fmod(t_sec, seconds_per_pattern);
        int step = (int)(local_sec / (seconds_per_pattern / 32.0));
        if (step > 31) step = 31;
        
        const char* note = lead_pat[step];
        int active_speaker_idx = (pat_idx + step / 8) % 5;
        
        for (int x = 0; x < WIDTH; x += 40) {
            for (int y = 0; y < HEIGHT; y++) {
                int idx = (y * WIDTH + x) * 3;
                pixels[idx] = 16; pixels[idx+1] = 16; pixels[idx+2] = 24;
            }
        }
        for (int y = 0; y < HEIGHT; y += 40) {
            for (int x = 0; x < WIDTH; x++) {
                int idx = (y * WIDTH + x) * 3;
                pixels[idx] = 16; pixels[idx+1] = 16; pixels[idx+2] = 24;
            }
        }
        
        draw_ellipse(pixels, cX, cY, 55, 55, 251, 191, 36, 60, false, 251, 191, 36);
        
        for (int i = 0; i < 5; i++) {
            bear_node_t* b = &bears[i];
            
            int sync_frame_offset = (int)(f - (evaluate_fw_threat_level(&b->geom) * 1.5));
            if (sync_frame_offset < 0) sync_frame_offset = 0;
            
            double sync_t_sec = (double)sync_frame_offset / FPS;
            int sync_pat_idx = (int)(sync_t_sec / seconds_per_pattern);
            if (sync_pat_idx >= num_patterns) sync_pat_idx = num_patterns - 1;
            const char** sync_lead_pat = (sync_pat_idx % 4 == 0) ? intro_lead : ((sync_pat_idx % 2 == 0) ? verse_lead : chorus_lead);
            
            double sync_local_sec = fmod(sync_t_sec, seconds_per_pattern);
            int sync_step = (int)(sync_local_sec / (seconds_per_pattern / 32.0));
            if (sync_step > 31) sync_step = 31;
            const char* sync_note = sync_lead_pat[sync_step];
            
            bool is_speaker = (i == active_speaker_idx && strcmp(sync_note, "REST") != 0);
            
            int bx = b->x, by = b->y;
            double f_hz = note_to_freq(note);
            
            double threat = evaluate_fw_threat_level(&b->geom);
            
            double transition_velocity = 0.0;
            if (is_speaker && f_hz > 0.0 && prev_pitch > 0.0) {
                transition_velocity = fabs(f_hz - prev_pitch);
                if (transition_velocity > 30.0) transition_velocity = 30.0;
            }
            
            bool is_frozen = false;
            double decayed_uncanny = 0.0;
            evaluate_freeze_habituation_decay(&b->geom, transition_velocity, &decayed_uncanny);
            
            int cloglog_scale_rating = evaluate_ordinal_cloglog_scale_rating(&b->geom, 1.0 + transition_velocity * 0.04);
            int blink_interval = 6 + (cloglog_scale_rating % 3);
            if (is_speaker && (f % blink_interval < 2)) {
                is_frozen = true;
            }
            
            if (is_speaker && !is_frozen) {
                int scale_rating = evaluate_ordinal_scale_rating(&b->geom, 1.0 + transition_velocity * 0.05);
                double covariance_stiffness = 1.0 + (scale_rating * 0.2) + (cloglog_scale_rating * 0.05);
                bx += (int)((6.0 / covariance_stiffness) * sin(f * 0.3 * covariance_stiffness));
                by += (int)((-8.0 / covariance_stiffness) * fabs(cos(f * 0.25 * covariance_stiffness)));
            }
            
            uint8_t alpha = is_speaker ? 240 : 180;
            
            double cheating_risk = 0.0;
            evaluate_cooperative_cheating_risk(&b->geom, transition_velocity * 0.1, &cheating_risk);
            
            double retaliation = 0.0;
            evaluate_reactive_retaliation_profile(&b->geom, transition_velocity, &retaliation);
            
            uint8_t halo_r = (uint8_t)(retaliation * 25.5);
            uint8_t halo_g = (uint8_t)(255 * (1.0 - retaliation * 0.1));
            uint8_t halo_b = (uint8_t)(200 * (1.0 - retaliation * 0.1));
            
            int halo_rad = 30 + (int)(retaliation * 5.0 * sin(f * 0.2));
            if (is_speaker && !is_frozen) {
                draw_ellipse(pixels, bx, by, halo_rad, halo_rad, halo_r, halo_g, halo_b, 40, false, halo_r, halo_g, halo_b);
            }
            
            int head_rx = (int)(14 * (1.0 + threat * 0.15));
            int head_ry = 12;
            
            int mixed_rating = 0;
            evaluate_ordinal_flexible_mixture_link(&b->geom, 0.5, &mixed_rating);
            int skew_offset = (int)((mixed_rating - 4) * 1.5);
            
            double exposure_consistency = 0.0;
            evaluate_exposure_threat_consistency(&b->geom, f * 33.3, &exposure_consistency);
            
            int eye_r = (int)(3 + exposure_consistency * 1.5);
            
            double cov_matrix[25] = {0.0};
            evaluate_parameter_covariance(&b->geom, cov_matrix);
            
            double jitter_amplitude = 0.0;
            evaluate_expression_jitter_uncanny(&b->geom, transition_velocity, &jitter_amplitude);
            
            double jitter_freq = 0.7 + (jitter_amplitude * 2.0);
            int jx = (int)(jitter_amplitude * 3.0 * sin(f * jitter_freq));
            int jy = (int)(jitter_amplitude * 3.0 * cos(f * (jitter_freq + 0.1)));
            
            double wald_stat = 0.0, wald_p = 1.0;
            double theta_val[3] = {0.1, 0.5, 0.9};
            double cov_val[9] = {0.01, 0.0, 0.0, 0.0, 0.01, 0.0, 0.0, 0.0, 0.01};
            evaluate_ordinal_mixture_threshold_wald_test(theta_val, cov_val, 0.5, 2, &wald_stat, &wald_p);
            
            uint8_t eye_color_r = 0;
            uint8_t eye_color_g = 0;
            uint8_t eye_color_b = 0;
            if (wald_p < 0.05) {
                eye_r = 2;
                eye_color_r = 240;
            }
            
            int hx = bx + skew_offset;
            int hy = by - 6;
            
            int eye_skew = (int)(threat * 2.0);
            
            double dominance_mult = 1.0;
            evaluate_status_dominance_multiplier(&b->geom, transition_velocity, &dominance_mult);
            int cheek_skew = (int)((dominance_mult - 1.0) * 3.0);
            
            int eye_ry = eye_r;
            if (is_frozen) {
                eye_ry = (int)(eye_r * 0.25);
            }
            if (decayed_uncanny > 0.4) {
                eye_ry = (int)(eye_ry * (1.0 - decayed_uncanny * 0.5));
            }
            
            int ear_ry = 6;
            if (retaliation > 0.4) {
                ear_ry = (int)(6.0 * (1.0 - retaliation * 0.5));
            }
            
            int snout_r = 2;
            if (cloglog_scale_rating > 4) {
                snout_r = (int)(2.0 * (1.0 + (cloglog_scale_rating - 4) * 0.2));
            }
            
            double cheating_threat = 0.0;
            evaluate_cheating_threat_index(&b->geom, transition_velocity, &cheating_threat);
            int snout_jx = 0, snout_jy = 0;
            if (cheating_threat > 0.3) {
                snout_jx = (int)(cheating_threat * 4.0 * sin(f * 1.5));
                snout_jy = (int)(cheating_threat * 4.0 * cos(f * 1.3));
            }
            
            draw_ellipse(pixels, bx, by + 12, 18, 14, b->color[0], b->color[1], b->color[2], alpha, true, 0, 0, 0);
            draw_ellipse(pixels, hx, hy, head_rx, head_ry, b->color[0], b->color[1], b->color[2], alpha, true, 0, 0, 0);
            draw_ellipse(pixels, hx - 14 - cheek_skew, hy - 10, 6, ear_ry, b->color[0], b->color[1], b->color[2], alpha, true, 0, 0, 0);
            draw_ellipse(pixels, hx + 14 + cheek_skew, hy - 10, 6, ear_ry, b->color[0], b->color[1], b->color[2], alpha, true, 0, 0, 0);
            
            draw_ellipse(pixels, hx - 6, hy - 4, 3, 3, 255, 255, 255, 120, true, 0, 0, 0);
            
            draw_ellipse(pixels, hx - 5 + jx + eye_skew, hy + 2 + jy, eye_r, eye_ry, eye_color_r, eye_color_g, eye_color_b, 255, true, 0, 0, 0);
            draw_ellipse(pixels, hx + 5 + jx - eye_skew, hy + 2 + jy, eye_r, eye_ry, eye_color_r, eye_color_g, eye_color_b, 255, true, 0, 0, 0);
            draw_ellipse(pixels, hx + jx + snout_jx, hy + 6 + jy + snout_jy, snout_r, snout_r, 30, 30, 30, 255, true, 0, 0, 0);
            
            if (is_speaker) {
                double dominance_multiplier = 1.0;
                evaluate_status_dominance_multiplier(&b->geom, transition_velocity, &dominance_multiplier);
                int connection_alpha = (int)(140 * (1.0 - cheating_risk));
                if (connection_alpha < 20) connection_alpha = 20;
                
                int target_x = cX + (int)((bx - cX) * (1.0 - dominance_multiplier * 0.15));
                int target_y = cY + (int)((by - cY) * (1.0 - dominance_multiplier * 0.15));
                
                if (cheating_threat > 0.4) {
                    draw_dashed_line(pixels, bx, by, target_x, target_y, 16, 185, 129, connection_alpha, 4);
                } else {
                    draw_line(pixels, bx, by, target_x, target_y, 16, 185, 129, connection_alpha);
                }
                
                if (f_hz > 0.0) {
                    double aggression_ceiling = 1.0;
                    evaluate_retaliation_aggression_ceiling(&b->geom, transition_velocity, &aggression_ceiling);
                    double capped_scale = 45.0 * aggression_ceiling;
                    
                    int prev_lx = 0, prev_ly = 0;
                    for (int pt = 0; pt < 100; pt++) {
                        double theta_t = (pt / 100.0) * 2.0 * M_PI;
                        
                        // Apply DisplacementShader validation mapping WinchesterMQ boundaries
                        double wmq_time = f * 0.04;
                        double dx = tsfi_displacementshader_eval(&ds_wmq, wmq_time, theta_t);
                        
                        int lx = cX + (int)(capped_scale * sin(theta_t * (f_hz / 10.0) + dx));
                        int ly = cY + (int)(35.0 * cos(theta_t * 15.0));
                        if (pt > 0) {
                            draw_line(pixels, prev_lx, prev_ly, lx, ly, 251, 191, 36, 180);
                        }
                        prev_lx = lx; prev_ly = ly;
                    }
                    prev_pitch = f_hz;
                }
            }
        }
        
        char frame_path[256];
        sprintf(frame_path, "rendered_frames/frame_%04d.ppm", f);
        FILE* img_out = fopen(frame_path, "wb");
        assert(img_out != NULL);
        fprintf(img_out, "P6\n%d %d\n255\n", WIDTH, HEIGHT);
        fwrite(pixels, 1, WIDTH * HEIGHT * 3, img_out);
        fclose(img_out);
    }
    
    printf("[C Engine] Rendering pipeline completed successfully!\n");
    free(audio_data);
    free(pixels);
    return 0;
}

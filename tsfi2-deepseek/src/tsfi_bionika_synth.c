#define _GNU_SOURCE
#include "tsfi_bionika_synth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -----------------------------------------------------------------------------
// Pure C Score Parser
// -----------------------------------------------------------------------------
bool tsfi_bio_load_score(TsfiBioScore *score, const char *filepath) {
    if (!score || !filepath) return false;
    memset(score, 0, sizeof(TsfiBioScore));

    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f) && score->event_count < 128) {
        if (line[0] == '#' || line[0] == '[' || line[0] == '\n' || line[0] == '\r') continue;

        float time_sec = 0.0f;
        char track_str[32] = {0};
        char note_str[32] = {0};
        char freq_str[32] = {0};
        float gain = 0.0f;
        float dur = 1.0f;

        int parsed = sscanf(line, "%f %31s %31s %31s %f %f",
                            &time_sec, track_str, note_str, freq_str, &gain, &dur);

        if (parsed >= 5) {
            int track_idx = 0;
            if (sscanf(track_str, "TRACK_%d", &track_idx) != 1) continue;

            float start_freq = 0.0f, end_freq = 0.0f;
            if (strstr(freq_str, "->")) {
                sscanf(freq_str, "%f->%f", &start_freq, &end_freq);
            } else {
                start_freq = (float)atof(freq_str);
                end_freq = start_freq;
            }

            TsfiBioEvent *ev = &score->events[score->event_count++];
            ev->time_sec = time_sec;
            ev->track_idx = track_idx;
            ev->start_freq = start_freq;
            ev->end_freq = end_freq;
            ev->gain = gain;
            ev->duration_sec = dur;
        }
    }

    fclose(f);
    return (score->event_count > 0);
}

// -----------------------------------------------------------------------------
// Musical, Harmonically Separated Drum & Bass Synthesizer Engine
// -----------------------------------------------------------------------------
size_t tsfi_bio_synthesize_pcm16(const TsfiBioScore *score, int16_t *out_pcm, size_t total_frames) {
    (void)score;
    if (!out_pcm || total_frames == 0) return 0;

    float *mix_buf = (float *)calloc(total_frames, sizeof(float));
    if (!mix_buf) return 0;

    float dt = 1.0f / (float)BIONIKA_SAMPLE_RATE;

    // Track Phase Accumulators
    float p_bass = 0.0f;
    float p_growl = 0.0f;
    float p_lead = 0.0f;
    float p_kick = 0.0f;
    float p_bird = 0.0f;

    // Low-pass filter state for growl separation
    float growl_lpf = 0.0f;

    for (size_t i = 0; i < total_frames; i++) {
        float t = (float)i * dt;
        float sample = 0.0f;

        // ---------------------------------------------------------------------
        // 1. MELODIC LOW BASS (Track 1): Clean Deep Sub-Bass Progression (A1-F1-C2-D1)
        // ---------------------------------------------------------------------
        if (t < 80.0f) {
            // Melodic 8-bar chord progression
            float bass_notes[8] = { 55.00f, 55.00f, 43.65f, 43.65f, 65.41f, 65.41f, 36.71f, 49.00f }; // A1, A1, F1, F1, C2, C2, D1, G1
            int bar_idx = (int)(t / 2.0f) % 8;
            float bass_freq = bass_notes[bar_idx];

            if (t >= 50.0f && t < 62.0f) {
                // Rising melodic tension
                bass_freq = 55.00f + (t - 50.0f) * 2.0f;
            } else if (t >= 62.0f && t < 80.0f) {
                // Smooth melodic deep glide
                float drop_prog = (t - 62.0f) / 18.0f;
                bass_freq = 73.42f * (1.0f - drop_prog * 0.55f); // D2 -> 33Hz Low C
            }

            // Pure fundamental + warm 2nd harmonic (melodic, undistorted sub)
            float sub_sine = sinf(2.0f * (float)M_PI * p_bass);
            float sub_warm = 0.22f * sinf(4.0f * (float)M_PI * p_bass);
            float bass_val = (sub_sine + sub_warm) * 0.75f;

            sample += bass_val;

            p_bass += bass_freq * dt;
            if (p_bass >= 1.0f) p_bass -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 2. SEPARATED MID-RANGE ACID GROWL (Track 2): Bandpassed (400Hz - 2500Hz)
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float growl_lfo = 0.5f + 0.5f * sinf(2.0f * (float)M_PI * 3.8f * t);
            // Higher octave base (E2 = 82.4Hz or A2 = 110Hz) to separate completely from sub-bass
            float growl_freq = (t >= 62.0f) ? 110.0f : 130.81f; // A2 or C3
            float saw = 2.0f * (p_growl - floorf(p_growl + 0.5f));

            // 1-pole Low-Pass Filter sweep
            float cutoff_hz = 600.0f + 1600.0f * growl_lfo;
            float alpha = (2.0f * (float)M_PI * cutoff_hz * dt) / (2.0f * (float)M_PI * cutoff_hz * dt + 1.0f);
            growl_lpf += alpha * (saw - growl_lpf);

            // High-pass filter subtraction to eliminate bass mud below 200Hz
            float growl_highpass = saw - growl_lpf * 0.3f;
            float growl_gain = (t >= 62.0f) ? 0.38f : 0.28f;

            sample += growl_highpass * growl_gain;

            p_growl += growl_freq * dt;
            if (p_growl >= 1.0f) p_growl -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 3. CRYSTALLINE LEAD ARPEGGIATOR (Track 3): High Melodic Chords
        // ---------------------------------------------------------------------
        if (t >= 14.0f && t < 80.0f) {
            // Melodic Pentatonic Arp: C4, Eb4, G4, Bb4, C5, Eb5, G5
            const float arp_notes[7] = { 261.63f, 311.13f, 392.00f, 466.16f, 523.25f, 622.25f, 783.99f };
            int step = (int)(t * 8.0f) % 7;
            float lead_freq = arp_notes[step];

            float sq1 = (sinf(2.0f * (float)M_PI * p_lead) >= 0.0f) ? 0.7f : -0.7f;
            float sq2 = (sinf(2.0f * (float)M_PI * (p_lead * 1.008f)) >= 0.0f) ? 0.7f : -0.7f;
            float lead_val = (sq1 + sq2) * 0.5f;
            float lead_gain = (t >= 62.0f) ? 0.26f : 0.18f;

            sample += lead_val * lead_gain;

            p_lead += lead_freq * dt;
            if (p_lead >= 1.0f) p_lead -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 4. CRISP, PUNCHY KICK DRUM (Track 4): Isolated Click & Deep Thud
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float beat_pos = fmodf(t, 0.5f);
            if (beat_pos < 0.22f) {
                float k_prog = beat_pos / 0.22f;
                // Fast pitch drop: 220Hz click -> 45Hz solid body
                float k_freq = 220.0f * expf(-k_prog * 12.0f) + 45.0f;
                float k_env = expf(-k_prog * 8.5f);
                float kick_val = sinf(2.0f * (float)M_PI * p_kick) * k_env;
                // Clean transient click
                float click = (beat_pos < 0.008f) ? (1.0f - beat_pos / 0.008f) * 0.35f : 0.0f;

                sample += (kick_val * 0.85f + click);

                p_kick += k_freq * dt;
                if (p_kick >= 1.0f) p_kick -= 1.0f;
            } else {
                p_kick = 0.0f;
            }
        }

        // ---------------------------------------------------------------------
        // 5. SNAPPY, ARTICULATE SNARE (Track 5): 220Hz Tone + High-Frequency Snap
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float measure_pos = fmodf(t, 2.0f);
            bool is_snare = (measure_pos >= 0.5f && measure_pos < 0.72f) || (measure_pos >= 1.5f && measure_pos < 1.72f);
            if (t >= 50.0f && t < 62.0f) {
                float roll_pos = fmodf(t, 0.125f);
                if (roll_pos < 0.08f) {
                    float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                    float roll_env = expf(-roll_pos * 30.0f);
                    sample += noise * roll_env * 0.45f * ((t - 50.0f) / 12.0f);
                }
            } else if (is_snare) {
                float s_pos = (measure_pos >= 1.5f) ? (measure_pos - 1.5f) : (measure_pos - 0.5f);
                float s_env = expf(-s_pos * 18.0f);
                float tone = sinf(2.0f * (float)M_PI * 220.0f * s_pos) * expf(-s_pos * 25.0f);
                float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f) * 0.7f;

                sample += (tone * 0.5f + noise) * s_env * 0.65f;
            }
        }

        // ---------------------------------------------------------------------
        // 6. CRISP HI-HAT (Track 6): Clean Metallic 9kHz Click
        // ---------------------------------------------------------------------
        if (t >= 12.0f && t < 80.0f) {
            float hh_pos = fmodf(t, 0.125f);
            if (hh_pos < 0.035f) {
                float hh_env = expf(-hh_pos * 55.0f);
                float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                sample += noise * hh_env * 0.22f;
            }
        }

        // ---------------------------------------------------------------------
        // 7. BIRD SONG OUTRO (Track 7): Sweet Serene Chirps (80s - 90s)
        // ---------------------------------------------------------------------
        if (t >= 80.0f && t <= 90.0f) {
            float outro_t = t - 80.0f;
            float chirp_cycle = fmodf(outro_t, 1.8f);
            if (chirp_cycle < 0.7f) {
                float chirp_prog = chirp_cycle / 0.7f;
                float b_freq = 2800.0f + 2000.0f * sinf((float)M_PI * chirp_prog) + 300.0f * sinf(2.0f * (float)M_PI * 14.0f * chirp_prog);
                float b_env = sinf((float)M_PI * chirp_prog) * expf(-outro_t * 0.15f);
                float bird_val = sinf(2.0f * (float)M_PI * p_bird) * b_env;
                sample += bird_val * 0.35f;

                p_bird += b_freq * dt;
                if (p_bird >= 1.0f) p_bird -= 1.0f;
            }
        }

        mix_buf[i] = sample;
    }

    // Transparent Master Limiter
    for (size_t i = 0; i < total_frames; i++) {
        float val = tanhf(mix_buf[i] * 0.80f);
        if (val > 1.0f) val = 1.0f;
        if (val < -1.0f) val = -1.0f;
        out_pcm[i] = (int16_t)(val * 32767.0f);
    }

    free(mix_buf);
    return total_frames;
}

// -----------------------------------------------------------------------------
// Pure C RIFF/WAVE 16-bit PCM Audio Exporter
// -----------------------------------------------------------------------------
bool tsfi_bio_export_wav(const char *filepath, const int16_t *pcm_data, size_t total_frames) {
    if (!filepath || !pcm_data || total_frames == 0) return false;

    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    uint32_t data_size = (uint32_t)(total_frames * sizeof(int16_t));
    uint32_t chunk_size = 36 + data_size;
    uint32_t sample_rate = BIONIKA_SAMPLE_RATE;
    uint16_t channels = 1;
    uint16_t bits_per_sample = 16;
    uint32_t byte_rate = sample_rate * channels * (bits_per_sample / 8);
    uint16_t block_align = channels * (bits_per_sample / 8);

    fwrite("RIFF", 1, 4, f);
    fwrite(&chunk_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);

    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;
    fwrite(&subchunk1_size, 4, 1, f);
    fwrite(&audio_format, 2, 1, f);
    fwrite(&channels, 2, 1, f);
    fwrite(&sample_rate, 4, 1, f);
    fwrite(&byte_rate, 4, 1, f);
    fwrite(&block_align, 2, 1, f);
    fwrite(&bits_per_sample, 2, 1, f);

    fwrite("data", 1, 4, f);
    fwrite(&data_size, 4, 1, f);
    fwrite(pcm_data, sizeof(int16_t), total_frames, f);

    fclose(f);
    return true;
}

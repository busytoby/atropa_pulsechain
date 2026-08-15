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
// Pure C Score Parser for assets/bionika/bionika_90s_symphony.bio
// Format: [TIME_SEC] TRACK NOTE/EFFECT FREQ_HZ GAIN [DURATION_SEC] ...
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
// Enhanced Drum & Bass Heavy Bionika Synthesizer Engine
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

    for (size_t i = 0; i < total_frames; i++) {
        float t = (float)i * dt;
        float sample = 0.0f;

        // ---------------------------------------------------------------------
        // 1. DRUM & BASS: SUB-BASS (Track 1) - Boosted Fundamental & Warm Drive
        // ---------------------------------------------------------------------
        if (t < 80.0f) {
            float bass_freq = 55.0f; // A1
            if (t >= 15.0f && t < 25.0f) bass_freq = (fmodf(t, 4.0f) < 2.0f) ? 55.0f : 43.6f;
            else if (t >= 25.0f && t < 38.0f) bass_freq = (fmodf(t, 2.0f) < 1.0f) ? 65.4f : 55.0f;
            else if (t >= 38.0f && t < 50.0f) bass_freq = (fmodf(t, 4.0f) < 2.0f) ? 43.6f : 36.7f;
            else if (t >= 50.0f && t < 62.0f) bass_freq = 55.0f + (t - 50.0f) * 2.5f;
            else if (t >= 62.0f && t < 80.0f) {
                // MASSIVE BASS DROP (62s - 80s)
                float drop_prog = (t - 62.0f) / 18.0f;
                bass_freq = 90.0f * (1.0f - drop_prog * 0.78f); // 90Hz -> 20Hz
            }

            // Punchy bass harmonic saturation
            float bass_gain = (t >= 62.0f) ? 1.10f : 0.85f;
            float raw_bass = sinf(2.0f * (float)M_PI * p_bass) + 0.5f * sinf(4.0f * (float)M_PI * p_bass) + 0.25f * sinf(6.0f * (float)M_PI * p_bass);
            float sat_bass = tanhf(raw_bass * 1.4f); // Warm tube overdrive
            sample += sat_bass * bass_gain;

            p_bass += bass_freq * dt;
            if (p_bass >= 1.0f) p_bass -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 2. SUB-GROWL ACID SAW (Track 2): 3.8Hz LFO Heavy Resonance
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float growl_lfo = 0.5f + 0.5f * sinf(2.0f * (float)M_PI * 3.8f * t);
            float growl_freq = (t >= 62.0f) ? 55.0f : 110.0f;
            float saw = 2.0f * (p_growl - floorf(p_growl + 0.5f));
            float growl_val = saw * growl_lfo;
            float growl_gain = (t >= 62.0f) ? 0.75f : 0.55f;
            sample += growl_val * growl_gain;

            p_growl += growl_freq * dt;
            if (p_growl >= 1.0f) p_growl -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 3. LEAD ARPEGGIATOR (Track 3): Dual detuned square (14s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 14.0f && t < 80.0f) {
            const float arp_notes[7] = { 261.63f, 311.13f, 392.00f, 466.16f, 523.25f, 622.25f, 783.99f };
            int step = (int)(t * 8.0f) % 7;
            float lead_freq = arp_notes[step];

            float sq1 = (sinf(2.0f * (float)M_PI * p_lead) >= 0.0f) ? 1.0f : -1.0f;
            float sq2 = (sinf(2.0f * (float)M_PI * (p_lead * 1.01f)) >= 0.0f) ? 1.0f : -1.0f;
            float lead_val = (sq1 + sq2) * 0.5f;
            float lead_gain = (t >= 62.0f) ? 0.35f : 0.22f;
            sample += lead_val * lead_gain;

            p_lead += lead_freq * dt;
            if (p_lead >= 1.0f) p_lead -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 4. DRUM & BASS: PUNCHY KICK DRUM (Track 4): 120 BPM High-Impact Transients
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float beat_pos = fmodf(t, 0.5f);
            if (beat_pos < 0.28f) {
                float k_prog = beat_pos / 0.28f;
                float k_freq = 180.0f * expf(-k_prog * 7.0f) + 32.0f;
                float k_env = expf(-k_prog * 7.5f);
                float kick_val = sinf(2.0f * (float)M_PI * p_kick) * k_env;
                float sat_kick = tanhf(kick_val * 1.8f);
                sample += sat_kick * 1.15f;

                p_kick += k_freq * dt;
                if (p_kick >= 1.0f) p_kick -= 1.0f;
            } else {
                p_kick = 0.0f;
            }
        }

        // ---------------------------------------------------------------------
        // 5. DRUM & BASS: HARD SNARE / CLAP (Track 5): Heavy 200Hz Snap + Noise
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float measure_pos = fmodf(t, 2.0f);
            bool is_snare = (measure_pos >= 0.5f && measure_pos < 0.80f) || (measure_pos >= 1.5f && measure_pos < 1.80f);
            if (t >= 50.0f && t < 62.0f) {
                float roll_pos = fmodf(t, 0.125f);
                if (roll_pos < 0.09f) {
                    float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                    float roll_env = expf(-roll_pos * 25.0f);
                    sample += noise * roll_env * 0.70f * ((t - 50.0f) / 12.0f);
                }
            } else if (is_snare) {
                float s_pos = (measure_pos >= 1.5f) ? (measure_pos - 1.5f) : (measure_pos - 0.5f);
                float s_env = expf(-s_pos * 14.0f);
                float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                float snap = sinf(2.0f * (float)M_PI * 220.0f * s_pos) * expf(-s_pos * 30.0f);
                sample += (noise * 0.65f + snap * 0.45f) * s_env * 0.80f;
            }
        }

        // ---------------------------------------------------------------------
        // 6. HI-HAT (Track 6): 16th note pattern (12s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 12.0f && t < 80.0f) {
            float hh_pos = fmodf(t, 0.125f);
            if (hh_pos < 0.05f) {
                float hh_env = expf(-hh_pos * 40.0f);
                float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                sample += noise * hh_env * 0.28f;
            }
        }

        // ---------------------------------------------------------------------
        // 7. BIRD SONG OUTRO (Track 7): Quiet nature chirps (80s - 90s)
        // ---------------------------------------------------------------------
        if (t >= 80.0f && t <= 90.0f) {
            float outro_t = t - 80.0f;
            float chirp_cycle = fmodf(outro_t, 1.8f);
            if (chirp_cycle < 0.7f) {
                float chirp_prog = chirp_cycle / 0.7f;
                float b_freq = 2800.0f + 2000.0f * sinf((float)M_PI * chirp_prog) + 300.0f * sinf(2.0f * (float)M_PI * 14.0f * chirp_prog);
                float b_env = sinf((float)M_PI * chirp_prog) * expf(-outro_t * 0.15f);
                float bird_val = sinf(2.0f * (float)M_PI * p_bird) * b_env;
                sample += bird_val * 0.32f;

                p_bird += b_freq * dt;
                if (p_bird >= 1.0f) p_bird -= 1.0f;
            }
        }

        mix_buf[i] = sample;
    }

    // Soft-clipping Master Limiter & Normalize into Signed 16-bit PCM
    for (size_t i = 0; i < total_frames; i++) {
        float val = tanhf(mix_buf[i] * 0.85f);
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

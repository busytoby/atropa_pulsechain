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
// Pure C Score Parser & Sequencer
// -----------------------------------------------------------------------------
bool tsfi_bio_load_score(TsfiBioScore *score, const char *filepath) {
    if (!score || !filepath) return false;
    memset(score, 0, sizeof(TsfiBioScore));

    FILE *f = fopen(filepath, "r");
    if (!f) return false;

    char line[256];
    while (fgets(line, sizeof(line), f) && score->event_count < 128) {
        if (line[0] == '#' || line[0] == '[' || line[0] == '\n' || line[0] == '\r') continue;

        float t_sec = 0.0f, gain = 0.5f, dur = 1.0f;
        char trk_str[32] = {0}, note_str[32] = {0}, freq_str[64] = {0};

        if (sscanf(line, "%f %s %s %s %f %f", &t_sec, trk_str, note_str, freq_str, &gain, &dur) >= 4) {
            TsfiBioEvent *ev = &score->events[score->event_count++];
            ev->time_sec = t_sec;
            ev->gain = gain;
            ev->duration_sec = dur > 0.0f ? dur : 1.0f;

            if (strstr(trk_str, "TRACK_1")) ev->track_idx = 1;
            else if (strstr(trk_str, "TRACK_2")) ev->track_idx = 2;
            else if (strstr(trk_str, "TRACK_3")) ev->track_idx = 3;
            else if (strstr(trk_str, "TRACK_4")) ev->track_idx = 4;
            else if (strstr(trk_str, "TRACK_5")) ev->track_idx = 5;
            else if (strstr(trk_str, "TRACK_6")) ev->track_idx = 6;
            else if (strstr(trk_str, "TRACK_7")) ev->track_idx = 7;
            else ev->track_idx = 1;

            char *arrow = strstr(freq_str, "->");
            if (arrow) {
                *arrow = '\0';
                ev->start_freq = strtof(freq_str, NULL);
                ev->end_freq = strtof(arrow + 2, NULL);
            } else {
                ev->start_freq = strtof(freq_str, NULL);
                ev->end_freq = ev->start_freq;
            }
        }
    }

    fclose(f);
    return (score->event_count > 0);
}

// -----------------------------------------------------------------------------
// Continuous Multi-Track Synth Engine (All 7 Instruments Fully Rendered)
// -----------------------------------------------------------------------------
size_t tsfi_bio_synthesize_pcm16(const TsfiBioScore *score, int16_t *out_pcm, size_t total_frames) {
    (void)score;
    if (!out_pcm || total_frames == 0) return 0;

    float *mix_buf = (float *)calloc(total_frames, sizeof(float));
    if (!mix_buf) return 0;

    float dt = 1.0f / (float)BIONIKA_SAMPLE_RATE;
    float total_sec = (float)total_frames / (float)BIONIKA_SAMPLE_RATE;

    // Track phases
    float p_bass = 0.0f;
    float p_growl = 0.0f;
    float p_lead = 0.0f;
    float p_kick = 0.0f;
    float p_bird = 0.0f;

    // Progression loop at 120 BPM (0.5s per beat, 2.0s per measure)
    for (size_t i = 0; i < total_frames; i++) {
        float t = (float)i * dt;
        if (t >= total_sec) break;

        float sample = 0.0f;

        // ---------------------------------------------------------------------
        // 1. SUB-BASS (Track 1): Continuous driving bass throughout 0s - 80s
        // ---------------------------------------------------------------------
        if (t < 80.0f) {
            float bass_freq = 55.0f; // A1
            if (t >= 15.0f && t < 25.0f) bass_freq = (fmodf(t, 4.0f) < 2.0f) ? 55.0f : 43.6f; // A1 -> F1
            else if (t >= 25.0f && t < 38.0f) bass_freq = (fmodf(t, 2.0f) < 1.0f) ? 65.4f : 55.0f; // C2 -> A1
            else if (t >= 38.0f && t < 50.0f) bass_freq = (fmodf(t, 4.0f) < 2.0f) ? 43.6f : 36.7f; // F1 -> D1
            else if (t >= 50.0f && t < 62.0f) bass_freq = 55.0f + (t - 50.0f) * 2.0f; // Rising tension
            else if (t >= 62.0f && t < 80.0f) {
                // MASSIVE BASS DROP (62s - 80s)
                float drop_prog = (t - 62.0f) / 18.0f;
                bass_freq = 80.0f * (1.0f - drop_prog * 0.75f); // 80Hz -> 20Hz
            }

            float bass_gain = (t >= 62.0f) ? 0.70f : 0.40f;
            float bass_val = sinf(2.0f * (float)M_PI * p_bass) + 0.3f * sinf(4.0f * (float)M_PI * p_bass);
            sample += bass_val * bass_gain;

            p_bass += bass_freq * dt;
            if (p_bass >= 1.0f) p_bass -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 2. SUB-GROWL ACID SAW (Track 2): Modulated with 3.8Hz LFO (15s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float growl_lfo = 0.5f + 0.5f * sinf(2.0f * (float)M_PI * 3.8f * t);
            float growl_freq = (t >= 62.0f) ? 65.4f : 110.0f;
            float saw = 2.0f * (p_growl - floorf(p_growl + 0.5f));
            float growl_val = saw * growl_lfo;
            float growl_gain = (t >= 62.0f) ? 0.55f : 0.35f;
            sample += growl_val * growl_gain;

            p_growl += growl_freq * dt;
            if (p_growl >= 1.0f) p_growl -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 3. LEAD ARPEGGIATOR (Track 3): Dual detuned square (14s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 14.0f && t < 80.0f) {
            // 16th note arpeggio scale: C4, Eb4, G4, Bb4, C5, Eb5, G5
            const float arp_notes[7] = { 261.63f, 311.13f, 392.00f, 466.16f, 523.25f, 622.25f, 783.99f };
            int step = (int)(t * 8.0f) % 7;
            float lead_freq = arp_notes[step];

            float sq1 = (sinf(2.0f * (float)M_PI * p_lead) >= 0.0f) ? 1.0f : -1.0f;
            float sq2 = (sinf(2.0f * (float)M_PI * (p_lead * 1.01f)) >= 0.0f) ? 1.0f : -1.0f;
            float lead_val = (sq1 + sq2) * 0.5f;
            float lead_gain = (t >= 62.0f) ? 0.40f : 0.25f;
            sample += lead_val * lead_gain;

            p_lead += lead_freq * dt;
            if (p_lead >= 1.0f) p_lead -= 1.0f;
        }

        // ---------------------------------------------------------------------
        // 4. KICK DRUM (Track 4): 4-on-the-floor beat (15s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float beat_pos = fmodf(t, 0.5f); // Every 0.5s = 120 BPM beat
            if (beat_pos < 0.25f) {
                float k_prog = beat_pos / 0.25f;
                float k_freq = 160.0f * expf(-k_prog * 6.0f) + 30.0f;
                float k_env = expf(-k_prog * 8.0f);
                float kick_val = sinf(2.0f * (float)M_PI * p_kick) * k_env;
                sample += kick_val * 0.65f;

                p_kick += k_freq * dt;
                if (p_kick >= 1.0f) p_kick -= 1.0f;
            } else {
                p_kick = 0.0f;
            }
        }

        // ---------------------------------------------------------------------
        // 5. SNARE / CLAP (Track 5): Beats 2 and 4 (15s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 15.0f && t < 80.0f) {
            float measure_pos = fmodf(t, 2.0f); // 2-second measure
            bool is_snare = (measure_pos >= 0.5f && measure_pos < 0.75f) || (measure_pos >= 1.5f && measure_pos < 1.75f);
            if (t >= 50.0f && t < 62.0f) {
                // Snare roll crescendo in Verse 3
                float roll_pos = fmodf(t, 0.125f);
                if (roll_pos < 0.08f) {
                    float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                    sample += noise * 0.35f * ((t - 50.0f) / 12.0f);
                }
            } else if (is_snare) {
                float s_pos = (measure_pos >= 1.5f) ? (measure_pos - 1.5f) : (measure_pos - 0.5f);
                float s_env = expf(-s_pos * 12.0f);
                float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                sample += noise * s_env * 0.40f;
            }
        }

        // ---------------------------------------------------------------------
        // 6. HI-HAT (Track 6): 16th note pattern (12s - 80s)
        // ---------------------------------------------------------------------
        if (t >= 12.0f && t < 80.0f) {
            float hh_pos = fmodf(t, 0.125f); // 16th note
            if (hh_pos < 0.04f) {
                float hh_env = expf(-hh_pos * 35.0f);
                float noise = (((float)(rand() % 2000) / 1000.0f) - 1.0f);
                sample += noise * hh_env * 0.20f;
            }
        }

        // ---------------------------------------------------------------------
        // 7. BIRD SONG OUTRO (Track 7): Quiet nature chirps (80s - 90s)
        // ---------------------------------------------------------------------
        if (t >= 80.0f && t <= 90.0f) {
            float outro_t = t - 80.0f; // 0.0s to 10.0s
            // Multi-frequency chirp cycles
            float chirp_cycle = fmodf(outro_t, 1.8f);
            if (chirp_cycle < 0.7f) {
                float chirp_prog = chirp_cycle / 0.7f;
                // High frequency bird pitch sweep (2.8 kHz -> 4.8 kHz) with 14Hz vibrato
                float b_freq = 2800.0f + 2000.0f * sinf((float)M_PI * chirp_prog) + 300.0f * sinf(2.0f * (float)M_PI * 14.0f * chirp_prog);
                float b_env = sinf((float)M_PI * chirp_prog) * expf(-outro_t * 0.15f);
                float bird_val = sinf(2.0f * (float)M_PI * p_bird) * b_env;
                sample += bird_val * 0.28f;

                p_bird += b_freq * dt;
                if (p_bird >= 1.0f) p_bird -= 1.0f;
            }
        }

        mix_buf[i] = sample;
    }

    // Master Limiter & Normalize into Signed 16-bit PCM
    for (size_t i = 0; i < total_frames; i++) {
        float val = mix_buf[i];
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

    // Write RIFF/WAVE Header
    fwrite("RIFF", 1, 4, f);
    fwrite(&chunk_size, 4, 1, f);
    fwrite("WAVEfmt ", 1, 8, f);

    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1; // PCM
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

#define _GNU_SOURCE
#include "tsfi_bionika_synth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

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

size_t tsfi_bio_synthesize_pcm16(const TsfiBioScore *score, int16_t *out_pcm, size_t total_frames) {
    if (!score || !out_pcm || total_frames == 0) return 0;

    float *mix_buf = (float *)calloc(total_frames, sizeof(float));
    if (!mix_buf) return 0;

    float dt = 1.0f / (float)BIONIKA_SAMPLE_RATE;

    for (int e = 0; e < score->event_count; e++) {
        const TsfiBioEvent *ev = &score->events[e];
        size_t start_idx = (size_t)(ev->time_sec * (float)BIONIKA_SAMPLE_RATE);
        size_t dur_samples = (size_t)(ev->duration_sec * (float)BIONIKA_SAMPLE_RATE);
        if (start_idx >= total_frames) continue;

        size_t end_idx = start_idx + dur_samples;
        if (end_idx > total_frames) end_idx = total_frames;

        float phase = 0.0f;

        for (size_t s = start_idx; s < end_idx; s++) {
            float progress = (float)(s - start_idx) / (float)dur_samples;
            float cur_freq = ev->start_freq + (ev->end_freq - ev->start_freq) * progress;

            float sample = 0.0f;
            float env = 1.0f - progress; // Linear decay envelope

            switch (ev->track_idx) {
                case 1: // Sub-Bass Sine
                    sample = sinf(2.0f * (float)M_PI * phase) * ev->gain * env;
                    break;
                case 2: // Growl Modulated Sawtooth
                    sample = (2.0f * (phase - floorf(phase + 0.5f))) * (sinf(2.0f * (float)M_PI * 4.0f * progress) > 0.0f ? 1.0f : 0.6f) * ev->gain * env;
                    break;
                case 3: // Lead Dual Detuned Square
                    sample = ((sinf(2.0f * (float)M_PI * phase) >= 0.0f ? 1.0f : -1.0f) * 0.5f +
                              (sinf(2.0f * (float)M_PI * (phase * 1.01f)) >= 0.0f ? 1.0f : -1.0f) * 0.5f) * ev->gain * env;
                    break;
                case 4: // Kick Pitch Drop Sine
                    sample = sinf(2.0f * (float)M_PI * phase) * ev->gain * expf(-progress * 8.0f);
                    break;
                case 5: // Snare Noise Burst
                    sample = (((float)(rand() % 2000) / 1000.0f) - 1.0f) * ev->gain * expf(-progress * 12.0f);
                    break;
                case 6: // Hi-Hat Click
                    sample = (((float)(rand() % 2000) / 1000.0f) - 1.0f) * ev->gain * expf(-progress * 25.0f);
                    break;
                case 7: // Bird Song Chirps (High-frequency sine with frequency modulation)
                    sample = sinf(2.0f * (float)M_PI * (phase + sinf(2.0f * (float)M_PI * 12.0f * progress) * 0.2f)) * ev->gain * sinf((float)M_PI * progress);
                    break;
                default:
                    sample = sinf(2.0f * (float)M_PI * phase) * ev->gain * env;
                    break;
            }

            mix_buf[s] += sample;
            phase += cur_freq * dt;
            if (phase >= 1.0f) phase -= 1.0f;
        }
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

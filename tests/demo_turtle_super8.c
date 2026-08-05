#include "../tsfi2-deepseek/src/auncient_turtle_usd.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#pragma pack(push, 1)
typedef struct {
    char     riff[4];
    uint32_t overall_size;
    char     wave[4];
    char     fmt_chunk_marker[4];
    uint32_t length_of_fmt;
    uint16_t format_type;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byterate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char     data_chunk_header[4];
    uint32_t data_size;
} WavHeader;
#pragma pack(pop)

// Note pitch converter helper
static double note_to_freq(int midi) {
    if (midi <= 0) return 0.0;
    return 440.0 * pow(2.0, (midi - 69.0) / 12.0);
}

// Quartet MIDI Score arrays (4 patterns: theme_a, theme_b, theme_a, coda. 32 steps each)
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

static const int midi_violin2[128] = {
    64, 64, 64, 64, 65, 65, 62, 62, 62, 62, 59, 59, 64, 64, 64, 64,
    64, 64, 64, 64, 60, 60, 60, 60, 65, 65, 62, 62, 64, 64, 60, 0,
    67, 67, 67, 67, 69, 69, 65, 65, 65, 65, 62, 62, 67, 67, 67, 67,
    64, 64, 64, 64, 60, 60, 60, 60, 65, 65, 62, 62, 64, 64, 60, 0,
    64, 64, 64, 64, 65, 65, 62, 62, 62, 62, 59, 59, 64, 64, 64, 64,
    64, 64, 64, 64, 60, 60, 60, 60, 65, 65, 62, 62, 64, 64, 60, 0,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 0, 64, 0, 64, 0, 64, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int midi_viola[128] = {
    60, 60, 60, 60, 57, 57, 53, 53, 53, 53, 56, 56, 56, 56, 56, 56,
    57, 57, 57, 57, 52, 52, 57, 57, 57, 57, 53, 53, 56, 56, 52, 0,
    64, 64, 64, 64, 60, 60, 57, 57, 57, 57, 59, 59, 59, 59, 59, 59,
    57, 57, 57, 57, 52, 52, 57, 57, 57, 57, 53, 53, 56, 56, 52, 0,
    60, 60, 60, 60, 57, 57, 53, 53, 53, 53, 56, 56, 56, 56, 56, 56,
    57, 57, 57, 57, 52, 52, 57, 57, 57, 57, 53, 53, 56, 56, 52, 0,
    60, 60, 60, 60, 60, 60, 60, 60, 60, 0, 60, 0, 60, 0, 60, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int midi_bass[128] = {
    45, 45, 45, 45, 38, 38, 38, 38, 40, 40, 40, 40, 45, 45, 45, 45,
    45, 45, 45, 45, 45, 45, 45, 45, 38, 38, 40, 40, 40, 40, 45, 0,
    48, 48, 48, 48, 41, 41, 41, 41, 43, 43, 43, 43, 48, 48, 48, 48,
    45, 45, 45, 45, 45, 45, 45, 45, 38, 38, 40, 40, 40, 40, 45, 0,
    45, 45, 45, 45, 38, 38, 38, 38, 40, 40, 40, 40, 45, 45, 45, 45,
    45, 45, 45, 45, 45, 45, 45, 45, 38, 38, 40, 40, 40, 40, 45, 0,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 0, 45, 0, 45, 0, 45, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT MULTI-LAYER SCHUMANN SOUNDTRACK GENERATOR\n");
    printf("=============================================================\n");

    const char *usda_path = "tests/turtle_super8_demo.usda";
    const char *wav_path = "tests/turtle_super8_demo.wav";

    TSFiRiInterface ri;
    tsfi_riinterface_init(&ri);
    ri.clip_min_x = -500.0;
    ri.clip_min_y = -500.0;
    ri.clip_max_x = 500.0;
    ri.clip_max_y = 500.0;

    auncient_turtle_generate_usd_with_ri(&ri, "F-F+F+F-F", 3, 10.0f, 90.0f, usda_path);

    uint32_t sample_rate = 44100;
    uint32_t duration_sec = 120;
    uint32_t total_samples = sample_rate * duration_sec;

    WavHeader header;
    memcpy(header.riff, "RIFF", 4);
    header.overall_size = sizeof(WavHeader) + total_samples * 2 * 2 - 8;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt_chunk_marker, "fmt ", 4);
    header.length_of_fmt = 16;
    header.format_type = 1;
    header.channels = 2;
    header.sample_rate = sample_rate;
    header.bits_per_sample = 16;
    header.byterate = sample_rate * 2 * 2;
    header.block_align = 2 * 2;
    memcpy(header.data_chunk_header, "data", 4);
    header.data_size = total_samples * 2 * 2;

    FILE *f_wav = fopen(wav_path, "wb");
    if (!f_wav) {
        printf("[ERROR] Failed to create WAV file: %s\n", wav_path);
        return 1;
    }
    fwrite(&header, sizeof(WavHeader), 1, f_wav);

    uint32_t samples_per_step = total_samples / 128; // 128 total steps over 2 mins

    for (uint32_t s = 0; s < total_samples; s++) {
        double t = (double)s / sample_rate;
        uint32_t step_idx = (s / samples_per_step) % 128;
        uint32_t step_samps = s % samples_per_step;
        double step_t = (double)step_samps / sample_rate;

        // Vintage Projector noise
        double hum = 0.10 * sin(2.0 * M_PI * 60.0 * t) + 0.03 * sin(2.0 * M_PI * 120.0 * t);
        double click_t = fmod(t, 1.0 / 18.0);
        double click = (click_t < 0.005) ? sin(2.0 * M_PI * 1100.0 * click_t) * exp(-400.0 * click_t) * 0.20 : 0.0;
        double noise = ((double)rand() / RAND_MAX * 2.0 - 1.0) * 0.01;

        // 1. Lead Violin 1 (Sawtooth)
        double voice_lead = 0.0;
        double f_lead = note_to_freq(midi_lead[step_idx]);
        if (f_lead > 0.0) {
            double phase = fmod(t * f_lead, 1.0);
            voice_lead = (phase * 2.0 - 1.0) * 0.12 * (0.8 + 0.2 * sin(2.0 * M_PI * 6.0 * t));
        }

        // 2. Violin 2 (Square)
        double voice_v2 = 0.0;
        double f_v2 = note_to_freq(midi_violin2[step_idx]);
        if (f_v2 > 0.0) {
            double phase = fmod(t * f_v2, 1.0);
            voice_v2 = (phase < 0.5 ? 0.06 : -0.06) * (0.9 + 0.1 * sin(2.0 * M_PI * 5.0 * t));
        }

        // 3. Viola (Triangle)
        double voice_va = 0.0;
        double f_va = note_to_freq(midi_viola[step_idx]);
        if (f_va > 0.0) {
            double phase = fmod(t * f_va, 1.0);
            voice_va = (phase < 0.5 ? (phase * 4.0 - 1.0) : (3.0 - phase * 4.0)) * 0.08;
        }

        // 4. Cello Bass (Low Sawtooth + Vactrol low-pass filter simulation)
        double voice_vc = 0.0;
        double f_vc = note_to_freq(midi_bass[step_idx]);
        // Vactrol simulation state variables
        static double vactrol_env = 0.0;
        double vactrol_target = (step_samps < samples_per_step / 3) ? 1.0 : 0.0;
        vactrol_env += (vactrol_target - vactrol_env) * (vactrol_target > vactrol_env ? 0.015 : 0.0008); // Vactrol LED response lag

        if (f_vc > 0.0) {
            double raw_saw = (fmod(t * f_vc, 1.0) * 2.0 - 1.0);
            // Low-pass filter with vactrol-modulated cutoff
            static double lpf_state = 0.0;
            double cutoff_hz = 90.0 + vactrol_env * 450.0; // Sweeps from 90Hz to 540Hz
            double rc = 1.0 / (2.0 * M_PI * cutoff_hz);
            double dt_s = 1.0 / sample_rate;
            double alpha = dt_s / (rc + dt_s);
            lpf_state = alpha * raw_saw + (1.0 - alpha) * lpf_state;
            voice_vc = lpf_state * 0.18;
        }

        // 5. NEW LAYER: Sub-Bass Growl (sub-octave sine)
        double voice_growl = 0.0;
        if (f_vc > 0.0) {
            voice_growl = sin(2.0 * M_PI * (f_vc * 0.5) * t) * 0.16;
        }

        // 6. NEW LAYER: Sparkling Arpeggiator (High 16th-note plucks)
        double voice_arp = 0.0;
        uint32_t arp_step = (step_samps / (samples_per_step / 4)) % 4; // 4 sub-steps per score step
        int arp_offset[4] = { 0, 4, 7, 12 };
        int arp_midi = midi_lead[step_idx] ? (midi_lead[step_idx] + arp_offset[arp_step] + 12) : 0;
        double f_arp = note_to_freq(arp_midi);
        if (f_arp > 0.0) {
            double arp_step_t = fmod(step_t, samples_per_step / 4.0 / sample_rate);
            voice_arp = sin(2.0 * M_PI * f_arp * t) * 0.05 * exp(-60.0 * arp_step_t);
        }

        // 7. NEW LAYER: Booty Bass Drop (Exponential sub-bass sweep every 16 steps)
        double voice_kick = 0.0;
        uint32_t drop_interval = samples_per_step * 16;
        uint32_t drop_samps = s % drop_interval;
        double drop_t = (double)drop_samps / sample_rate;
        if (drop_samps < samples_per_step * 6) {
            double sweep_freq = 130.0 * exp(-drop_t * 4.5) + 30.0; // Heavy pitch sweep down to 30Hz
            voice_kick = sin(2.0 * M_PI * sweep_freq * drop_t) * 0.48 * exp(-drop_t * 1.6);
        }

        // 8. NEW LAYER: Shutter Tick Hi-Hat (Noise burst on offbeats)
        double voice_hat = 0.0;
        bool trigger_hat = (step_idx % 2 == 1) && (step_samps < samples_per_step / 8);
        if (trigger_hat) {
            double r_noise = ((double)rand() / RAND_MAX * 2.0 - 1.0);
            voice_hat = r_noise * 0.06 * exp(-step_t * 80.0);
        }

        // 9. NEW LAYER: Dripping Water Tinkles (High-frequency resonant FM drops)
        double voice_drip = 0.0;
        if (s % (sample_rate / 6) < samples_per_step / 16) {
            double drip_t = (double)(s % (sample_rate / 6)) / sample_rate;
            double drip_freq = 1500.0 + sin(t * 5.0) * 800.0;
            voice_drip = sin(2.0 * M_PI * drip_freq * t + 3.0 * sin(2.0 * M_PI * 300.0 * t)) * 0.09 * exp(-75.0 * drip_t);
        }

        // Mix stereo tracks
        double left = hum + click + noise + voice_lead + 0.5 * voice_v2 + 0.3 * voice_va + 0.6 * voice_vc + voice_growl + voice_arp + voice_kick + voice_hat + voice_drip;
        double right = hum - click + noise + voice_lead - 0.5 * voice_v2 + 0.6 * voice_va + 0.3 * voice_vc + voice_growl - voice_arp + voice_kick - voice_hat + voice_drip;

        // Clip
        if (left > 1.0) left = 1.0;
        if (left < -1.0) left = -1.0;
        if (right > 1.0) right = 1.0;
        if (right < -1.0) right = -1.0;

        int16_t out_l = (int16_t)(left * 16384.0);
        int16_t out_r = (int16_t)(right * 16384.0);

        fwrite(&out_l, sizeof(int16_t), 1, f_wav);
        fwrite(&out_r, sizeof(int16_t), 1, f_wav);
    }

    fclose(f_wav);
    printf("[SUCCESS] Generated demo USDA stage: %s\n", usda_path);
    printf("[SUCCESS] Generated demo Schumann Quartet soundtrack: %s\n", wav_path);
    printf("=============================================================\n");
    return 0;
}

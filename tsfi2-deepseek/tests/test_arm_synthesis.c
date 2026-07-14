#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_trie_dispatcher.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 8000
#define TOTAL_SAMPLES 3600 // 450 ms at 8 kHz

// Chamberlin State Variable Filter (SVF) for formants
typedef struct {
    double f; // freq coefficient
    double q; // damping coefficient
    double low;
    double band;
    double high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0); // Stable coefficient approximation
    r->q = 1.0 / Q;
    r->low = 0.0;
    r->band = 0.0;
    r->high = 0.0;
}

static void resonator_update_freq(Resonator *r, double freq) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band; // Bandpass output
}

// 2nd-order Feed-Forward Notch filter for modeling the antiformant (zero)
typedef struct {
    double x1, x2;
    double b0, b1, b2;
} Antiformant;

static void antiformant_init(Antiformant *a, double zero_freq, double radius) {
    double theta = 2.0 * M_PI * zero_freq / SAMPLE_RATE;
    a->b0 = 1.0;
    a->b1 = -2.0 * radius * cos(theta);
    a->b2 = radius * radius;
    a->x1 = 0.0;
    a->x2 = 0.0;
}

static double antiformant_tick(Antiformant *a, double input) {
    double out = a->b0 * input + a->b1 * a->x1 + a->b2 * a->x2;
    a->x2 = a->x1;
    a->x1 = input;
    return out;
}

// Liljencrants-Fant (LF) Glottal Flow Model
static double lf_glottal_pulse(double phase, double open_phase, double closed_phase) {
    double pulse = 0.0;
    if (phase < open_phase) {
        double t = phase / open_phase;
        pulse = exp(1.5 * t) * sin(M_PI * t);
        pulse = (pulse / exp(1.5)) * 1.5;
    } else if (phase < (open_phase + closed_phase)) {
        double t = (phase - open_phase) / closed_phase;
        pulse = 1.5 * exp(-3.0 * t);
    }
    return pulse;
}

// Cubic Hermite Spline interpolation helper for smooth formant trajectories
static double cubic_interpolate(double y0, double y1, double mu) {
    double mu2 = mu * mu;
    double mu3 = mu2 * mu;
    double a0 = 2.0 * mu3 - 3.0 * mu2 + 1.0;
    double a1 = -2.0 * mu3 + 3.0 * mu2;
    return a0 * y0 + a1 * y1;
}

// Write standard 16-bit PCM WAV file header
static void write_wav_header(FILE *fp, uint32_t data_size) {
    uint32_t chunk_size = 36 + data_size;
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t num_channels = 1; // Mono
    uint32_t sample_rate = SAMPLE_RATE;
    uint32_t byte_rate = SAMPLE_RATE * 2;
    uint16_t block_align = 2;
    uint16_t bits_per_sample = 16;

    fwrite("RIFF", 1, 4, fp);
    fwrite(&chunk_size, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);
    fwrite("fmt ", 1, 4, fp);
    fwrite(&subchunk1_size, 4, 1, fp);
    fwrite(&audio_format, 2, 1, fp);
    fwrite(&num_channels, 2, 1, fp);
    fwrite(&sample_rate, 4, 1, fp);
    fwrite(&byte_rate, 4, 1, fp);
    fwrite(&block_align, 2, 1, fp);
    fwrite(&bits_per_sample, 2, 1, fp);
    fwrite("data", 1, 4, fp);
    fwrite(&data_size, 4, 1, fp);
}

int main() {
    printf("[ARM_SYNTH] Initializing formant resonators...\n");

    // Initialize resonators for F1, F2, F3
    Resonator f1, f2, f3;
    resonator_init(&f1, 730.0, 8.0);  // /a/ starting targets
    resonator_init(&f2, 1090.0, 10.0);
    resonator_init(&f3, 2440.0, 12.0);

    // Initialize antiformant for /m/ (spectral null around 800 Hz)
    Antiformant zero;
    antiformant_init(&zero, 800.0, 0.95);

    // Initialize the Phoneme Formant Parameter Trie Router
    tsfi_trie_node *formant_router = tsfi_trie_init_formant_router();
    double aa_f1 = 0, aa_f2 = 0, aa_f3 = 0, aa_zero = 0, aa_voice = 0;
    double er_f1 = 0, er_f2 = 0, er_f3 = 0, er_zero = 0, er_voice = 0;
    double m_f1 = 0, m_f2 = 0, m_f3 = 0, m_zero = 0, m_voice = 0;

    tsfi_trie_resolve_formant(formant_router, "aa", &aa_f1, &aa_f2, &aa_f3, &aa_zero, &aa_voice);
    tsfi_trie_resolve_formant(formant_router, "er", &er_f1, &er_f2, &er_f3, &er_zero, &er_voice);
    tsfi_trie_resolve_formant(formant_router, "m", &m_f1, &m_f2, &m_f3, &m_zero, &m_voice);

    FILE *fp = fopen("arm_synthesis.wav", "wb");
    if (!fp) {
        perror("Failed to open output file");
        return 1;
    }

    write_wav_header(fp, TOTAL_SAMPLES * 2);

    double glottal_phase = 0.0;
    double glottal_f0 = 110.0; // Male pitch
    double dt = 1.0 / SAMPLE_RATE;
    double last_pulse = 0.0;
    double last_noise = 0.0;

    int16_t *pcm_buffer = malloc(TOTAL_SAMPLES * sizeof(int16_t));

    for (int t = 0; t < TOTAL_SAMPLES; t++) {
        // --- 1. Formant Trajectory Interpolation ---
        double f1_tgt = aa_f1, f2_tgt = aa_f2, f3_tgt = aa_f3;
        double zero_mix = 0.0; // Only activate zero during /m/
        double voicing_strength = aa_voice;

        if (t < 1200) {
            // /a/ vowel stage (0 - 150 ms)
            f1_tgt = aa_f1;
            f2_tgt = aa_f2;
            f3_tgt = aa_f3;
            voicing_strength = aa_voice;
        } else if (t < 1600) {
            // Transition /a/ to /r/ (150 - 200 ms) using smooth cubic spline interpolation
            double ratio = (t - 1200) / 400.0;
            f1_tgt = cubic_interpolate(aa_f1, er_f1, ratio);
            f2_tgt = cubic_interpolate(aa_f2, er_f2, ratio);
            f3_tgt = cubic_interpolate(aa_f3, er_f3, ratio);
            voicing_strength = cubic_interpolate(aa_voice, er_voice, ratio);
        } else if (t < 2000) {
            // /r/ glide stage (200 - 250 ms)
            f1_tgt = er_f1;
            f2_tgt = er_f2;
            f3_tgt = er_f3;
            voicing_strength = er_voice;
        } else if (t < 2400) {
            // Transition /r/ to /m/ (250 - 300 ms) using smooth cubic spline interpolation
            double ratio = (t - 2000) / 400.0;
            f1_tgt = cubic_interpolate(er_f1, m_f1, ratio);
            f2_tgt = cubic_interpolate(er_f2, m_f2, ratio);
            f3_tgt = cubic_interpolate(er_f3, m_f3, ratio);
            voicing_strength = cubic_interpolate(er_voice, m_voice, ratio);
            zero_mix = ratio * m_zero; // Fade in nasal zero
        } else {
            // /m/ nasal stage (300 - 450 ms)
            f1_tgt = m_f1;
            f2_tgt = m_f2;
            f3_tgt = m_f3;
            voicing_strength = m_voice;
            zero_mix = m_zero;
        }

        // Apply parameter updates to resonators
        resonator_update_freq(&f1, f1_tgt);
        resonator_update_freq(&f2, f2_tgt);
        resonator_update_freq(&f3, f3_tgt);

        // --- 2. Voicing Source Generation ---
        glottal_phase += glottal_f0 * dt;
        if (glottal_phase >= 1.0) {
            glottal_phase -= 1.0;
        }

        // Liljencrants-Fant (LF) glottal pulse shape
        double pulse = lf_glottal_pulse(glottal_phase, 0.6, 0.15);

        // Add small aspiration noise floor for realistic breathiness
        double noise = ((double)rand() / RAND_MAX - 0.5) * 0.02;

        // Apply Multi-Band Mixed Excitation (MBE) Filter mapping
        double pulse_lp = 0.6 * pulse + 0.4 * last_pulse;
        last_pulse = pulse;

        double noise_hp = 0.8 * noise - 0.8 * last_noise;
        last_noise = noise;

        double excitation = voicing_strength * pulse_lp + (1.0 - voicing_strength) * noise_hp;

        // --- 3. Filtering Network ---
        // Resonators in parallel
        double out_f1 = resonator_tick(&f1, excitation);
        double out_f2 = resonator_tick(&f2, excitation);
        double out_f3 = resonator_tick(&f3, excitation);

        // Mix formant outputs (relative gains to optimize spectral balance)
        double synth_signal = 1.0 * out_f1 + 0.5 * out_f2 + 0.25 * out_f3;

        // Apply antiformant zero if nasalized
        if (zero_mix > 0.001) {
            double zero_signal = antiformant_tick(&zero, synth_signal);
            synth_signal = (1.0 - zero_mix) * synth_signal + zero_mix * zero_signal;
        }

        // Fade out at the end to prevent click
        if (t > TOTAL_SAMPLES - 400) {
            double fade = (double)(TOTAL_SAMPLES - t) / 400.0;
            synth_signal *= fade;
        }

        // Scale and clip to 16-bit signed range
        double scaled = synth_signal * 12000.0;
        if (scaled > 32767.0) scaled = 32767.0;
        if (scaled < -32768.0) scaled = -32768.0;

        pcm_buffer[t] = (int16_t)scaled;
    }

    fwrite(pcm_buffer, sizeof(int16_t), TOTAL_SAMPLES, fp);
    fclose(fp);
    free(pcm_buffer);

    tsfi_trie_destroy(formant_router);

    printf("[ARM_SYNTH] Completed. Wrote 450 ms of synthesized speech to 'arm_synthesis.wav'\n");
    return 0;
}

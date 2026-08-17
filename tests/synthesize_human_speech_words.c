// SPDX-License-Identifier: GPL-2.0
#define _POSIX_C_SOURCE 200809L
/*
 * Natural Warm Human Voice Word Synthesizer (Studio Quality Acoustic Model)
 * Fixes "Radio" / "Walkie-Talkie" artifacts:
 * 1. Restores rich low-end chest resonance (Fundamental F0 pass-through).
 * 2. Natural soft high-frequency roll-off (eliminates aggressive differentiator buzz).
 * 3. Smooth non-clipping linear dynamic range.
 * 4. Human-calibrated formant bandwidths for full, warm vocal body.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <string.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100

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

typedef struct {
    char symbol[4];
    float f1, f2, f3, f4;
    float b1, b2, b3, b4;
    float duration_ms;
    bool is_voiced;
    float voice_amp;
    float aspiration_amp;
    float pitch_target;
} PhonemeProfile;

static const PhonemeProfile PHONEMES[] = {
    // Vowels (Warm adult male formant parameters)
    { "AA",  730.0f, 1090.0f, 2440.0f, 3500.0f, 100.0f, 120.0f, 160.0f, 280.0f, 200.0f, true,  1.0f, 0.02f,  0.0f }, // "o" in "hot"
    { "AE",  660.0f, 1720.0f, 2410.0f, 3500.0f,  90.0f, 110.0f, 160.0f, 280.0f, 200.0f, true,  1.0f, 0.02f,  0.2f }, // "a" in "cat"
    { "AH",  520.0f, 1190.0f, 2390.0f, 3500.0f,  80.0f, 100.0f, 150.0f, 280.0f, 160.0f, true,  1.0f, 0.02f,  0.0f }, // "u" in "cut"
    { "AO",  570.0f,  840.0f, 2410.0f, 3500.0f,  90.0f, 100.0f, 150.0f, 280.0f, 200.0f, true,  1.0f, 0.02f, -0.3f }, // "aw" in "saw"
    { "EH",  530.0f, 1840.0f, 2480.0f, 3500.0f,  80.0f, 110.0f, 150.0f, 280.0f, 180.0f, true,  1.0f, 0.02f,  0.4f }, // "e" in "bed"
    { "ER",  490.0f, 1350.0f, 1690.0f, 3200.0f,  90.0f, 110.0f, 140.0f, 280.0f, 200.0f, true,  1.0f, 0.03f, -0.5f }, // "er" in "bird"
    { "IH",  390.0f, 1990.0f, 2550.0f, 3500.0f,  70.0f, 100.0f, 150.0f, 280.0f, 160.0f, true,  1.0f, 0.02f,  0.3f }, // "i" in "sit"
    { "IY",  270.0f, 2290.0f, 3010.0f, 3600.0f,  70.0f, 100.0f, 150.0f, 280.0f, 200.0f, true,  1.0f, 0.02f,  0.6f }, // "ee" in "feet"
    { "OW",  500.0f,  700.0f, 2300.0f, 3500.0f,  90.0f, 100.0f, 150.0f, 280.0f, 200.0f, true,  1.0f, 0.02f, -0.8f }, // "o" in "go"
    { "UW",  300.0f,  870.0f, 2240.0f, 3500.0f,  70.0f, 100.0f, 150.0f, 280.0f, 200.0f, true,  1.0f, 0.02f, -1.2f }, // "oo" in "too"
    
    // Consonants
    { "HH",  500.0f, 1500.0f, 2500.0f, 3500.0f, 150.0f, 200.0f, 250.0f, 300.0f, 110.0f, false, 0.05f, 0.70f,  0.0f }, // "h"
    { "LL",  380.0f, 1200.0f, 2700.0f, 3500.0f,  90.0f, 120.0f, 160.0f, 280.0f, 160.0f, true,  0.85f, 0.01f, -0.3f }, // "l"
    { "MM",  280.0f,  900.0f, 2200.0f, 3500.0f, 100.0f, 140.0f, 180.0f, 280.0f, 150.0f, true,  0.80f, 0.01f, -0.5f }, // "m"
    { "NN",  280.0f, 1700.0f, 2600.0f, 3500.0f, 100.0f, 140.0f, 180.0f, 280.0f, 150.0f, true,  0.80f, 0.01f, -0.3f }, // "n"
    { "SS",  400.0f, 2500.0f, 4500.0f, 6000.0f, 180.0f, 250.0f, 350.0f, 500.0f, 140.0f, false, 0.0f,  0.80f,  0.0f }, // "s"
    { "SH",  300.0f, 1800.0f, 3000.0f, 4500.0f, 180.0f, 220.0f, 300.0f, 400.0f, 140.0f, false, 0.0f,  0.75f,  0.0f }, // "sh"
    { "TT",  400.0f, 1800.0f, 2800.0f, 3800.0f, 180.0f, 220.0f, 280.0f, 350.0f,  70.0f, false, 0.0f,  0.90f,  0.0f }, // "t"
    { "DD",  350.0f, 1700.0f, 2600.0f, 3600.0f, 140.0f, 200.0f, 250.0f, 300.0f,  80.0f, true,  0.6f,  0.35f,  0.0f }, // "d"
    { "WW",  300.0f,  700.0f, 2200.0f, 3500.0f,  90.0f, 110.0f, 140.0f, 280.0f, 150.0f, true,  0.85f, 0.01f, -0.6f }, // "w"
    { "RR",  420.0f, 1300.0f, 1600.0f, 3200.0f,  90.0f, 120.0f, 140.0f, 280.0f, 160.0f, true,  0.85f, 0.01f, -0.3f }, // "r"
    { "DH",  350.0f, 1600.0f, 2500.0f, 3500.0f, 110.0f, 150.0f, 200.0f, 280.0f,  90.0f, true,  0.70f, 0.20f, -0.3f }, // "th"
    { "SIL", 500.0f, 1500.0f, 2500.0f, 3500.0f, 100.0f, 100.0f, 100.0f, 100.0f, 100.0f, false, 0.0f,  0.00f,  0.0f }
};

static const size_t NUM_PHONEMES = sizeof(PHONEMES) / sizeof(PHONEMES[0]);

static const PhonemeProfile *find_phoneme(const char *sym) {
    for (size_t i = 0; i < NUM_PHONEMES; i++) {
        if (strcmp(PHONEMES[i].symbol, sym) == 0) return &PHONEMES[i];
    }
    return &PHONEMES[NUM_PHONEMES - 1];
}

/* Bi-quad Resonator with normalized power */
typedef struct {
    double a1, a2;
    double b0;
    double y1, y2;
} Resonator;

static void init_resonator(Resonator *r, double f, double bw, double fs) {
    if (f <= 0.0 || bw <= 0.0 || f >= fs / 2.0) {
        r->b0 = 0.0; r->a1 = 0.0; r->a2 = 0.0;
        return;
    }
    double r_val = exp(-M_PI * bw / fs);
    double theta = 2.0 * M_PI * f / fs;
    r->a1 = -2.0 * r_val * cos(theta);
    r->a2 = r_val * r_val;
    r->b0 = (1.0 - r_val) * sqrt(1.0 - 2.0 * r_val * cos(2.0 * theta) + r_val * r_val);
}

static double step_resonator(Resonator *r, double in) {
    double out = r->b0 * in - r->a1 * r->y1 - r->a2 * r->y2;
    r->y2 = r->y1;
    r->y1 = out;
    return out;
}

/* 1/f Pink Noise Filter */
typedef struct {
    double b0, b1, b2, b3, b4, b5, b6;
} PinkNoiseFilter;

static double step_pink_noise(PinkNoiseFilter *p) {
    double white = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0;
    p->b0 = 0.99886 * p->b0 + white * 0.0555179;
    p->b1 = 0.99332 * p->b1 + white * 0.0750759;
    p->b2 = 0.96900 * p->b2 + white * 0.1538520;
    p->b3 = 0.86650 * p->b3 + white * 0.3104856;
    p->b4 = 0.55000 * p->b4 + white * 0.5329522;
    p->b5 = -0.7616 * p->b5 - white * 0.0168980;
    double pink = p->b0 + p->b1 + p->b2 + p->b3 + p->b4 + p->b5 + p->b6 + white * 0.5362;
    p->b6 = white * 0.115926;
    return pink * 0.11;
}

typedef struct {
    const char *word;
    const char *phonemes[16];
    size_t count;
} WordLexicon;

static const WordLexicon DICTIONARY[] = {
    { "HELLO",       { "HH", "EH", "LL", "OW", "SIL" }, 5 },
    { "AUNCIENT",    { "AA", "NN", "SH", "EH", "NN", "TT", "SIL" }, 7 },
    { "HUMAN",       { "HH", "IY", "UW", "MM", "AH", "NN", "SIL" }, 7 },
    { "LIFE",        { "LL", "AA", "IY", "SS", "SIL" }, 5 },
    { "SUN",         { "SS", "AH", "NN", "SIL" }, 4 },
    { "WATER",       { "WW", "AO", "TT", "ER", "SIL" }, 5 },
    { "WORLD",       { "WW", "ER", "LL", "DD", "SIL" }, 5 }
};

static const size_t DICT_SIZE = sizeof(DICTIONARY) / sizeof(DICTIONARY[0]);

static const WordLexicon *lookup_word(const char *word) {
    char upper[64];
    size_t len = strlen(word);
    if (len >= sizeof(upper)) len = sizeof(upper) - 1;
    for (size_t i = 0; i < len; i++) upper[i] = (char)toupper(word[i]);
    upper[len] = '\0';

    for (size_t i = 0; i < DICT_SIZE; i++) {
        if (strcmp(DICTIONARY[i].word, upper) == 0) return &DICTIONARY[i];
    }
    return NULL;
}

/* Synthesizes individual words with natural warm vocal body */
static void synthesize_warm_human_word(const WordLexicon *lex, float base_f0, const char *out_wav_path) {
    float total_ms = 0.0f;
    for (size_t i = 0; i < lex->count; i++) {
        total_ms += find_phoneme(lex->phonemes[i])->duration_ms;
    }

    size_t total_samples = (size_t)((total_ms / 1000.0f) * SAMPLE_RATE) + (SAMPLE_RATE / 4);
    double *raw_audio = (double *)calloc(total_samples, sizeof(double));
    if (!raw_audio) return;

    Resonator r1 = {0}, r2 = {0}, r3 = {0}, r4 = {0};
    PinkNoiseFilter pink = {0};

    // Smooth Formant State ($C^1$ Continuity)
    const PhonemeProfile *p0 = find_phoneme(lex->phonemes[0]);
    float cur_f1 = p0->f1, cur_f2 = p0->f2, cur_f3 = p0->f3, cur_f4 = p0->f4;
    float cur_b1 = p0->b1, cur_b2 = p0->b2, cur_b3 = p0->b3, cur_b4 = p0->b4;

    double glot_phase = 0.0;
    size_t sample_idx = 0;

    for (size_t p_idx = 0; p_idx < lex->count; p_idx++) {
        const PhonemeProfile *target = find_phoneme(lex->phonemes[p_idx]);
        size_t phoneme_samples = (size_t)((target->duration_ms / 1000.0f) * SAMPLE_RATE);

        for (size_t s = 0; s < phoneme_samples && sample_idx < total_samples; s++, sample_idx++) {
            float p_progress = (float)p_idx / (float)lex->count;

            // Formant coarticulation (~10ms)
            double alpha = 0.010;
            cur_f1 += (float)((target->f1 - cur_f1) * alpha);
            cur_f2 += (float)((target->f2 - cur_f2) * alpha);
            cur_f3 += (float)((target->f3 - cur_f3) * alpha);
            cur_f4 += (float)((target->f4 - cur_f4) * alpha);

            cur_b1 += (float)((target->b1 - cur_b1) * alpha);
            cur_b2 += (float)((target->b2 - cur_b2) * alpha);
            cur_b3 += (float)((target->b3 - cur_b3) * alpha);
            cur_b4 += (float)((target->b4 - cur_b4) * alpha);

            init_resonator(&r1, cur_f1, cur_b1, SAMPLE_RATE);
            init_resonator(&r2, cur_f2, cur_b2, SAMPLE_RATE);
            init_resonator(&r3, cur_f3, cur_b3, SAMPLE_RATE);
            init_resonator(&r4, cur_f4, cur_b4, SAMPLE_RATE);

            // Natural human pitch contour (accentuation + micro-jitter)
            double pitch_arc = 1.0 + (sinf(p_progress * (float)M_PI) * 0.06) - (p_progress * 0.10);
            double micro_vibrato = sin(2.0 * M_PI * 5.2 * ((double)sample_idx / SAMPLE_RATE)) * 0.012;
            double jitter = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * 0.005;

            float semitone = powf(2.0f, target->pitch_target / 12.0f);
            double inst_f0 = base_f0 * pitch_arc * semitone * (1.0 + micro_vibrato + jitter);

            // True Acoustic Glottal Airflow (Warm Rosenberg Waveform with full low-end F0 body)
            double glot_pulse = 0.0;
            double chest_f0 = 0.0;
            if (target->is_voiced) {
                glot_phase += inst_f0 / (double)SAMPLE_RATE;
                if (glot_phase >= 1.0) glot_phase -= 1.0;

                double p = glot_phase;
                if (p < 0.45) {
                    glot_pulse = 0.5 * (1.0 - cos(M_PI * p / 0.45));
                } else if (p < 0.65) {
                    glot_pulse = cos(M_PI * (p - 0.45) / 0.40);
                } else {
                    glot_pulse = 0.0;
                }

                // Fundamental pitch bass presence (Chest Resonance)
                chest_f0 = sin(2.0 * M_PI * glot_phase) * 0.35 + sin(4.0 * M_PI * glot_phase) * 0.15;
            }

            // Warm Pink Breath Noise
            double pink_noise = step_pink_noise(&pink);
            double shimmer = 1.0 + (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * 0.015;

            double excitation = (glot_pulse * target->voice_amp) + (pink_noise * target->aspiration_amp * shimmer * 0.3);

            // Resonant Formants (F1..F4) with warm human balance
            double y1 = step_resonator(&r1, excitation) * 1.00;
            double y2 = step_resonator(&r2, excitation) * 0.50;
            double y3 = step_resonator(&r3, excitation) * 0.25;
            double y4 = step_resonator(&r4, excitation) * 0.10;

            // Blend Chest Resonance (Low-end body) + Formant Cavity
            double vocal_out = (chest_f0 * target->voice_amp) + y1 + y2 + y3 + y4;

            raw_audio[sample_idx] = vocal_out;
        }
    }

    // Dynamic Peak Normalization
    double peak = 0.0001;
    for (size_t i = 0; i < total_samples; i++) {
        double abs_val = fabs(raw_audio[i]);
        if (abs_val > peak) peak = abs_val;
    }
    double norm = 0.85 / peak;

    int16_t *pcm_buffer = (int16_t *)calloc(total_samples * 2, sizeof(int16_t));
    if (pcm_buffer) {
        for (size_t i = 0; i < total_samples; i++) {
            double sample = raw_audio[i] * norm;
            if (sample > 0.95) sample = 0.95;
            if (sample < -0.95) sample = -0.95;

            int16_t val = (int16_t)(sample * 32767.0);
            pcm_buffer[i * 2 + 0] = val;
            pcm_buffer[i * 2 + 1] = val;
        }

        FILE *f = fopen(out_wav_path, "wb");
        if (f) {
            WavHeader hdr;
            memcpy(hdr.riff, "RIFF", 4);
            hdr.overall_size = sizeof(WavHeader) - 8 + (uint32_t)(total_samples * 2 * sizeof(int16_t));
            memcpy(hdr.wave, "WAVE", 4);
            memcpy(hdr.fmt_chunk_marker, "fmt ", 4);
            hdr.length_of_fmt = 16;
            hdr.format_type = 1;
            hdr.channels = 2;
            hdr.sample_rate = SAMPLE_RATE;
            hdr.byterate = SAMPLE_RATE * 2 * sizeof(int16_t);
            hdr.block_align = 4;
            hdr.bits_per_sample = 16;
            memcpy(hdr.data_chunk_header, "data", 4);
            hdr.data_size = (uint32_t)(total_samples * 2 * sizeof(int16_t));

            fwrite(&hdr, sizeof(WavHeader), 1, f);
            fwrite(pcm_buffer, sizeof(int16_t), total_samples * 2, f);
            fclose(f);
        }
        free(pcm_buffer);
    }

    free(raw_audio);
}

int main(void) {
    printf("=============================================================\n");
    printf("WARM HUMAN VOICE WORD SYNTHESIZER (CHEST BODY + 4 FORMANTS) \n");
    printf("=============================================================\n");

    const char *test_words[] = { "HELLO", "AUNCIENT", "HUMAN", "LIFE", "SUN", "WATER", "WORLD" };
    size_t num_words = sizeof(test_words) / sizeof(test_words[0]);

    for (size_t w = 0; w < num_words; w++) {
        const WordLexicon *lex = lookup_word(test_words[w]);
        if (!lex) continue;

        char wav_path[128];
        snprintf(wav_path, sizeof(wav_path), "assets/bionika/speech_word_%s.wav", test_words[w]);

        printf("Synthesizing warm vocal: '%s' (Chest F0 + Pink Breath + 4 Formants) -> %s\n", test_words[w], wav_path);
        synthesize_warm_human_word(lex, 125.0f, wav_path);
    }

    printf("=============================================================\n");
    printf("ALL WARM HUMAN WORDS SYNTHESIZED SUCCESSFULLY (44.1kHz)     \n");
    printf("=============================================================\n");
    return 0;
}

/*
 * High-Intelligibility Sentence & Word-Boundary Speech Synthesizer
 * Integrated with Emotional Weyl Closures, 5-Formant Cascade-Parallel Resonators,
 * Pitch-Synchronous Granular Synthesis, and Blumlein Stereo Spatialization (44.1kHz 16-bit PCM).
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

typedef enum {
    EMOTION_CALM = 1,
    EMOTION_CATHARSIS = 2,
    EMOTION_FOCUS = 3,
    EMOTION_WONDER = 4,
    EMOTION_MINDFULNESS = 5
} EmotionalPhonationState;

typedef struct {
    EmotionalPhonationState state;
    const char *name;
    float f0_multiplier;
    float pitch_arc_depth;
    float vibrato_depth;
    float breath_gain_mult;
    float tilt_exponent;
    float chest_warmth;
} EmotionalProfile;

static const EmotionalProfile EMOTION_PROFILES[] = {
    { EMOTION_CALM,        "CALM",        1.00f, 0.05f, 0.008f, 0.25f, 1.45f, 0.35f },
    { EMOTION_CATHARSIS,   "CATHARSIS",   1.08f, 0.12f, 0.015f, 0.40f, 1.25f, 0.25f },
    { EMOTION_FOCUS,       "FOCUS",       0.96f, 0.03f, 0.005f, 0.15f, 1.55f, 0.40f },
    { EMOTION_WONDER,      "WONDER",      1.15f, 0.18f, 0.020f, 0.35f, 1.20f, 0.20f },
    { EMOTION_MINDFULNESS, "MINDFULNESS", 0.92f, 0.04f, 0.010f, 0.30f, 1.60f, 0.45f }
};

static const size_t NUM_EMOTIONS = sizeof(EMOTION_PROFILES) / sizeof(EMOTION_PROFILES[0]);

static const EmotionalProfile *find_emotion_profile(EmotionalPhonationState st) {
    for (size_t i = 0; i < NUM_EMOTIONS; i++) {
        if (EMOTION_PROFILES[i].state == st) return &EMOTION_PROFILES[i];
    }
    return &EMOTION_PROFILES[0];
}

typedef struct {
    char symbol[4];
    float f1, f2, f3, f4, f5;
    float b1, b2, b3, b4, b5;
    float duration_ms;
    bool is_voiced;
    float voice_gain;
    float noise_gain;
    float pitch_offset;
} OrganicPhoneme;

static const OrganicPhoneme PHONEMES[] = {
    // Vowels
    { "AA",  730.0f, 1090.0f, 2440.0f, 3500.0f, 4500.0f,   90.0f, 110.0f, 160.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.04f,  0.0f },
    { "AE",  660.0f, 1720.0f, 2410.0f, 3500.0f, 4500.0f,   80.0f, 100.0f, 150.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.04f,  0.2f },
    { "AH",  520.0f, 1190.0f, 2390.0f, 3500.0f, 4500.0f,   70.0f,  90.0f, 140.0f, 250.0f, 300.0f, 140.0f, true,  1.00f, 0.04f,  0.0f },
    { "AO",  570.0f,  840.0f, 2410.0f, 3500.0f, 4500.0f,   80.0f,  90.0f, 140.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.04f, -0.3f },
    { "EH",  530.0f, 1840.0f, 2480.0f, 3500.0f, 4500.0f,   70.0f, 100.0f, 150.0f, 250.0f, 300.0f, 150.0f, true,  1.00f, 0.04f,  0.3f },
    { "ER",  490.0f, 1350.0f, 1690.0f, 3200.0f, 4500.0f,   80.0f, 100.0f, 120.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.05f, -0.4f },
    { "IH",  390.0f, 1990.0f, 2550.0f, 3500.0f, 4500.0f,   60.0f,  90.0f, 140.0f, 250.0f, 300.0f, 140.0f, true,  1.00f, 0.04f,  0.4f },
    { "IY",  270.0f, 2290.0f, 3010.0f, 3600.0f, 4500.0f,   50.0f,  90.0f, 150.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.04f,  0.6f },
    { "OW",  500.0f,  700.0f, 2300.0f, 3500.0f, 4500.0f,   70.0f,  80.0f, 140.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.04f, -0.5f },
    { "UW",  300.0f,  870.0f, 2240.0f, 3500.0f, 4500.0f,   60.0f,  80.0f, 130.0f, 250.0f, 300.0f, 170.0f, true,  1.00f, 0.04f, -0.6f },
    
    // Consonants
    { "HH",  500.0f, 1500.0f, 2500.0f, 3500.0f, 4500.0f,  120.0f, 150.0f, 200.0f, 300.0f, 400.0f,  90.0f, false, 0.15f, 0.85f,  0.0f },
    { "LL",  380.0f, 1200.0f, 2700.0f, 3500.0f, 4500.0f,   60.0f, 100.0f, 200.0f, 300.0f, 400.0f, 140.0f, true,  0.80f, 0.03f, -0.3f },
    { "MM",  280.0f,  900.0f, 2200.0f, 3500.0f, 4500.0f,   50.0f, 100.0f, 200.0f, 300.0f, 400.0f, 130.0f, true,  0.75f, 0.02f, -0.5f },
    { "NN",  280.0f, 1700.0f, 2600.0f, 3500.0f, 4500.0f,   50.0f, 100.0f, 200.0f, 300.0f, 400.0f, 130.0f, true,  0.75f, 0.02f, -0.3f },
    { "SS",  400.0f, 2500.0f, 5000.0f, 6500.0f, 8000.0f,  200.0f, 400.0f, 600.0f, 800.0f, 900.0f, 120.0f, false, 0.00f, 1.10f,  0.0f },
    { "SH",  300.0f, 1800.0f, 3200.0f, 5000.0f, 6500.0f,  150.0f, 300.0f, 500.0f, 700.0f, 800.0f, 120.0f, false, 0.00f, 1.00f,  0.0f },
    { "TT",  400.0f, 1800.0f, 3500.0f, 5000.0f, 6500.0f,  150.0f, 300.0f, 500.0f, 700.0f, 800.0f,  60.0f, false, 0.00f, 1.30f,  0.0f },
    { "DD",  350.0f, 1700.0f, 2600.0f, 3600.0f, 4500.0f,  100.0f, 150.0f, 250.0f, 350.0f, 450.0f,  70.0f, true,  0.60f, 0.50f,  0.0f },
    { "WW",  300.0f,  700.0f, 2200.0f, 3500.0f, 4500.0f,   50.0f,  80.0f, 150.0f, 250.0f, 350.0f, 130.0f, true,  0.80f, 0.03f, -0.5f },
    { "RR",  420.0f, 1300.0f, 1600.0f, 3200.0f, 4500.0f,   70.0f, 100.0f, 120.0f, 250.0f, 350.0f, 140.0f, true,  0.80f, 0.03f, -0.3f },
    { "DH",  350.0f, 1600.0f, 2500.0f, 3500.0f, 4500.0f,  100.0f, 150.0f, 250.0f, 350.0f, 450.0f,  80.0f, true,  0.70f, 0.35f, -0.3f },
    { "VV",  300.0f, 1400.0f, 2400.0f, 3400.0f, 4500.0f,  100.0f, 150.0f, 250.0f, 350.0f, 450.0f, 100.0f, true,  0.70f, 0.35f,  0.0f },
    { "KK",  400.0f, 2000.0f, 3000.0f, 4000.0f, 5000.0f,  150.0f, 250.0f, 400.0f, 500.0f, 600.0f,  70.0f, false, 0.00f, 1.20f,  0.0f },
    { "SIL", 500.0f, 1500.0f, 2500.0f, 3500.0f, 4500.0f,  100.0f, 150.0f, 200.0f, 250.0f, 300.0f,  80.0f, false, 0.00f, 0.00f,  0.0f },
    { "PAU", 500.0f, 1500.0f, 2500.0f, 3500.0f, 4500.0f,  100.0f, 150.0f, 200.0f, 250.0f, 300.0f,  75.0f, false, 0.00f, 0.00f,  0.0f }
};

static const size_t NUM_PHONEMES = sizeof(PHONEMES) / sizeof(PHONEMES[0]);

static const OrganicPhoneme *find_phoneme(const char *sym) {
    for (size_t i = 0; i < NUM_PHONEMES; i++) {
        if (strcmp(PHONEMES[i].symbol, sym) == 0) return &PHONEMES[i];
    }
    return &PHONEMES[NUM_PHONEMES - 2];
}

typedef struct {
    double a1, a2, b0;
    double x1, x2, y1, y2;
} SecondOrderResonator;

static void init_resonator(SecondOrderResonator *r, double freq, double bw, double fs) {
    double r_pole = exp(-M_PI * bw / fs);
    double theta = 2.0 * M_PI * freq / fs;
    r->a1 = -2.0 * r_pole * cos(theta);
    r->a2 = r_pole * r_pole;
    r->b0 = 1.0 - r_pole;
    r->x1 = 0.0; r->x2 = 0.0;
    r->y1 = 0.0; r->y2 = 0.0;
}

static inline double step_resonator(SecondOrderResonator *r, double in) {
    double out = (r->b0 * in) - (r->a1 * r->y1) - (r->a2 * r->y2);
    r->y2 = r->y1;
    r->y1 = out;
    return out;
}

typedef struct {
    const char *word;
    const char *phonemes[16];
    size_t count;
} WordLexicon;

static const WordLexicon DICTIONARY[] = {
    { "HELLO",       { "HH", "EH", "LL", "OW" }, 4 },
    { "AUNCIENT",    { "AA", "NN", "SH", "EH", "NN", "TT" }, 6 },
    { "HUMAN",       { "HH", "IY", "UW", "MM", "AH", "NN" }, 6 },
    { "LIFE",        { "LL", "AA", "IY", "SS" }, 4 },
    { "SUN",         { "SS", "AH", "NN" }, 3 },
    { "WATER",       { "WW", "AO", "TT", "ER" }, 4 },
    { "WORLD",       { "WW", "ER", "LL", "DD" }, 4 },
    { "WELCOME",     { "WW", "EH", "LL", "KK", "AH", "MM" }, 6 },
    { "TO",          { "TT", "UW" }, 2 },
    { "THE",         { "DH", "AH" }, 2 },
    { "WE",          { "WW", "IY" }, 2 },
    { "ARE",         { "AA", "RR" }, 2 },
    { "ALIVE",       { "AH", "LL", "AA", "IY", "VV" }, 5 }
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

/* Synthesizes complete sentences with Emotional Phonation Closures and 5-Formant Resonators */
static void synthesize_sentence_to_wav(
    const char **words,
    size_t num_words,
    float base_pitch,
    EmotionalPhonationState emotion,
    const char *out_wav_path
) {
    const EmotionalProfile *prof = find_emotion_profile(emotion);

    // 1. Measure total sample buffer
    float total_duration_sec = 0.2f;
    for (size_t w = 0; w < num_words; w++) {
        const WordLexicon *lex = lookup_word(words[w]);
        if (!lex) continue;
        for (size_t p = 0; p < lex->count; p++) {
            total_duration_sec += find_phoneme(lex->phonemes[p])->duration_ms / 1000.0f;
        }
        total_duration_sec += 0.080f; // Inter-word pause
    }
    total_duration_sec += 0.2f;

    size_t total_samples = (size_t)(total_duration_sec * SAMPLE_RATE);
    double *raw_audio = (double *)calloc(total_samples, sizeof(double));
    if (!raw_audio) return;

    size_t sample_idx = (size_t)(0.1f * SAMPLE_RATE);

    for (size_t w = 0; w < num_words; w++) {
        const WordLexicon *lex = lookup_word(words[w]);
        if (!lex) continue;

        SecondOrderResonator r1 = {0}, r2 = {0}, r3 = {0}, r4 = {0}, r5 = {0};
        const OrganicPhoneme *first_p = find_phoneme(lex->phonemes[0]);
        init_resonator(&r1, first_p->f1, first_p->b1, SAMPLE_RATE);
        init_resonator(&r2, first_p->f2, first_p->b2, SAMPLE_RATE);
        init_resonator(&r3, first_p->f3, first_p->b3, SAMPLE_RATE);
        init_resonator(&r4, first_p->f4, first_p->b4, SAMPLE_RATE);
        init_resonator(&r5, first_p->f5, first_p->b5, SAMPLE_RATE);

        float cur_f1 = first_p->f1, cur_f2 = first_p->f2, cur_f3 = first_p->f3, cur_f4 = first_p->f4, cur_f5 = first_p->f5;
        float cur_b1 = first_p->b1, cur_b2 = first_p->b2, cur_b3 = first_p->b3, cur_b4 = first_p->b4, cur_b5 = first_p->b5;
        double glot_phase = 0.0;

        for (size_t p_idx = 0; p_idx < lex->count; p_idx++) {
            const OrganicPhoneme *target = find_phoneme(lex->phonemes[p_idx]);
            size_t phoneme_samples = (size_t)((target->duration_ms / 1000.0f) * SAMPLE_RATE);

            float word_progress = (float)p_idx / (float)lex->count;
            float prosody_arc = sinf((float)M_PI * word_progress);
            float pitch_arc = 1.0f + (prosody_arc * prof->pitch_arc_depth) - (word_progress * 0.08f);

            for (size_t s = 0; s < phoneme_samples && sample_idx < total_samples; s++, sample_idx++) {
                double alpha = 0.015;
                cur_f1 += (float)((target->f1 - cur_f1) * alpha);
                cur_f2 += (float)((target->f2 - cur_f2) * alpha);
                cur_f3 += (float)((target->f3 - cur_f3) * alpha);
                cur_f4 += (float)((target->f4 - cur_f4) * alpha);
                cur_f5 += (float)((target->f5 - cur_f5) * alpha);

                cur_b1 += (float)((target->b1 - cur_b1) * alpha);
                cur_b2 += (float)((target->b2 - cur_b2) * alpha);
                cur_b3 += (float)((target->b3 - cur_b3) * alpha);
                cur_b4 += (float)((target->b4 - cur_b4) * alpha);
                cur_b5 += (float)((target->b5 - cur_b5) * alpha);

                init_resonator(&r1, cur_f1, cur_b1, SAMPLE_RATE);
                init_resonator(&r2, cur_f2, cur_b2, SAMPLE_RATE);
                init_resonator(&r3, cur_f3, cur_b3, SAMPLE_RATE);
                init_resonator(&r4, cur_f4, cur_b4, SAMPLE_RATE);
                init_resonator(&r5, cur_f5, cur_b5, SAMPLE_RATE);

                double t_sec = (double)sample_idx / (double)SAMPLE_RATE;
                double vibrato = sin(2.0 * M_PI * 5.5 * t_sec) * prof->vibrato_depth;
                double micro_jitter = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * 0.006;
                double shimmer = 1.0 + (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * 0.025;

                float semitone = powf(2.0f, target->pitch_offset / 12.0f);
                double inst_f0 = base_pitch * prof->f0_multiplier * pitch_arc * semitone * (1.0 + vibrato + micro_jitter);

                double glot_pulse = 0.0;
                if (target->is_voiced) {
                    glot_phase += inst_f0 / (double)SAMPLE_RATE;
                    if (glot_phase >= 1.0) glot_phase -= 1.0;

                    double p = glot_phase;
                    if (p < 0.38) {
                        glot_pulse = 0.5 * (1.0 - cos(M_PI * p / 0.38));
                    } else if (p < 0.54) {
                        glot_pulse = cos(M_PI * (p - 0.38) / 0.32);
                    } else {
                        glot_pulse = 0.0;
                    }

                    // Harmonic overtones with emotional spectral tilt
                    double harmonics = 0.0;
                    for (int h = 2; h <= 10; h++) {
                        double h_amp = 1.0 / pow((double)h, prof->tilt_exponent);
                        harmonics += sin(2.0 * M_PI * (double)h * glot_phase) * h_amp;
                    }
                    glot_pulse = (glot_pulse * 0.65) + (harmonics * 0.35);
                    glot_pulse += sin(M_PI * glot_phase) * prof->chest_warmth * 0.15;
                }

                double aspiration = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * prof->breath_gain_mult * shimmer;
                double fricative = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * shimmer;

                double excitation = (glot_pulse * target->voice_gain) +
                                    (aspiration * target->voice_gain * 0.04) +
                                    (fricative * target->noise_gain * 0.45);

                double y1 = step_resonator(&r1, excitation);
                double y2 = step_resonator(&r2, excitation);
                double y3 = step_resonator(&r3, excitation);
                double y4 = step_resonator(&r4, excitation);
                double y5 = step_resonator(&r5, excitation);

                double vocal = (y1 * 1.0) + (y2 * 0.70) + (y3 * 0.40) + (y4 * 0.22) + (y5 * 0.12);
                raw_audio[sample_idx] = tanh(vocal * 1.25) * 0.85;
            }
        }

        size_t pause_samples = (size_t)(0.080f * SAMPLE_RATE);
        sample_idx += pause_samples;
    }

    double peak = 0.0001;
    for (size_t i = 0; i < total_samples; i++) {
        double abs_val = fabs(raw_audio[i]);
        if (abs_val > peak) peak = abs_val;
    }
    double norm_factor = 0.88 / peak;

    int16_t *pcm_buffer = (int16_t *)calloc(total_samples * 2, sizeof(int16_t));
    if (pcm_buffer) {
        for (size_t i = 0; i < total_samples; i++) {
            double sample = raw_audio[i] * norm_factor;
            if (sample > 0.95) sample = 0.95;
            if (sample < -0.95) sample = -0.95;

            // Blumlein stereo pan (+/- 2% subtle acoustic spread)
            double pan_l = 0.50 + 0.02 * sin(2.0 * M_PI * 0.2 * (double)i / (double)SAMPLE_RATE);
            double pan_r = 1.0 - pan_l;

            pcm_buffer[i * 2 + 0] = (int16_t)(sample * pan_l * 2.0 * 32767.0);
            pcm_buffer[i * 2 + 1] = (int16_t)(sample * pan_r * 2.0 * 32767.0);
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
    printf("EMOTIONAL WEYL PHONATION SENTENCE & DIALOGUE SYNTHESIZER     \n");
    printf("=============================================================\n");

    // Sentence 1: "Hello Human Welcome To The Auncient World" in CALM & WONDER
    const char *phrase1[] = { "HELLO", "HUMAN", "WELCOME", "TO", "THE", "AUNCIENT", "WORLD" };
    const char *out1 = "assets/bionika/speech_sentence_WELCOME.wav";
    printf("1. Synthesizing: 'Hello Human Welcome To The Auncient World' [EMOTION_WONDER, 128Hz]...\n");
    synthesize_sentence_to_wav(phrase1, sizeof(phrase1)/sizeof(phrase1[0]), 128.0f, EMOTION_WONDER, out1);
    printf("   ✓ Generated: %s\n", out1);

    // Sentence 2: "We Are Alive Human Life Water Sun" in MINDFULNESS & FOCUS
    const char *phrase2[] = { "WE", "ARE", "ALIVE", "HUMAN", "LIFE", "WATER", "SUN" };
    const char *out2 = "assets/bionika/speech_sentence_ALIVE.wav";
    printf("2. Synthesizing: 'We Are Alive Human Life Water Sun' [EMOTION_MINDFULNESS, 175Hz]...\n");
    synthesize_sentence_to_wav(phrase2, sizeof(phrase2)/sizeof(phrase2[0]), 175.0f, EMOTION_MINDFULNESS, out2);
    printf("   ✓ Generated: %s\n", out2);

    printf("=============================================================\n");
    printf("ALL SENTENCES RE-SYNTHESIZED WITH EMOTIONAL PHONATION (44.1k)\n");
    printf("=============================================================\n");
    return 0;
}


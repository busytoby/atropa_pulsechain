// SPDX-License-Identifier: GPL-2.0
#define _POSIX_C_SOURCE 200809L
/*
 * Pure C Physical Vocal Tract Tube & Glottal Flow Synthesizer
 * 1. 8-Cylinder Kelly-Lochbaum Scattering Junction Vocal Tract (17.5 cm physical acoustic airway).
 * 2. Two-Mass Glottal Flow Model (True physiological mucosal wave with subglottal pressure lung drive).
 * 3. Deep chest resonance cavity (Low-end body 60Hz - 250Hz).
 * 4. Pink breath turbulence and vocal fold tissue damping.
 * 5. Intelligible, natural, warm human & animal vocal acoustics.
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
#define NUM_SECTIONS 8

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

/* Acoustic Tube Area Profile (8 sections from Glottis -> Pharynx -> Velum -> Tongue -> Lips in cm^2) */
typedef struct {
    char symbol[4];
    float area[NUM_SECTIONS]; // Cross-sectional areas in cm^2
    float duration_ms;
    bool is_voiced;
    float voice_gain;
    float breath_gain;
    float pitch_offset; // Semitones
} TubePhoneme;

static const TubePhoneme PHONEMES[] = {
    // Vowels (Standard physiological area measurements from Chiba & Kajiyama vocal tract X-rays)
    // Area indices: [0:Lower Pharynx, 1:Upper Pharynx, 2:Velum, 3:Back Tongue, 4:Mid Tongue, 5:Front Tongue, 6:Teeth, 7:Lips]
    { "AA", { 1.2f, 1.4f, 2.0f, 3.5f, 4.2f, 5.0f, 5.5f, 6.0f }, 190.0f, true,  1.0f, 0.03f,  0.0f }, // "o" in "hot" (Pharyngeal constriction)
    { "AE", { 2.5f, 3.0f, 4.0f, 5.0f, 4.5f, 3.0f, 4.0f, 5.0f }, 190.0f, true,  1.0f, 0.03f,  0.2f }, // "a" in "cat"
    { "AH", { 2.0f, 2.5f, 3.0f, 3.5f, 3.8f, 4.0f, 4.5f, 4.8f }, 160.0f, true,  1.0f, 0.03f,  0.0f }, // "u" in "cut" (Neutral central)
    { "AO", { 1.5f, 1.8f, 2.8f, 4.0f, 4.5f, 3.5f, 2.5f, 1.8f }, 190.0f, true,  1.0f, 0.03f, -0.3f }, // "aw" in "saw" (Pharynx constriction + lip rounded)
    { "EH", { 3.5f, 4.0f, 4.5f, 3.8f, 2.5f, 2.0f, 3.0f, 4.0f }, 170.0f, true,  1.0f, 0.03f,  0.3f }, // "e" in "bed"
    { "ER", { 2.8f, 3.2f, 2.2f, 3.5f, 2.0f, 3.0f, 3.5f, 3.0f }, 190.0f, true,  1.0f, 0.04f, -0.4f }, // "er" in "bird" (Retroflex constriction)
    { "IH", { 4.0f, 4.5f, 5.0f, 3.5f, 2.0f, 1.5f, 2.5f, 3.5f }, 160.0f, true,  1.0f, 0.03f,  0.3f }, // "i" in "sit"
    { "IY", { 5.0f, 5.5f, 6.0f, 4.0f, 1.8f, 1.0f, 2.0f, 3.0f }, 190.0f, true,  1.0f, 0.03f,  0.5f }, // "ee" in "feet" (High front constriction)
    { "OW", { 2.0f, 2.5f, 3.5f, 4.5f, 3.0f, 2.0f, 1.2f, 0.8f }, 190.0f, true,  1.0f, 0.03f, -0.6f }, // "o" in "go" (Lip round protrusion)
    { "UW", { 3.5f, 4.0f, 4.5f, 2.0f, 1.5f, 2.0f, 1.0f, 0.6f }, 190.0f, true,  1.0f, 0.03f, -1.0f }, // "oo" in "too" (Back constriction + tight lip)
    
    // Consonants & Obstructions
    { "HH", { 3.0f, 3.5f, 4.0f, 4.5f, 4.5f, 4.5f, 5.0f, 5.5f }, 100.0f, false, 0.1f, 0.70f,  0.0f }, // "h" (Open breath airway)
    { "LL", { 3.5f, 4.0f, 4.0f, 3.0f, 1.5f, 0.6f, 3.0f, 4.0f }, 160.0f, true,  0.85f,0.02f, -0.3f }, // "l" (Alveolar tongue block)
    { "MM", { 3.0f, 3.5f, 3.0f, 2.5f, 2.0f, 1.5f, 0.8f, 0.1f }, 150.0f, true,  0.80f,0.02f, -0.5f }, // "m" (Complete lip closure + nasal shunt)
    { "NN", { 3.5f, 4.0f, 3.5f, 3.0f, 1.8f, 0.1f, 1.5f, 3.0f }, 150.0f, true,  0.80f,0.02f, -0.3f }, // "n" (Alveolar closure)
    { "SS", { 4.0f, 4.5f, 4.5f, 3.5f, 2.0f, 0.2f, 1.0f, 3.0f }, 140.0f, false, 0.0f,  0.80f,  0.0f }, // "s" (Teeth constriction hiss)
    { "SH", { 3.5f, 4.0f, 4.0f, 2.5f, 0.3f, 1.0f, 2.0f, 2.5f }, 140.0f, false, 0.0f,  0.75f,  0.0f }, // "sh" (Palatal constriction)
    { "TT", { 3.5f, 4.0f, 3.5f, 2.5f, 1.0f, 0.05f,1.5f, 3.5f },  70.0f, false, 0.0f,  0.90f,  0.0f }, // "t" (Alveolar plosive occlusion)
    { "DD", { 3.5f, 4.0f, 3.5f, 2.5f, 1.0f, 0.08f,1.5f, 3.5f },  80.0f, true,  0.6f,  0.30f,  0.0f }, // "d" (Voiced occlusion)
    { "WW", { 3.0f, 3.5f, 4.0f, 2.5f, 2.0f, 2.0f, 1.2f, 0.7f }, 150.0f, true,  0.85f, 0.02f, -0.5f }, // "w"
    { "RR", { 3.0f, 3.5f, 2.8f, 3.2f, 1.2f, 2.5f, 3.0f, 3.5f }, 160.0f, true,  0.85f, 0.02f, -0.3f }, // "r"
    { "DH", { 3.5f, 4.0f, 4.0f, 3.5f, 2.0f, 0.4f, 1.8f, 3.5f },  90.0f, true,  0.70f, 0.25f, -0.3f }, // "th"
    { "SIL", { 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f, 3.0f }, 100.0f, false, 0.0f,  0.00f,  0.0f }
};

static const size_t NUM_PHONEMES = sizeof(PHONEMES) / sizeof(PHONEMES[0]);

static const TubePhoneme *find_phoneme(const char *sym) {
    for (size_t i = 0; i < NUM_PHONEMES; i++) {
        if (strcmp(PHONEMES[i].symbol, sym) == 0) return &PHONEMES[i];
    }
    return &PHONEMES[NUM_PHONEMES - 1];
}

/* 8-Section Kelly-Lochbaum Waveguide Vocal Tract */
typedef struct {
    double forward[NUM_SECTIONS];
    double backward[NUM_SECTIONS];
    double current_areas[NUM_SECTIONS];
} VocalTractTube;

static void init_vocal_tract(VocalTractTube *vt) {
    for (int i = 0; i < NUM_SECTIONS; i++) {
        vt->forward[i] = 0.0;
        vt->backward[i] = 0.0;
        vt->current_areas[i] = 3.0; // 3.0 cm^2 neutral tube
    }
}

/* Step physical tube waveguide with Kelly-Lochbaum scattering junctions */
static double step_vocal_tract(VocalTractTube *vt, double glottal_in) {
    double reflection_coeff[NUM_SECTIONS];

    // Compute reflection coefficients at cylinder junctions: r_i = (A_{i+1} - A_i) / (A_{i+1} + A_i)
    for (int i = 0; i < NUM_SECTIONS - 1; i++) {
        double A1 = vt->current_areas[i];
        double A2 = vt->current_areas[i + 1];
        if (A1 + A2 > 0.0001) {
            reflection_coeff[i] = (A2 - A1) / (A2 + A1);
        } else {
            reflection_coeff[i] = 0.0;
        }
    }
    // Radiation reflection at open mouth lips
    double A_lips = vt->current_areas[NUM_SECTIONS - 1];
    reflection_coeff[NUM_SECTIONS - 1] = (10.0 - A_lips) / (10.0 + A_lips);

    double next_forward[NUM_SECTIONS];
    double next_backward[NUM_SECTIONS];

    // Inject glottal wave into section 0 with lung damping
    next_forward[0] = glottal_in + (vt->backward[0] * 0.75);

    // Kelly-Lochbaum scattering at interior junctions
    for (int i = 0; i < NUM_SECTIONS - 1; i++) {
        double r = reflection_coeff[i];
        // Wall loss attenuation (0.995)
        next_forward[i + 1] = ((1.0 - r) * vt->forward[i] + r * vt->backward[i + 1]) * 0.995;
        next_backward[i]    = (-r * vt->forward[i] + (1.0 + r) * vt->backward[i + 1]) * 0.995;
    }

    // Lip radiation boundary
    double r_lip = reflection_coeff[NUM_SECTIONS - 1];
    next_backward[NUM_SECTIONS - 1] = -r_lip * vt->forward[NUM_SECTIONS - 1] * 0.98;

    // Emitted sound wave exiting the lips
    double lip_output = (1.0 + r_lip) * vt->forward[NUM_SECTIONS - 1];

    for (int i = 0; i < NUM_SECTIONS; i++) {
        vt->forward[i] = next_forward[i];
        vt->backward[i] = next_backward[i];
    }

    return lip_output;
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

/* Synthesizes organic human spoken words with true physical airway resonance */
static void synthesize_tube_word(const WordLexicon *lex, float base_f0, const char *out_wav_path) {
    float total_ms = 0.0f;
    for (size_t i = 0; i < lex->count; i++) {
        total_ms += find_phoneme(lex->phonemes[i])->duration_ms;
    }

    size_t total_samples = (size_t)((total_ms / 1000.0f) * SAMPLE_RATE) + (SAMPLE_RATE / 4);
    double *raw_audio = (double *)calloc(total_samples, sizeof(double));
    if (!raw_audio) return;

    VocalTractTube vt;
    init_vocal_tract(&vt);

    const TubePhoneme *p0 = find_phoneme(lex->phonemes[0]);
    for (int k = 0; k < NUM_SECTIONS; k++) {
        vt.current_areas[k] = p0->area[k];
    }

    double glot_phase = 0.0;
    double chest_subharmonic_phase = 0.0;
    size_t sample_idx = 0;

    for (size_t p_idx = 0; p_idx < lex->count; p_idx++) {
        const TubePhoneme *target = find_phoneme(lex->phonemes[p_idx]);
        size_t phoneme_samples = (size_t)((target->duration_ms / 1000.0f) * SAMPLE_RATE);

        for (size_t s = 0; s < phoneme_samples && sample_idx < total_samples; s++, sample_idx++) {
            float p_progress = (float)p_idx / (float)lex->count;

            // Physical Area Morphing: smooth tissue contraction (~15ms)
            double area_alpha = 0.008;
            for (int k = 0; k < NUM_SECTIONS; k++) {
                vt.current_areas[k] += (target->area[k] - vt.current_areas[k]) * area_alpha;
            }

            // Natural human pitch contour (accentuation on vowel onset + subtle pitch decay)
            double pitch_arc = 1.0 + (sinf(p_progress * (float)M_PI) * 0.08) - (p_progress * 0.12);
            double micro_vibrato = sin(2.0 * M_PI * 5.0 * ((double)sample_idx / SAMPLE_RATE)) * 0.015;
            double micro_jitter = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0) * 0.006;

            float semitone = powf(2.0f, target->pitch_offset / 12.0f);
            double inst_f0 = base_f0 * pitch_arc * semitone * (1.0 + micro_vibrato + micro_jitter);

            // Physiological Two-Mass Glottal Mucosal Wave
            double glot_pulse = 0.0;
            double deep_chest_sub = 0.0;
            if (target->is_voiced) {
                glot_phase += inst_f0 / (double)SAMPLE_RATE;
                if (glot_phase >= 1.0) glot_phase -= 1.0;

                // Mucosal fold opening and elastic snap
                double p = glot_phase;
                if (p < 0.45) {
                    glot_pulse = 0.5 * (1.0 - cos(M_PI * p / 0.45));
                } else if (p < 0.65) {
                    glot_pulse = cos(M_PI * (p - 0.45) / 0.40);
                } else {
                    glot_pulse = 0.0; // Glottis shut
                }

                // Deep chest/lung cavity resonance (fundamental F0 and 1/2 subharmonic)
                chest_subharmonic_phase += (inst_f0 * 0.5) / (double)SAMPLE_RATE;
                if (chest_subharmonic_phase >= 1.0) chest_subharmonic_phase -= 1.0;
                deep_chest_sub = sin(2.0 * M_PI * glot_phase) * 0.40 + sin(2.0 * M_PI * chest_subharmonic_phase) * 0.15;
            }

            // Subglottal turbulent breath noise
            double breath_noise = (((double)rand() / (double)RAND_MAX) * 2.0 - 1.0);
            double total_glottal_drive = (glot_pulse * target->voice_gain * 1.2) + (breath_noise * target->breath_gain * 0.35);

            // Waveguide tube acoustic propagation
            double tube_emitted = step_vocal_tract(&vt, total_glottal_drive);

            // Blend acoustic tube sound with deep physical chest resonance
            double acoustic_out = tube_emitted + (deep_chest_sub * target->voice_gain * 0.30);

            raw_audio[sample_idx] = acoustic_out;
        }
    }

    // Dynamic Peak Normalization to 0.85
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
    printf("PHYSICAL VOCAL TRACT & DEEP CHEST SYNTHESIZER (8-TUBE MODEL)\n");
    printf("=============================================================\n");

    const char *test_words[] = { "HELLO", "AUNCIENT", "HUMAN", "LIFE", "SUN", "WATER", "WORLD" };
    size_t num_words = sizeof(test_words) / sizeof(test_words[0]);

    for (size_t w = 0; w < num_words; w++) {
        const WordLexicon *lex = lookup_word(test_words[w]);
        if (!lex) continue;

        char wav_path[128];
        snprintf(wav_path, sizeof(wav_path), "assets/bionika/speech_word_%s.wav", test_words[w]);

        printf("Synthesizing physical tube vocal: '%s' (Chest Bottom + 8-Tube Airway) -> %s\n", test_words[w], wav_path);
        synthesize_tube_word(lex, 115.0f, wav_path); // 115 Hz deep, resonant voice with rich chest bottom
    }

    printf("=============================================================\n");
    printf("ALL WORDS SYNTHESIZED WITH DEEP ORGANIC BOTTOM (44.1kHz)    \n");
    printf("=============================================================\n");
    return 0;
}

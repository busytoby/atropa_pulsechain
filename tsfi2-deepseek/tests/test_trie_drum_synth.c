#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include "tsfi_trie.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 8000
#define TOTAL_SAMPLES 16000 // 2 seconds of audio

// Simple 2nd-order State Variable Filter (SVF) for synth formants/drums
typedef struct {
    double f;
    double q;
    double low;
    double band;
    double high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
    r->q = 1.0 / Q;
    r->low = 0.0;
    r->band = 0.0;
    r->high = 0.0;
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band; // Bandpass
}

// Write standard 16-bit PCM WAV file header
static void write_wav_header(FILE *fp, uint32_t data_size) {
    uint32_t chunk_size = 36 + data_size;
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;
    uint16_t num_channels = 1;
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

int main(void) {
    printf("=== TSFi Trie-Triggered Drum & Synth Accumulator Test ===\n");

    // 1. Initialize Trie Router and insert instrument patterns
    tsfi_trie_node *trie_root = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie_root, "bd", "KICK_TRIGGER");
    tsfi_trie_insert(trie_root, "sd", "SNARE_TRIGGER");
    tsfi_trie_insert(trie_root, "hh", "HIHAT_TRIGGER");
    tsfi_trie_insert(trie_root, "lead", "SYNTH_TRIGGER");

    // 2. Setup Sequencer Steps (16 steps over 2 seconds, 125 ms per step)
    // Step: 0   1   2   3   4   5   6   7   8   9  10  11  12  13  14  15
    // Inst: bd  hh  sd  hh  bd  hh  sd  hh  bd  hh  sd  hh  bd  hh  sd  lead
    const char *sequence[16] = {
        "bd", "hh", "sd", "hh",
        "bd", "hh", "sd", "hh",
        "bd", "hh", "sd", "hh",
        "bd", "hh", "sd", "lead"
    };

    FILE *fp = fopen("trie_drum_synth.wav", "wb");
    if (!fp) {
        perror("Failed to open output WAV file");
        return 1;
    }
    write_wav_header(fp, TOTAL_SAMPLES * 2);

    int16_t *pcm_buffer = malloc(TOTAL_SAMPLES * sizeof(int16_t));
    memset(pcm_buffer, 0, TOTAL_SAMPLES * sizeof(int16_t));

    // Synthesizer voice states
    double kick_env = 0.0;
    double kick_pitch = 150.0;
    double snare_env = 0.0;
    double hihat_env = 0.0;
    double lead_env = 0.0;
    double lead_phase = 0.0;

    Resonator snare_res, hihat_res;
    resonator_init(&snare_res, 1000.0, 3.0);
    resonator_init(&hihat_res, 6000.0, 1.5);

    int step_duration = TOTAL_SAMPLES / 16;

    for (int t = 0; t < TOTAL_SAMPLES; t++) {
        int step = t / step_duration;
        int step_sample = t % step_duration;

        // Trigger Instrument at step boundary
        if (step_sample == 0) {
            const char *pattern = sequence[step];
            const char *trigger = tsfi_trie_lookup(trie_root, pattern);
            
            // Resolve path accumulator from Trie traversal to dynamically modulate envelope decay & filter resonance
            uint64_t acc = tsfi_trie_get_accumulator(trie_root, pattern);
            
            if (trigger) {
                if (strcmp(trigger, "KICK_TRIGGER") == 0) {
                    kick_env = 1.0;
                    // Modulate pitch sweep using accumulator value
                    kick_pitch = 150.0 + (acc % 100);
                    printf("[SEQ] Step %d: Triggered Bass Drum (Accumulator Mod Pitch: %.1f Hz)\n", step, kick_pitch);
                } else if (strcmp(trigger, "SNARE_TRIGGER") == 0) {
                    snare_env = 1.0;
                    // Modulate snare bandpass filter frequency based on accumulator
                    double snare_freq = 800.0 + (acc % 500);
                    resonator_init(&snare_res, snare_freq, 4.0);
                    printf("[SEQ] Step %d: Triggered Snare Drum (Accumulator Mod Filter Freq: %.1f Hz)\n", step, snare_freq);
                } else if (strcmp(trigger, "HIHAT_TRIGGER") == 0) {
                    hihat_env = 0.5;
                    printf("[SEQ] Step %d: Triggered Hi-Hat\n", step);
                } else if (strcmp(trigger, "SYNTH_TRIGGER") == 0) {
                    lead_env = 0.8;
                    printf("[SEQ] Step %d: Triggered Synth Lead (Accumulator: %lu)\n", step, acc);
                }
            }
        }

        // --- 1. Synthesize Bass Drum (Kick) ---
        double kick_out = 0.0;
        if (kick_env > 0.001) {
            // Fast downward pitch sweep
            double current_freq = 40.0 + (kick_pitch - 40.0) * kick_env;
            kick_out = sin(2.0 * M_PI * current_freq * (step_sample / (double)SAMPLE_RATE)) * kick_env;
            kick_env *= 0.985; // Quick exponential decay
        }

        // --- 2. Synthesize Snare Drum ---
        double snare_out = 0.0;
        if (snare_env > 0.001) {
            double noise = ((double)rand() / RAND_MAX - 0.5) * 2.0;
            // Bandpass filter noise for snare body resonance
            snare_out = resonator_tick(&snare_res, noise) * snare_env;
            snare_env *= 0.99; // Medium decay
        }

        // --- 3. Synthesize Hi-Hat ---
        double hihat_out = 0.0;
        if (hihat_env > 0.001) {
            double noise = ((double)rand() / RAND_MAX - 0.5) * 2.0;
            // Highpass filter noise for hi-hat sizzle
            hihat_out = (noise - resonator_tick(&hihat_res, noise)) * hihat_env;
            hihat_env *= 0.96; // Very fast decay
        }

        // --- 4. Synthesize Synth Lead ---
        double lead_out = 0.0;
        if (lead_env > 0.001) {
            // Sawtooth lead sound
            lead_phase += (220.0 / SAMPLE_RATE);
            if (lead_phase >= 1.0) lead_phase -= 1.0;
            lead_out = (2.0 * lead_phase - 1.0) * lead_env * 0.4;
            lead_env *= 0.995;
        }

        // Mix all audio channels together
        double mix = 0.6 * kick_out + 0.4 * snare_out + 0.3 * hihat_out + 0.4 * lead_out;

        // Scale and clip to 16-bit range
        double scaled = mix * 15000.0;
        if (scaled > 32767.0) scaled = 32767.0;
        if (scaled < -32768.0) scaled = -32768.0;
        pcm_buffer[t] = (int16_t)scaled;
    }

    fwrite(pcm_buffer, sizeof(int16_t), TOTAL_SAMPLES, fp);
    fclose(fp);
    free(pcm_buffer);
    tsfi_trie_destroy(trie_root);

    printf("[SEQ] Synthesis completed. Generated WAV at: trie_drum_synth.wav\n");
    return 0;
}

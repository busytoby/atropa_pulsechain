#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "libantigravity_interop.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define DURATION 10.0
#define NUM_SAMPLES ((size_t)(SAMPLE_RATE * DURATION))
#define CHANNELS 7  // 7 unique instruments

// Helper to write WAV header
void write_wav_header(FILE *f, uint32_t num_samples, uint32_t sample_rate, uint16_t channels) {
    uint32_t byte_rate = sample_rate * channels * 2;
    uint32_t data_chunk_size = num_samples * channels * 2;
    uint32_t file_size = 36 + data_chunk_size;
    
    fwrite("RIFF", 1, 4, f);
    fwrite(&file_size, 4, 1, f);
    fwrite("WAVE", 1, 4, f);
    fwrite("fmt ", 1, 4, f);
    
    uint32_t fmt_chunk_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t block_align = channels * 2;
    uint16_t bits_per_sample = 16;
    
    fwrite(&fmt_chunk_size, 4, 1, f);
    fwrite(&audio_format, 2, 1, f);
    fwrite(&channels, 2, 1, f);
    fwrite(&sample_rate, 4, 1, f);
    fwrite(&byte_rate, 4, 1, f);
    fwrite(&block_align, 2, 1, f);
    fwrite(&bits_per_sample, 2, 1, f);
    
    fwrite("data", 1, 4, f);
    fwrite(&data_chunk_size, 4, 1, f);
}

int main(void) {
    printf("Initializing GEMM Wave Synthesizer for 7 instruments...\n");
    
    // Allocate memory for 7 oscillator buffers
    float *oscillators = calloc(CHANNELS * NUM_SAMPLES, sizeof(float));
    float *mixed_buffer = calloc(CHANNELS * NUM_SAMPLES, sizeof(float));
    float *mixing_matrix = calloc(CHANNELS * CHANNELS, sizeof(float));
    
    if (!oscillators || !mixed_buffer || !mixing_matrix) {
        fprintf(stderr, "Out of memory.\n");
        return 1;
    }
    
    // 1. Synthesize 7 unique raw instrument waveforms
    // Bass notes (A minor)
    double bass_notes[4] = { 55.0, 65.4, 73.4, 82.4 };
    // Lead notes (A minor pentatonic)
    double lead_notes[5] = { 220.0, 261.6, 293.7, 329.6, 392.0 };
    
    for (size_t s = 0; s < NUM_SAMPLES; s++) {
        double t = (double)s / SAMPLE_RATE;
        int beat_idx = (int)(t * 2.0); // 120 BPM
        double sub_beat = fmod(t, 0.5) / 0.5;
        
        // Instrument 1: Sine (Sub-bass)
        double f1 = bass_notes[beat_idx % 4];
        oscillators[0 * NUM_SAMPLES + s] = (float)(sin(2.0 * M_PI * f1 * t) * exp(-4.0 * sub_beat));
        
        // Instrument 2: Triangle (Bass Pluck)
        double f2 = bass_notes[(beat_idx + 1) % 4] * 2.0;
        double tri = 2.0 * fabs(fmod(t * f2, 1.0) - 0.5) - 1.0;
        oscillators[1 * NUM_SAMPLES + s] = (float)(tri * exp(-6.0 * sub_beat));
        
        // Instrument 3: Square (Lead Melody)
        double f3 = lead_notes[(beat_idx * 3 + (int)(t * 4.0) % 5) % 5];
        double sq = (fmod(t * f3, 1.0) < 0.5) ? 1.0 : -1.0;
        oscillators[2 * NUM_SAMPLES + s] = (float)(sq * exp(-3.0 * fmod(t, 0.25) / 0.25));
        
        // Instrument 4: Sawtooth (Pads/Chords)
        double f4 = lead_notes[beat_idx % 5] * 0.5;
        double saw = 2.0 * (fmod(t * f4, 1.0)) - 1.0;
        oscillators[3 * NUM_SAMPLES + s] = (float)(saw * 0.3);
        
        // Instrument 5: FM Pluck
        double f5 = lead_notes[(beat_idx + 2) % 5] * 1.5;
        double mod = sin(2.0 * M_PI * f5 * 2.0 * t) * 1.5;
        oscillators[4 * NUM_SAMPLES + s] = (float)(sin(2.0 * M_PI * f5 * t + mod) * exp(-8.0 * sub_beat));
        
        // Instrument 6: AM Pad
        double f6 = lead_notes[(beat_idx + 3) % 5];
        double amp_mod = 0.5 * (1.0 + sin(2.0 * M_PI * 4.0 * t)); // 4Hz tremolo
        oscillators[5 * NUM_SAMPLES + s] = (float)(sin(2.0 * M_PI * f6 * t) * amp_mod * 0.4);
        
        // Instrument 7: White Noise (Percussive Hi-hats)
        double perc_env = exp(-80.0 * fmod(t, 0.25) / 0.25);
        float noise = ((float)(rand() % 2000) / 1000.0f) - 1.0f;
        oscillators[6 * NUM_SAMPLES + s] = noise * perc_env;
    }
    
    // 2. Set up Mixing Matrix for the 7 channels
    // Each row represents how the 7 raw oscillators mix into that output channel.
    // We will mix them into a stereo output (represented here by channels 0 & 1).
    for (int c = 0; c < CHANNELS; c++) {
        for (int src = 0; src < CHANNELS; src++) {
            if (c == 0) {
                // Left channel mix coefficients
                float left_weights[7] = { 0.4f, 0.2f, 0.3f, 0.1f, 0.2f, 0.2f, 0.1f };
                mixing_matrix[c * CHANNELS + src] = left_weights[src];
            } else if (c == 1) {
                // Right channel mix coefficients (slight panning differences)
                float right_weights[7] = { 0.4f, 0.3f, 0.2f, 0.1f, 0.3f, 0.1f, 0.2f };
                mixing_matrix[c * CHANNELS + src] = right_weights[src];
            } else {
                // Other channels silent or simple mono pass-through
                mixing_matrix[c * CHANNELS + src] = (c == src) ? 0.3f : 0.0f;
            }
        }
    }
    
    // 3. Perform GEMM matrix synthesis mixing
    printf("Synthesizing mixed matrix...\n");
    interop_gemm_synthesize(oscillators, mixing_matrix, mixed_buffer, CHANNELS, NUM_SAMPLES);
    
    // 4. Output to stereo WAV file
    FILE *wav = fopen("assets/soundtrack_gemm.wav", "wb");
    if (!wav) {
        perror("Failed to open WAV output file");
        free(oscillators);
        free(mixed_buffer);
        free(mixing_matrix);
        return 1;
    }
    
    // Write Stereo 16-bit WAV header
    write_wav_header(wav, NUM_SAMPLES, SAMPLE_RATE, 2);
    
    // Write interleaved stereo sample buffer
    for (size_t s = 0; s < NUM_SAMPLES; s++) {
        // Left channel mix (channel 0)
        float left = mixed_buffer[0 * NUM_SAMPLES + s];
        // Right channel mix (channel 1)
        float right = mixed_buffer[1 * NUM_SAMPLES + s];
        
        // Clamp values
        if (left > 1.0f) left = 1.0f;
        if (left < -1.0f) left = -1.0f;
        if (right > 1.0f) right = 1.0f;
        if (right < -1.0f) right = -1.0f;
        
        int16_t l_val = (int16_t)(left * 32767.0f);
        int16_t r_val = (int16_t)(right * 32767.0f);
        
        fwrite(&l_val, 2, 1, wav);
        fwrite(&r_val, 2, 1, wav);
    }
    
    fclose(wav);
    printf("✓ Successfully generated assets/soundtrack_gemm.wav\n");
    
    free(oscillators);
    free(mixed_buffer);
    free(mixing_matrix);
    return 0;
}

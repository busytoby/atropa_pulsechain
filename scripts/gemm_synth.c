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
#define DURATION 120.0
#define NUM_SAMPLES ((size_t)(SAMPLE_RATE * DURATION))
#define CHANNELS 7

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
    uint16_t audio_format = 1;
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

// ADSR envelope helper
float get_adsr(double t_note, double duration, float att, float dec, float sus, float rel) {
    if (t_note < 0.0) return 0.0f;
    if (t_note < att) {
        return (float)(t_note / att);
    }
    if (t_note < att + dec) {
        return (float)(1.0f - (1.0f - sus) * ((t_note - att) / dec));
    }
    if (t_note < duration - rel) {
        return sus;
    }
    if (t_note < duration) {
        double r_time = t_note - (duration - rel);
        return (float)(sus * (1.0f - (r_time / rel)));
    }
    return 0.0f;
}

int main(void) {
    printf("Initializing Enhanced GEMM Wave Synthesizer...\n");
    
    float *oscillators = calloc(CHANNELS * NUM_SAMPLES, sizeof(float));
    float *mixed_buffer = calloc(CHANNELS * NUM_SAMPLES, sizeof(float));
    float *mixing_matrix = calloc(CHANNELS * CHANNELS, sizeof(float));
    
    // Allocate simple feedback delay lines (0.25 seconds delay)
    size_t delay_samples = SAMPLE_RATE / 4; // 250ms
    float *delay_line = calloc(delay_samples, sizeof(float));
    size_t delay_ptr = 0;
    
    if (!oscillators || !mixed_buffer || !mixing_matrix || !delay_line) {
        fprintf(stderr, "Out of memory.\n");
        if (oscillators) free(oscillators);
        if (mixed_buffer) free(mixed_buffer);
        if (mixing_matrix) free(mixing_matrix);
        if (delay_line) free(delay_line);
        return 1;
    }
    
    // Melodic notes configurations
    double bass_notes[4] = { 55.0, 65.4, 73.4, 82.4 };  // A1, C2, D2, E2
    double lead_notes[5] = { 220.0, 261.6, 293.7, 329.6, 392.0 };  // A3, C4, D4, E4, G4
    
    // Evolving chord frequencies
    double chord_freqs[4][3] = {
        { 110.0, 130.8, 164.8 }, // Am
        { 87.3, 110.0, 130.8 },  // F
        { 98.0, 123.5, 146.8 },  // G
        { 82.4, 98.0, 123.5 }    // Em
    };
    
    for (size_t s = 0; s < NUM_SAMPLES; s++) {
        double t = (double)s / SAMPLE_RATE;
        double spb = 0.5; // 120 BPM -> 0.5s per beat
        int beat_idx = (int)(t / spb);
        double sub_beat = fmod(t, spb);
        
        // 1. Sine Sub-bass (with ADSR envelope)
        double f1 = bass_notes[beat_idx % 4];
        float env1 = get_adsr(sub_beat, spb, 0.05, 0.1, 0.7, 0.05);
        oscillators[0 * NUM_SAMPLES + s] = (float)(sin(2.0 * M_PI * f1 * t) * env1);
        
        // 2. Triangle Bass Pluck
        double f2 = bass_notes[(beat_idx + 1) % 4] * 2.0;
        double tri = 2.0 * fabs(fmod(t * f2, 1.0) - 0.5) - 1.0;
        float env2 = get_adsr(sub_beat, spb, 0.01, 0.2, 0.0, 0.01);
        oscillators[1 * NUM_SAMPLES + s] = (float)(tri * env2);
        
        // 3. Square Lead Melody (ADSR + vibrato modulation)
        double vibrato = 1.0 + 0.01 * sin(2.0 * M_PI * 6.0 * t);
        double f3 = lead_notes[(beat_idx * 3 + (int)(t * 4.0) % 5) % 5] * vibrato;
        double sq = (fmod(t * f3, 1.0) < 0.5) ? 1.0 : -1.0;
        float env3 = get_adsr(fmod(t, 0.25), 0.25, 0.02, 0.05, 0.5, 0.03);
        oscillators[2 * NUM_SAMPLES + s] = (float)(sq * env3);
        
        // 4. Sawtooth Polyphonic Pad (mixed chords)
        int chord_idx = (beat_idx / 4) % 4;
        double chord_saw = 0.0;
        for (int k = 0; k < 3; k++) {
            chord_saw += 2.0 * (fmod(t * chord_freqs[chord_idx][k], 1.0)) - 1.0;
        }
        oscillators[3 * NUM_SAMPLES + s] = (float)(chord_saw * 0.15f);
        
        // 5. FM Pluck with Echo/Delay
        double f5 = lead_notes[(beat_idx + 2) % 5] * 1.5;
        double mod = sin(2.0 * M_PI * f5 * 2.0 * t) * 1.5;
        float raw_pluck = (float)(sin(2.0 * M_PI * f5 * t + mod) * get_adsr(sub_beat, spb, 0.01, 0.15, 0.0, 0.05));
        
        float delayed_val = delay_line[delay_ptr];
        float input_val = raw_pluck + delayed_val * 0.4f;
        delay_line[delay_ptr] = input_val;
        delay_ptr = (delay_ptr + 1) % delay_samples;
        
        oscillators[4 * NUM_SAMPLES + s] = input_val;
        
        // 6. AM Pad
        double f6 = lead_notes[(beat_idx + 3) % 5];
        double amp_mod = 0.5 * (1.0 + sin(2.0 * M_PI * 4.0 * t));
        oscillators[5 * NUM_SAMPLES + s] = (float)(sin(2.0 * M_PI * f6 * t) * amp_mod * 0.35);
        
        // 7. White Noise Hats
        double hat_beat = fmod(t, 0.125);
        float noise = ((float)(rand() % 2000) / 1000.0f) - 1.0f;
        oscillators[6 * NUM_SAMPLES + s] = (float)(noise * get_adsr(hat_beat, 0.125, 0.005, 0.03, 0.0, 0.01) * 0.7);
    }
    
    for (int c = 0; c < CHANNELS; c++) {
        for (int src = 0; src < CHANNELS; src++) {
            if (c == 0) {
                float left_weights[7] = { 0.4f, 0.15f, 0.3f, 0.1f, 0.2f, 0.25f, 0.1f };
                mixing_matrix[c * CHANNELS + src] = left_weights[src];
            } else if (c == 1) {
                float right_weights[7] = { 0.4f, 0.25f, 0.15f, 0.1f, 0.25f, 0.15f, 0.2f };
                mixing_matrix[c * CHANNELS + src] = right_weights[src];
            } else {
                mixing_matrix[c * CHANNELS + src] = (c == src) ? 0.3f : 0.0f;
            }
        }
    }
    
    printf("Synthesizing enhanced mixed matrix...\n");
    interop_gemm_synthesize(oscillators, mixing_matrix, mixed_buffer, CHANNELS, NUM_SAMPLES);
    
    FILE *wav = fopen("assets/soundtrack_gemm.wav", "wb");
    if (!wav) {
        perror("Failed to open WAV output file");
        free(oscillators);
        free(mixed_buffer);
        free(mixing_matrix);
        free(delay_line);
        return 1;
    }
    
    write_wav_header(wav, NUM_SAMPLES, SAMPLE_RATE, 2);
    for (size_t s = 0; s < NUM_SAMPLES; s++) {
        float left = mixed_buffer[0 * NUM_SAMPLES + s];
        float right = mixed_buffer[1 * NUM_SAMPLES + s];
        
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
    free(delay_line);
    return 0;
}

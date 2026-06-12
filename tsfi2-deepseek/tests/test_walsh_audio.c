#include "tsfi_fourier.h"
#include "tsfi_c_math.h"
#include "tsfi_steiner_evi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "lau_memory.h"

struct wav_header {
    char riff[4];
    uint32_t size;
    char wave[4];
    char fmt[4];
    uint32_t fmt_size;
    uint16_t format;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t data_size;
};

// Melody played on the EVI: C4, D4, E4, G4 (2 semitones, 1 semitone, 3 semitones fingerings)
static const SteinerEVIState melody[4] = {
    { .breath_pressure = 0.9f, .valve1 = 0, .valve2 = 0, .valve3 = 0, .octave = 0, .base_pitch = 261.63f }, // C4
    { .breath_pressure = 0.8f, .valve1 = 1, .valve2 = 0, .valve3 = 1, .octave = 1, .base_pitch = 146.83f }, // D4 (approx)
    { .breath_pressure = 0.9f, .valve1 = 0, .valve2 = 1, .valve3 = 1, .octave = 1, .base_pitch = 164.81f }, // E4 (approx)
    { .breath_pressure = 0.7f, .valve1 = 0, .valve2 = 0, .valve3 = 0, .octave = 1, .base_pitch = 196.00f }  // G4 (approx)
};

int main() {
    printf("[AUDIO] Starting SCORTOS EVI Walsh Audio Synthesis...\n");

    // 1. Initialize Walsh Basis
    TSFiFourierBasis basis;
    tsfi_walsh_init_basis(&basis);

    // Create a 2-second audio buffer at 16000Hz, mono, 16-bit
    uint32_t sample_rate = 16000;
    uint32_t duration_sec = 2;
    uint32_t total_samples = sample_rate * duration_sec;
    int16_t *pcm_buffer = (int16_t*)malloc(total_samples * sizeof(int16_t));
    if (!pcm_buffer) {
        printf("[AUDIO] Error: Failed to allocate PCM buffer.\n");
        return 1;
    }

    uint32_t blocks_count = total_samples / TSFI_FOURIER_SAMPLES;
    for (uint32_t block = 0; block < blocks_count; block++) {
        float time_val = (float)block * TSFI_FOURIER_SAMPLES / sample_rate;
        
        // Select note from melody based on time (0.5 seconds per note)
        int note_idx = (int)(time_val / 0.5f) % 4;
        SteinerEVIState note = melody[note_idx];
        
        // Add a gentle vibrato using breath pressure modulation
        note.breath_pressure += 0.05f * sinf(time_val * 30.0f);
        
        float pitch = 0.0f;
        float volume = 0.0f;
        tsfi_steiner_evi_translate(&note, &pitch, &volume);

        TSFiFourierGlyph glyph;
        memset(&glyph, 0, sizeof(glyph));
        
        // Populate harmonics
        for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
            float amp = (1.0f / (n + 1)) * volume; // Scaled by EVI breath volume
            glyph.coeffs[n][0] = amp * sinf(pitch * (n + 1) * time_val);
            glyph.coeffs[n][1] = amp * cosf(pitch * (n + 1) * time_val);
            glyph.coeffs[n][2] = amp * cosf(pitch * (n + 1) * time_val);
            glyph.coeffs[n][3] = amp * sinf(pitch * (n + 1) * time_val);
        }

        float reconstructed[TSFI_FOURIER_SAMPLES * 2];
        tsfi_fourier_reconstruct_avx512(reconstructed, &basis, &glyph);

        // Map reconstructed contour to PCM samples
        for (int i = 0; i < TSFI_FOURIER_SAMPLES; i++) {
            uint32_t sample_idx = block * TSFI_FOURIER_SAMPLES + i;
            if (sample_idx < total_samples) {
                float val = reconstructed[i * 2 + 0] * 8000.0f;
                if (val > 32767.0f) val = 32767.0f;
                if (val < -32768.0f) val = -32768.0f;
                pcm_buffer[sample_idx] = (int16_t)val;
            }
        }
    }

    // 2. Write to WAV File
    FILE *fp = fopen("walsh_music.wav", "wb");
    if (!fp) {
        printf("[AUDIO] Error: Failed to open output file.\n");
        free(pcm_buffer);
        return 1;
    }

    struct wav_header header;
    memcpy(header.riff, "RIFF", 4);
    header.size = sizeof(struct wav_header) - 8 + total_samples * sizeof(int16_t);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmt_size = 16;
    header.format = 1; // PCM
    header.channels = 1;
    header.sample_rate = sample_rate;
    header.byte_rate = sample_rate * sizeof(int16_t);
    header.block_align = sizeof(int16_t);
    header.bits_per_sample = 16;
    memcpy(header.data, "data", 4);
    header.data_size = total_samples * sizeof(int16_t);

    fwrite(&header, sizeof(struct wav_header), 1, fp);
    fwrite(pcm_buffer, sizeof(int16_t), total_samples, fp);
    fclose(fp);

    printf("[AUDIO] SCORTOS EVI Walsh Audio Synthesis complete: walsh_music.wav (%d bytes)\n", header.size + 8);
    free(pcm_buffer);

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    return 0;
}

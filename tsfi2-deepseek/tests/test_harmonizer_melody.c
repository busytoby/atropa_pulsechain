#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "tsfi_valve.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 44100
#define NOTE_DURATION_SEC 0.5
#define NUM_NOTES 4
#define SAMPLE_BLOCK_SIZE (int)(SAMPLING_RATE * NOTE_DURATION_SEC)
#define TOTAL_SAMPLES (SAMPLE_BLOCK_SIZE * NUM_NOTES)

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

// Perfect fifth melody progression: fundamental (f1) and fifth (f2)
static const float melody_f1[NUM_NOTES] = { 261.63f, 329.63f, 349.23f, 392.00f }; // C4, E4, F4, G4
static const float melody_f2[NUM_NOTES] = { 392.00f, 493.88f, 523.25f, 587.33f }; // G4, B4, C5, D5

int main() {
    printf("=== TSFi2 Dual YI Harmonizer Melody Generator ===\n");
    printf("[MELODY] Synthesizing a %d-note perfect fifth progression...\n", NUM_NOTES);

    // Initialize dual YI parameterized physical valves
    TsfiValveTriode valve1;
    TsfiValveTriode valve2;
    tsfi_valve_init(&valve1, 90.0, 0.00002, 250.0, -1.2);
    tsfi_valve_init(&valve2, 110.0, 0.000025, 250.0, -1.6);
    
    // Configure based on YI bijections:
    valve1.is_tubular = 1;        // Coaxial tubular Type T (VOID)
    valve2.is_tubular = 0;        // Flat-plate Type S (LAU)
    valve1.R_plate = 120000.0;    // 120k load
    valve2.R_plate = 80000.0;     // 80k load
    valve1.Vp_tuner_offset = 1.5; // Fine plate tuner step offset
    valve2.Vp_tuner_offset = 1.5;

    float *input_grid1 = (float*)lau_memalign(64, SAMPLE_BLOCK_SIZE * sizeof(float));
    float *input_grid2 = (float*)lau_memalign(64, SAMPLE_BLOCK_SIZE * sizeof(float));
    float *output_plate1 = (float*)lau_memalign(64, SAMPLE_BLOCK_SIZE * sizeof(float));
    float *output_plate2 = (float*)lau_memalign(64, SAMPLE_BLOCK_SIZE * sizeof(float));
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));

    // Process notes one by one
    for (int note = 0; note < NUM_NOTES; note++) {
        float f1 = melody_f1[note];
        float f2 = melody_f2[note];
        
        printf("  Note %d: f1=%.2f Hz, f2=%.2f Hz\n", note, f1, f2);
        
        // Generate sine wave signals for this note block
        for (int i = 0; i < SAMPLE_BLOCK_SIZE; i++) {
            float t = (float)i / (float)SAMPLING_RATE;
            input_grid1[i] = 0.5f * sinf(2.0f * (float)M_PI * f1 * t);
            input_grid2[i] = 0.5f * sinf(2.0f * (float)M_PI * f2 * t);
        }

        // Process through the YI-parameterized regenerative valve stages
        tsfi_valve_process_regenerative(&valve1, input_grid1, output_plate1, SAMPLE_BLOCK_SIZE, -0.2, 1.0, 0.15);
        tsfi_valve_process_regenerative(&valve2, input_grid2, output_plate2, SAMPLE_BLOCK_SIZE, 0.2, 1.0, 0.15);

        // Get DC offsets
        float idle_vg[16] = {0};
        float idle_vp1[16] = {0};
        float idle_vp2[16] = {0};
        tsfi_valve_process_regenerative(&valve1, idle_vg, idle_vp1, 16, -0.2, 1.0, 0.15);
        tsfi_valve_process_regenerative(&valve2, idle_vg, idle_vp2, 16, 0.2, 1.0, 0.15);
        float dc1 = idle_vp1[0];
        float dc2 = idle_vp2[0];

        // Bijective cross-modulation blending
        for (int i = 0; i < SAMPLE_BLOCK_SIZE; i++) {
            float ac1 = output_plate1[i] - dc1;
            float ac2 = output_plate2[i] - dc2;
            float cross = ac1 * ac2 * 0.05f;
            float blended = ac1 + ac2 + cross;
            
            // Normalize and scale to 16-bit PCM
            float val = blended * 8000.0f; // Soft-level scaling
            if (val > 32767.0f) val = 32767.0f;
            if (val < -32768.0f) val = -32768.0f;
            
            uint32_t global_idx = note * SAMPLE_BLOCK_SIZE + i;
            pcm_buffer[global_idx] = (int16_t)val;
        }
    }

    // Write to WAV File
    FILE *fp = fopen("harmonizer_melody.wav", "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open harmonizer_melody.wav for writing\n");
        free(pcm_buffer);
        return 1;
    }

    struct wav_header header;
    memcpy(header.riff, "RIFF", 4);
    header.size = sizeof(struct wav_header) - 8 + TOTAL_SAMPLES * sizeof(int16_t);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmt_size = 16;
    header.format = 1; // PCM
    header.channels = 1;
    header.sample_rate = SAMPLING_RATE;
    header.byte_rate = SAMPLING_RATE * sizeof(int16_t);
    header.block_align = sizeof(int16_t);
    header.bits_per_sample = 16;
    memcpy(header.data, "data", 4);
    header.data_size = TOTAL_SAMPLES * sizeof(int16_t);

    fwrite(&header, sizeof(struct wav_header), 1, fp);
    fwrite(pcm_buffer, sizeof(int16_t), TOTAL_SAMPLES, fp);
    fclose(fp);

    printf("[SUCCESS] Harmonizer melody synthesized: harmonizer_melody.wav (%d bytes)\n", header.size + 8);

    lau_free(input_grid1);
    lau_free(input_grid2);
    lau_free(output_plate1);
    lau_free(output_plate2);
    free(pcm_buffer);

    return 0;
}

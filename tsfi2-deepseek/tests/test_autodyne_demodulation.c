#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "tsfi_valve.h"
#include "lau_memory.h"
#include "tsfi_fourier.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define FFT_SIZE 512
#define SAMPLING_RATE 96000.0 // Higher sampling rate to support RF carrier frequencies
#define TOTAL_SAMPLES 96000    // 1 second of audio

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

int main() {
    printf("=== TSFi2 Autodyne Demodulation & Beat Reception Verification ===\n");

    // Initialize the Audion configured as an autodyne receiver (self-oscillating detector)
    TsfiValveTriode audion;
    tsfi_valve_init(&audion, 120.0, 0.00003, 250.0, -1.5);
    audion.is_tubular = 1; // Tubular (Type T / VOID) geometry for high frequency stability

    float *rf_input = (float*)lau_memalign(64, TOTAL_SAMPLES * sizeof(float));
    float *plate_output = (float*)lau_memalign(64, TOTAL_SAMPLES * sizeof(float));
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));

    // Incoming RF undamped wave: carrier at 20 kHz (f_incoming)
    float f_incoming = 20000.0f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        rf_input[i] = 1.5f * sinf(2.0f * (float)M_PI * f_incoming * (float)i / (float)SAMPLING_RATE);
    }

    // Set autodyne feedback factor (beta = 0.90) to cause self-oscillation at local frequency f_local
    double beta_autodyne = 0.90;

    // Process through the autodyne receiver
    tsfi_valve_process_regenerative(&audion, rf_input, plate_output, TOTAL_SAMPLES, 0.2, 1.0, beta_autodyne);

    // Get the DC offset
    float idle_vg[16] = {0};
    float idle_vp[16] = {0};
    tsfi_valve_process_regenerative(&audion, idle_vg, idle_vp, 16, 0.2, 1.0, beta_autodyne);
    float dc_offset = idle_vp[0];

    // Fourier analysis on the first 512-sample low-frequency AC plate component to find the audio beat tone
    double *fft_real = (double*)lau_memalign(64, FFT_SIZE * sizeof(double));
    double *magnitude = (double*)lau_memalign(64, (FFT_SIZE / 2) * sizeof(double));

    for (int i = 0; i < FFT_SIZE; i++) {
        fft_real[i] = (double)(plate_output[i] - dc_offset);
    }

    for (int k = 0; k < FFT_SIZE / 2; k++) {
        double real_sum = 0.0;
        double imag_sum = 0.0;
        for (int n = 0; n < FFT_SIZE; n++) {
            double angle = 2.0 * M_PI * k * n / FFT_SIZE;
            real_sum += fft_real[n] * cos(angle);
            imag_sum -= fft_real[n] * sin(angle);
        }
        magnitude[k] = sqrt(real_sum * real_sum + imag_sum * imag_sum) / FFT_SIZE;
    }

    // Bin calculations
    double max_beat_mag = 0.0;
    int beat_bin = 0;
    for (int bin = 2; bin < 15; bin++) {
        if (magnitude[bin] > max_beat_mag) {
            max_beat_mag = magnitude[bin];
            beat_bin = bin;
        }
    }


    printf("[AUTODYNE] Detected peak beat frequency at %.1f Hz (mag: %.6f)\n", 
           (double)beat_bin * SAMPLING_RATE / FFT_SIZE, max_beat_mag);

    // Convert the AC output of the autodyne receiver to 16-bit PCM audio samples
    // Search for max AC amplitude to normalize
    float max_ac = 0.0001f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float ac = fabsf(plate_output[i] - dc_offset);
        if (ac > max_ac) max_ac = ac;
    }

    float norm_factor = 28000.0f / max_ac;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float val = (plate_output[i] - dc_offset) * norm_factor;
        if (val > 32767.0f) val = 32767.0f;
        if (val < -32768.0f) val = -32768.0f;
        pcm_buffer[i] = (int16_t)val;
    }

    // Write to WAV File
    FILE *fp = fopen("autodyne_sounding.wav", "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open autodyne_sounding.wav for writing\n");
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
    header.sample_rate = (uint32_t)SAMPLING_RATE;
    header.byte_rate = (uint32_t)SAMPLING_RATE * sizeof(int16_t);
    header.block_align = sizeof(int16_t);
    header.bits_per_sample = 16;
    memcpy(header.data, "data", 4);
    header.data_size = TOTAL_SAMPLES * sizeof(int16_t);

    fwrite(&header, sizeof(struct wav_header), 1, fp);
    fwrite(pcm_buffer, sizeof(int16_t), TOTAL_SAMPLES, fp);
    fclose(fp);

    printf("[AUTODYNE] Synthesized and exported reproducible sample: autodyne_sounding.wav (%d bytes)\n", 
           header.size + 8);

    // Verify that a valid low-frequency beat tone exists in the audio range
    assert(max_beat_mag > 0.001);

    lau_free(rf_input);
    lau_free(plate_output);
    free(pcm_buffer);
    lau_free(fft_real);
    lau_free(magnitude);

    printf("[SUCCESS] Autodyne Demodulation verified! Beat reception successfully recovered from RF carrier.\n");
    return 0;
}

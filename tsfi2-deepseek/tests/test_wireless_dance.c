#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "tsfi_valve.h"
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 96000.0 // 96 kHz sampling rate
#define DURATION_SEC 1.0      // 1 second duration
#define TOTAL_SAMPLES (int)(SAMPLING_RATE * DURATION_SEC)

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

// LCG random generator for atmospheric noise
static float generate_noise() {
    static uint32_t seed = 123456789;
    seed = seed * 1664525 + 1013904223;
    return ((float)(seed & 0xFFFF) / 65536.0f) * 2.0f - 1.0f;
}

int main() {
    printf("=== TSFi2 QST Issue #13: De Forest Wireless Dance Physical Simulation ===\n");

    // 1. Generate Input Audio (2XG Broadcast): Blended perfect fifth (C4 = 261.63 Hz, G4 = 392.00 Hz)
    float *tx_audio = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float t = (float)i / (float)SAMPLING_RATE;
        tx_audio[i] = 0.5f * sinf(2.0f * (float)M_PI * 261.63f * t) +
                      0.5f * sinf(2.0f * (float)M_PI * 392.00f * t);
    }

    // 2. AM Modulation onto an RF carrier of 24 kHz (to respect Nyquist at 96 kHz)
    float *rf_tx = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    float fc = 24000.0f; // 24 kHz RF carrier
    float mk = 0.8f;     // Modulation index
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float t = (float)i / (float)SAMPLING_RATE;
        rf_tx[i] = (1.0f + mk * tx_audio[i]) * sinf(2.0f * (float)M_PI * fc * t);
    }

    // 3. Propagation Channel: 40-mile attenuation & AWGN noise
    float *rf_rx = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    float A_path = 0.05f; // path attenuation factor
    float noise_amplitude = 0.01f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        rf_rx[i] = A_path * rf_tx[i] + noise_amplitude * generate_noise();
    }

    // 4. Regenerative Detector (Type T tubular triode)
    TsfiValveTriode detector;
    tsfi_valve_init(&detector, 120.0, 0.00003, 250.0, -1.5);
    detector.is_tubular = 1;

    float *demod_audio = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    double beta_regen = 0.85; // positive feedback below self-oscillation
    tsfi_valve_process_regenerative(&detector, rf_rx, demod_audio, TOTAL_SAMPLES, 0.2, 1.0, beta_regen);

    // Get detector DC idle level to filter it out
    float idle_in[16] = {0};
    float idle_out[16] = {0};
    tsfi_valve_process_regenerative(&detector, idle_in, idle_out, 16, 0.2, 1.0, beta_regen);
    float detector_dc = idle_out[0];

    // Remove DC offset and normalize demodulated audio before amplifier stages
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        demod_audio[i] -= detector_dc;
    }

    // 5. Two-Stage Cascade Amplifier
    // Stage 1: Coaxial tubular triode for voltage amplification
    TsfiValveTriode amp_stage1;
    tsfi_valve_init(&amp_stage1, 12.0, 0.00002, 200.0, -1.0);
    amp_stage1.is_tubular = 1;

    // Stage 2: Flat-plate triode for power amplification
    TsfiValveTriode amp_stage2;
    tsfi_valve_init(&amp_stage2, 8.0, 0.00003, 200.0, -2.0);
    amp_stage2.is_tubular = 0;

    float *amp1_out = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    float *amp2_out = (float*)malloc(TOTAL_SAMPLES * sizeof(float));

    // Process Stage 1
    tsfi_valve_process_regenerative(&amp_stage1, demod_audio, amp1_out, TOTAL_SAMPLES, 0.2, 1.0, 0.0);
    
    // Get Stage 1 DC to filter
    tsfi_valve_process_regenerative(&amp_stage1, idle_in, idle_out, 16, 0.2, 1.0, 0.0);
    float amp1_dc = idle_out[0];

    // Apply high-pass coupling between Stage 1 and Stage 2
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        amp1_out[i] -= amp1_dc;
    }

    // Process Stage 2
    tsfi_valve_process_regenerative(&amp_stage2, amp1_out, amp2_out, TOTAL_SAMPLES, 0.2, 1.0, 0.0);

    // Get Stage 2 DC to filter
    tsfi_valve_process_regenerative(&amp_stage2, idle_in, idle_out, 16, 0.2, 1.0, 0.0);
    float amp2_dc = idle_out[0];

    // Remove DC from Stage 2 output
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        amp2_out[i] -= amp2_dc;
    }

    // 6. Loud-Speaking Phone Transducer model
    // 2nd-order diaphragm: M_dia * z'' + R_dia * z' + K_dia * z = K_force * I_p
    double M_dia = 0.001;
    double R_dia = 0.15;
    double K_dia = 450.0;
    double K_force = 0.02;
    double dt = 1.0 / SAMPLING_RATE;

    float *acoustic_pressure = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    double z = 0.0;
    double v = 0.0;

    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        double current_ip = (double)amp2_out[i];
        double f_drive = K_force * current_ip;
        double accel = (f_drive - R_dia * v - K_dia * z) / M_dia;
        v += accel * dt;
        z += v * dt;
        acoustic_pressure[i] = (float)accel; // Acoustic pressure is proportional to acceleration
    }

    // 7. Write to WAV File (Normalized)
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));
    float max_val = 0.0001f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        if (fabsf(acoustic_pressure[i]) > max_val) {
            max_val = fabsf(acoustic_pressure[i]);
        }
    }

    float norm = 28000.0f / max_val;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float val = acoustic_pressure[i] * norm;
        if (val > 32767.0f) val = 32767.0f;
        if (val < -32768.0f) val = -32768.0f;
        pcm_buffer[i] = (int16_t)val;
    }

    FILE *fp = fopen("wireless_dance.wav", "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open wireless_dance.wav for writing\n");
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

    printf("[SUCCESS] Wireless Dance simulation completed.\n");
    printf("[ANALYSIS] Acoustic output WAV generated: wireless_dance.wav\n");

    // Clean up
    free(tx_audio);
    free(rf_tx);
    free(rf_rx);
    free(demod_audio);
    free(amp1_out);
    free(amp2_out);
    free(acoustic_pressure);
    free(pcm_buffer);

    return 0;
}

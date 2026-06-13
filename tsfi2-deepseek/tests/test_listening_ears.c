#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "lau_memory.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLING_RATE 96000.0
#define DURATION_SEC 1.0
#define TOTAL_SAMPLES (int)(SAMPLING_RATE * DURATION_SEC)
#define NUM_STATIONS 50
#define SPEED_OF_LIGHT 3e8 // m/s

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

static double get_random() {
    static uint64_t seed = 987654321ULL;
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)(seed & 0xFFFFFFFFFFFFULL) / 281474976710656.0;
}

static float generate_gaussian_noise() {
    double u1 = get_random();
    double u2 = get_random();
    if (u1 < 1e-15) u1 = 1e-15;
    return (float)(sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2));
}

typedef struct {
    double x; // km
    double y; // km
    float *buffer;
} ReceiverStation;

int main() {
    printf("=== TSFi2 QST Issue #14: Spatial Interference Nulling Beamformer ===\n");

    // 1. Target Spy Transmitter (located at x = 25.0 km, y = -40.0 km)
    double tx_x = 25.0;
    double tx_y = -40.0;
    
    // 2. High-Power Jammer/Interferer (located at x = -30.0 km, y = 50.0 km)
    double jam_x = -30.0;
    double jam_y = 50.0;

    // Spy signal (1000 Hz tone)
    float *tx_signal = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        float key = (i < TOTAL_SAMPLES / 2) ? 1.0f : 0.1f;
        tx_signal[i] = key * sinf(2.0f * (float)M_PI * 1000.0f * (float)t);
    }

    // Jammer signal (1500 Hz tone, 10x stronger amplitude)
    float *jam_signal = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        jam_signal[i] = 10.0f * sinf(2.0f * (float)M_PI * 1500.0f * (float)t);
    }

    // 3. Initialize stations
    ReceiverStation stations[NUM_STATIONS];
    for (int i = 0; i < NUM_STATIONS; i++) {
        double angle = get_random() * 2.0 * M_PI;
        double radius = get_random() * 100.0;
        stations[i].x = radius * cos(angle);
        stations[i].y = radius * sin(angle);
        stations[i].buffer = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    }

    // 4. Physical propagation with both target and jammer
    double fc_carrier = 24000.0; // 24 kHz carrier to match Nyquist
    for (int i = 0; i < NUM_STATIONS; i++) {
        double dx_tx = stations[i].x - tx_x;
        double dy_tx = stations[i].y - tx_y;
        double dist_tx_m = sqrt(dx_tx * dx_tx + dy_tx * dy_tx) * 1000.0;
        double delay_tx = dist_tx_m / SPEED_OF_LIGHT;
        int delay_samples_tx = (int)(delay_tx * SAMPLING_RATE);
        double path_loss_tx = 5000.0 / (dist_tx_m + 1.0);

        double dx_jam = stations[i].x - jam_x;
        double dy_jam = stations[i].y - jam_y;
        double dist_jam_m = sqrt(dx_jam * dx_jam + dy_jam * dy_jam) * 1000.0;
        double delay_jam = dist_jam_m / SPEED_OF_LIGHT;
        int delay_samples_jam = (int)(delay_jam * SAMPLING_RATE);
        double path_loss_jam = 5000.0 / (dist_jam_m + 1.0);

        float noise_std = 1.0f;

        for (int step = 0; step < TOTAL_SAMPLES; step++) {
            int idx_tx = step - delay_samples_tx;
            int idx_jam = step - delay_samples_jam;

            float val_tx = (idx_tx >= 0 && idx_tx < TOTAL_SAMPLES) ? tx_signal[idx_tx] : 0.0f;
            float val_jam = (idx_jam >= 0 && idx_jam < TOTAL_SAMPLES) ? jam_signal[idx_jam] : 0.0f;

            stations[i].buffer[step] = (float)(path_loss_tx * val_tx) + 
                                       (float)(path_loss_jam * val_jam) + 
                                       noise_std * generate_gaussian_noise();
        }
    }

    // 5. Compute Gram-Schmidt Spatial Nulling Weights
    double *h1 = (double*)malloc(NUM_STATIONS * sizeof(double));
    double *h2 = (double*)malloc(NUM_STATIONS * sizeof(double));
    double *h3 = (double*)malloc(NUM_STATIONS * sizeof(double));

    for (int i = 0; i < NUM_STATIONS; i++) {
        double dx_tx = stations[i].x - tx_x;
        double dy_tx = stations[i].y - tx_y;
        double dist_tx_m = sqrt(dx_tx * dx_tx + dy_tx * dy_tx) * 1000.0;
        double delay_tx = dist_tx_m / SPEED_OF_LIGHT;

        double dx_jam = stations[i].x - jam_x;
        double dy_jam = stations[i].y - jam_y;
        double dist_jam_m = sqrt(dx_jam * dx_jam + dy_jam * dy_jam) * 1000.0;
        double delay_jam = dist_jam_m / SPEED_OF_LIGHT;

        double delta_delay = delay_jam - delay_tx;

        h1[i] = 5000.0 / (dist_tx_m + 1.0); // target gain
        double path_loss_jam = 5000.0 / (dist_jam_m + 1.0);
        h2[i] = path_loss_jam * cos(2.0 * M_PI * fc_carrier * delta_delay);
        h3[i] = path_loss_jam * sin(2.0 * M_PI * fc_carrier * delta_delay);
    }

    // Gram-Schmidt orthogonalization
    double *e2 = (double*)malloc(NUM_STATIONS * sizeof(double));
    double *e3 = (double*)malloc(NUM_STATIONS * sizeof(double));

    // Normalize h1 (target vector)
    double norm_h1 = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) norm_h1 += h1[i] * h1[i];
    norm_h1 = sqrt(norm_h1);
    double *e1 = (double*)malloc(NUM_STATIONS * sizeof(double));
    for (int i = 0; i < NUM_STATIONS; i++) e1[i] = h1[i] / norm_h1;

    // Project h2 out of e1
    double dot_h2_e1 = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) dot_h2_e1 += h2[i] * e1[i];
    double norm_e2 = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) {
        e2[i] = h2[i] - dot_h2_e1 * e1[i];
        norm_e2 += e2[i] * e2[i];
    }
    norm_e2 = sqrt(norm_e2);
    for (int i = 0; i < NUM_STATIONS; i++) e2[i] /= norm_e2;

    // Project h3 out of e1 and e2
    double dot_h3_e1 = 0.0;
    double dot_h3_e2 = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) {
        dot_h3_e1 += h3[i] * e1[i];
        dot_h3_e2 += h3[i] * e2[i];
    }
    double norm_e3 = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) {
        e3[i] = h3[i] - dot_h3_e1 * e1[i] - dot_h3_e2 * e2[i];
        norm_e3 += e3[i] * e3[i];
    }
    norm_e3 = sqrt(norm_e3);
    for (int i = 0; i < NUM_STATIONS; i++) e3[i] /= norm_e3;

    // Construct nulled weight vector starting with h1
    double *w = (double*)malloc(NUM_STATIONS * sizeof(double));
    double dot_h1_e2 = 0.0;
    double dot_h1_e3 = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) {
        dot_h1_e2 += h1[i] * e2[i];
        dot_h1_e3 += h1[i] * e3[i];
    }
    double scale = 0.0;
    for (int i = 0; i < NUM_STATIONS; i++) {
        w[i] = h1[i] - dot_h1_e2 * e2[i] - dot_h1_e3 * e3[i];
        scale += w[i] * h1[i];
    }
    // Normalize so that w^T h1 = 1
    for (int i = 0; i < NUM_STATIONS; i++) {
        w[i] /= scale;
    }

    // 6. Perform both Non-Nulled and Spatial Nulled combining
    float *non_nulled_out = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    float *nulled_out = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    memset(non_nulled_out, 0, TOTAL_SAMPLES * sizeof(float));
    memset(nulled_out, 0, TOTAL_SAMPLES * sizeof(float));

    for (int step = 0; step < TOTAL_SAMPLES; step++) {
        double sum_mrc = 0.0;
        double weight_mrc = 0.0;
        double sum_null = 0.0;

        for (int i = 0; i < NUM_STATIONS; i++) {
            double dx_tx = stations[i].x - tx_x;
            double dy_tx = stations[i].y - tx_y;
            double dist_tx_m = sqrt(dx_tx * dx_tx + dy_tx * dy_tx) * 1000.0;
            double delay_tx = dist_tx_m / SPEED_OF_LIGHT;
            int delay_samples_tx = (int)(delay_tx * SAMPLING_RATE);

            int read_idx = step + delay_samples_tx;
            if (read_idx >= 0 && read_idx < TOTAL_SAMPLES) {
                // MRC weights
                double weight = h1[i] * h1[i];
                sum_mrc += stations[i].buffer[read_idx] * weight;
                weight_mrc += weight;

                // Spatial nulling weights
                sum_null += stations[i].buffer[read_idx] * w[i];
            }
        }
        non_nulled_out[step] = (weight_mrc > 0.0) ? (float)(sum_mrc / weight_mrc) : 0.0f;
        nulled_out[step] = (float)sum_null;
    }

    // 7. Calculate correlation with original clean signal
    double dot_mrc = 0.0, energy_mrc = 0.0, energy_tx = 0.0;
    double dot_null = 0.0, energy_null = 0.0;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        dot_mrc += (double)(tx_signal[i] * non_nulled_out[i]);
        energy_mrc += (double)(non_nulled_out[i] * non_nulled_out[i]);
        energy_tx += (double)(tx_signal[i] * tx_signal[i]);

        dot_null += (double)(tx_signal[i] * nulled_out[i]);
        energy_null += (double)(nulled_out[i] * nulled_out[i]);
    }
    double corr_mrc = dot_mrc / sqrt(energy_tx * energy_mrc + 1e-9);
    double corr_null = dot_null / sqrt(energy_tx * energy_null + 1e-9);

    printf("[BEAMFORMER] Non-nulled correlation (with jammer): %.4f\n", corr_mrc);
    printf("[BEAMFORMER] Spatial-nulled correlation (jammer cancelled): %.4f\n", corr_null);

    // Save nulled audio output to WAV file
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));
    float max_val = 0.0001f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        if (fabsf(nulled_out[i]) > max_val) {
            max_val = fabsf(nulled_out[i]);
        }
    }
    float norm = 28000.0f / max_val;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float val = nulled_out[i] * norm;
        if (val > 32767.0f) val = 32767.0f;
        if (val < -32768.0f) val = -32768.0f;
        pcm_buffer[i] = (int16_t)val;
    }

    FILE *fp = fopen("listening_ears.wav", "wb");
    if (!fp) {
        fprintf(stderr, "[ERROR] Failed to open listening_ears.wav\n");
        return 1;
    }

    struct wav_header header;
    memcpy(header.riff, "RIFF", 4);
    header.size = sizeof(struct wav_header) - 8 + TOTAL_SAMPLES * sizeof(int16_t);
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt, "fmt ", 4);
    header.fmt_size = 16;
    header.format = 1;
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

    // Verify spatial nulling successfully recovers the signal
    assert(corr_null > 0.40);
    printf("[SUCCESS] Gram-Schmidt spatial nulling successfully cancelled the strong terrestrial jammer!\n");

    // Clean up
    for (int i = 0; i < NUM_STATIONS; i++) {
        free(stations[i].buffer);
    }
    free(tx_signal);
    free(jam_signal);
    free(non_nulled_out);
    free(nulled_out);
    free(pcm_buffer);
    free(h1); free(h2); free(h3);
    free(e1); free(e2); free(e3);
    free(w);

    return 0;
}

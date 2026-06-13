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

// LCG random generator for noise and placement
static double get_random() {
    static uint64_t seed = 987654321ULL;
    seed = seed * 6364136223846793005ULL + 1442695040888963407ULL;
    return (double)(seed & 0xFFFFFFFFFFFFULL) / 281474976710656.0; // [0, 1]
}

static float generate_gaussian_noise() {
    // Box-Muller transform
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
    printf("=== TSFi2 QST Issue #14: One Thousand Listening Ears Network Simulation ===\n");

    // 1. Target Spy Transmitter (located at x = 25.0 km, y = -40.0 km)
    double tx_x = 25.0;
    double tx_y = -40.0;
    
    // Message signal: Morse code sequence of 1000 Hz tone keyed (0.5s ON, 0.5s OFF)
    float *tx_signal = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        float key = (i < TOTAL_SAMPLES / 2) ? 1.0f : 0.1f;
        tx_signal[i] = key * sinf(2.0f * (float)M_PI * 1000.0f * (float)t);
    }

    // 2. Initialize 50 decentralized stations dispersed across a 100 km radius
    ReceiverStation stations[NUM_STATIONS];
    for (int i = 0; i < NUM_STATIONS; i++) {
        double angle = get_random() * 2.0 * M_PI;
        double radius = get_random() * 100.0; // up to 100 km
        stations[i].x = radius * cos(angle);
        stations[i].y = radius * sin(angle);
        stations[i].buffer = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    }

    // 3. Physical propagation with delay, path loss, and high local static noise
    for (int i = 0; i < NUM_STATIONS; i++) {
        double dx = stations[i].x - tx_x;
        double dy = stations[i].y - tx_y;
        double distance_m = sqrt(dx * dx + dy * dy) * 1000.0; // convert to meters
        double delay_sec = distance_m / SPEED_OF_LIGHT;
        int delay_samples = (int)(delay_sec * SAMPLING_RATE);

        double path_loss = 5000.0 / (distance_m + 1.0); // simple inverse distance
        float noise_std = 1.0f; // High noise (buried signal)

        for (int step = 0; step < TOTAL_SAMPLES; step++) {
            int src_idx = step - delay_samples;
            float sig_val = (src_idx >= 0 && src_idx < TOTAL_SAMPLES) ? tx_signal[src_idx] : 0.0f;
            stations[i].buffer[step] = (float)(path_loss * sig_val) + noise_std * generate_gaussian_noise();
        }
    }

    // 4. Spatial Diversity Combining (Delay alignment & Summation)
    float *combined_output = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    memset(combined_output, 0, TOTAL_SAMPLES * sizeof(float));

    for (int step = 0; step < TOTAL_SAMPLES; step++) {
        double sum = 0.0;
        double weight_sum = 0.0;
        for (int i = 0; i < NUM_STATIONS; i++) {
            double dx = stations[i].x - tx_x;
            double dy = stations[i].y - tx_y;
            double distance_m = sqrt(dx * dx + dy * dy) * 1000.0;
            double delay_sec = distance_m / SPEED_OF_LIGHT;
            int delay_samples = (int)(delay_sec * SAMPLING_RATE);

            int read_idx = step + delay_samples;
            if (read_idx >= 0 && read_idx < TOTAL_SAMPLES) {
                // Maximum Ratio Combining weighting based on expected path strength
                double path_loss = 5000.0 / (distance_m + 1.0);
                double weight = path_loss * path_loss; 
                sum += stations[i].buffer[read_idx] * weight;
                weight_sum += weight;
            }
        }
        combined_output[step] = (weight_sum > 0.0) ? (float)(sum / weight_sum) : 0.0f;
    }

    // 5. Calculate Correlation (to verify signal recovery)
    double dot_product = 0.0;
    double energy_tx = 0.0;
    double energy_combined = 0.0;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        dot_product += (double)(tx_signal[i] * combined_output[i]);
        energy_tx += (double)(tx_signal[i] * tx_signal[i]);
        energy_combined += (double)(combined_output[i] * combined_output[i]);
    }
    double correlation = dot_product / sqrt(energy_tx * energy_combined + 1e-9);
    printf("[ANALYSIS] Spatial diversity recovery correlation: %.4f\n", correlation);

    // Write to WAV File
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));
    float max_val = 0.0001f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        if (fabsf(combined_output[i]) > max_val) {
            max_val = fabsf(combined_output[i]);
        }
    }
    float norm = 28000.0f / max_val;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float val = combined_output[i] * norm;
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

    // Verify recovery is successful (correlation should be significantly positive, e.g. > 0.05 despite massive noise)
    assert(correlation > 0.05);
    printf("[SUCCESS] Spy transmitter signal recovered from severe local noise floor!\n");

    // Clean up
    for (int i = 0; i < NUM_STATIONS; i++) {
        free(stations[i].buffer);
    }
    free(tx_signal);
    free(combined_output);
    free(pcm_buffer);

    return 0;
}

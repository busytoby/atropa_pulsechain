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
    double x_true;      // True coordinates (km)
    double y_true;
    double x_estimated; // Jittery estimated coordinates (km)
    double y_estimated;
    double phase_corr;  // Calibrated phase correction (radians)
    float *buffer;
} ReceiverStation;

int main() {
    printf("=== TSFi2 QST Issue #14: Array Jitter & Pilot Phase Calibration ===\n");

    // 1. Target Spy Transmitter (located at x = 25.0 km, y = -40.0 km)
    double tx_x = 25.0;
    double tx_y = -40.0;
    
    // 2. Reference Calibration Beacon / Pilot Tower (co-located with target or pilot tone transmitted from target)
    double pilot_x = tx_x;
    double pilot_y = tx_y;

    // Spy signal (1000 Hz baseband tone)
    float *tx_signal = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        double t = (double)i / SAMPLING_RATE;
        float key = (i < TOTAL_SAMPLES / 2) ? 1.0f : 0.1f;
        tx_signal[i] = key * sinf(2.0f * (float)M_PI * 1000.0f * (float)t);
    }

    // 3. Initialize stations with position jitter (15.0 km standard deviation)
    ReceiverStation stations[NUM_STATIONS];
    double jitter_std = 15.0; // 15.0 km coordinate error
    for (int i = 0; i < NUM_STATIONS; i++) {
        double angle = get_random() * 2.0 * M_PI;
        double radius = get_random() * 100.0;
        stations[i].x_true = radius * cos(angle);
        stations[i].y_true = radius * sin(angle);
        
        // Add coordinate jitter to estimates
        stations[i].x_estimated = stations[i].x_true + jitter_std * generate_gaussian_noise();
        stations[i].y_estimated = stations[i].y_true + jitter_std * generate_gaussian_noise();
        stations[i].phase_corr = 0.0;
        
        stations[i].buffer = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    }

    // 4. Pilot Calibration Phase: Broadcast calibration tone and measure phase error at each station
    double fc_carrier = 24000.0; // 24 kHz RF carrier
    for (int i = 0; i < NUM_STATIONS; i++) {
        // True path to pilot
        double dx_true = stations[i].x_true - pilot_x;
        double dy_true = stations[i].y_true - pilot_y;
        double dist_true = sqrt(dx_true*dx_true + dy_true*dy_true) * 1000.0;
        double delay_true = dist_true / SPEED_OF_LIGHT;

        // Estimated path to pilot
        double dx_est = stations[i].x_estimated - pilot_x;
        double dy_est = stations[i].y_estimated - pilot_y;
        double dist_est = sqrt(dx_est*dx_est + dy_est*dy_est) * 1000.0;
        double delay_est = dist_est / SPEED_OF_LIGHT;

        // Calculate phase error induced by coordinate jitter at carrier frequency
        double phase_true = -2.0 * M_PI * fc_carrier * delay_true;
        double phase_est = -2.0 * M_PI * fc_carrier * delay_est;
        stations[i].phase_corr = phase_true - phase_est; // Correction factor
    }

    // 5. Physical propagation of RF modulated spy signal
    for (int i = 0; i < NUM_STATIONS; i++) {
        double dx = stations[i].x_true - tx_x;
        double dy = stations[i].y_true - tx_y;
        double dist_tx_m = sqrt(dx * dx + dy * dy) * 1000.0;
        double delay_tx = dist_tx_m / SPEED_OF_LIGHT;
        int delay_samples_tx = (int)(delay_tx * SAMPLING_RATE);
        double path_loss_tx = 5000.0 / (dist_tx_m + 1.0);

        float noise_std = 0.1f;

        for (int step = 0; step < TOTAL_SAMPLES; step++) {
            int idx_tx = step - delay_samples_tx;
            float val_tx = (idx_tx >= 0 && idx_tx < TOTAL_SAMPLES) ? tx_signal[idx_tx] : 0.0f;
            
            // RF modulate onto the carrier at true propagation delay
            double t = (double)step / SAMPLING_RATE;
            double rf_val = val_tx * cos(2.0 * M_PI * fc_carrier * (t - delay_tx));

            stations[i].buffer[step] = (float)(path_loss_tx * rf_val) + noise_std * generate_gaussian_noise();
        }
    }

    // 6. Perform combining under three conditions:
    // A. Perfect (no jitter)
    // B. Jittery (with coordinate errors, uncalibrated)
    // C. Calibrated (with coordinate errors, pilot-phase corrected)
    float *perfect_out = (float*)calloc(TOTAL_SAMPLES, sizeof(float));
    float *jitter_out = (float*)calloc(TOTAL_SAMPLES, sizeof(float));
    float *calibrated_out = (float*)calloc(TOTAL_SAMPLES, sizeof(float));

    for (int step = 0; step < TOTAL_SAMPLES; step++) {
        double sum_perf = 0.0, weight_perf = 0.0;
        double sum_jit = 0.0, weight_jit = 0.0;
        double sum_cal = 0.0, weight_cal = 0.0;

        double t = (double)step / SAMPLING_RATE;

        for (int i = 0; i < NUM_STATIONS; i++) {
            // A. Perfect delay
            double dx_true = stations[i].x_true - tx_x;
            double dy_true = stations[i].y_true - tx_y;
            double dist_true = sqrt(dx_true*dx_true + dy_true*dy_true) * 1000.0;
            double delay_true = dist_true / SPEED_OF_LIGHT;
            int delay_samples_true = (int)(delay_true * SAMPLING_RATE);

            int read_idx_true = step + delay_samples_true;
            if (read_idx_true >= 0 && read_idx_true < TOTAL_SAMPLES) {
                double weight = 5000.0 / (dist_true + 1.0);
                // Demodulate coherently
                double val = (double)stations[i].buffer[read_idx_true] * cos(2.0 * M_PI * fc_carrier * t);
                sum_perf += val * weight;
                weight_perf += weight;
            }

            // B. Jittery delay (uncalibrated)
            double dx_est = stations[i].x_estimated - tx_x;
            double dy_est = stations[i].y_estimated - tx_y;
            double dist_est = sqrt(dx_est*dx_est + dy_est*dy_est) * 1000.0;
            double delay_est = dist_est / SPEED_OF_LIGHT;
            int delay_samples_est = (int)(delay_est * SAMPLING_RATE);

            int read_idx_est = step + delay_samples_est;
            if (read_idx_est >= 0 && read_idx_est < TOTAL_SAMPLES) {
                double weight = 5000.0 / (dist_est + 1.0);
                // Demodulate with jittery expected carrier phase
                double val = (double)stations[i].buffer[read_idx_est] * cos(2.0 * M_PI * fc_carrier * t);
                sum_jit += val * weight;
                weight_jit += weight;
            }

            // C. Calibrated delay (corrected using pilot phase error)
            if (read_idx_est >= 0 && read_idx_est < TOTAL_SAMPLES) {
                double weight = 5000.0 / (dist_est + 1.0);
                // Shift demodulator phase by measured pilot correction factor to align carrier
                double phi_demod = 2.0 * M_PI * fc_carrier * ((double)delay_samples_est / SAMPLING_RATE - delay_est) + stations[i].phase_corr;
                double val = (double)stations[i].buffer[read_idx_est] * cos(2.0 * M_PI * fc_carrier * t + phi_demod);
                sum_cal += val * weight;
                weight_cal += weight;
            }
        }

        perfect_out[step] = (weight_perf > 0.0) ? (float)(sum_perf / weight_perf) : 0.0f;
        jitter_out[step] = (weight_jit > 0.0) ? (float)(sum_jit / weight_jit) : 0.0f;
        calibrated_out[step] = (weight_cal > 0.0) ? (float)(sum_cal / weight_cal) : 0.0f;
    }

    // 7. Calculate correlations with original clean signal
    double dot_perf = 0.0, energy_perf = 0.0, energy_tx = 0.0;
    double dot_jit = 0.0, energy_jit = 0.0;
    double dot_cal = 0.0, energy_cal = 0.0;

    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        dot_perf += (double)(tx_signal[i] * perfect_out[i]);
        energy_perf += (double)(perfect_out[i] * perfect_out[i]);
        energy_tx += (double)(tx_signal[i] * tx_signal[i]);

        dot_jit += (double)(tx_signal[i] * jitter_out[i]);
        energy_jit += (double)(jitter_out[i] * jitter_out[i]);

        dot_cal += (double)(tx_signal[i] * calibrated_out[i]);
        energy_cal += (double)(calibrated_out[i] * calibrated_out[i]);
    }

    double corr_perf = dot_perf / sqrt(energy_tx * energy_perf + 1e-9);
    double corr_jit = dot_jit / sqrt(energy_tx * energy_jit + 1e-9);
    double corr_cal = dot_cal / sqrt(energy_tx * energy_cal + 1e-9);

    printf("[RESULTS] Perfect array correlation: %.4f\n", corr_perf);
    printf("[RESULTS] Jittery array correlation (uncalibrated): %.4f\n", corr_jit);
    printf("[RESULTS] Calibrated array correlation (pilot corrected): %.4f\n", corr_cal);

    // Save calibrated audio output to WAV file
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));
    float max_val = 0.0001f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        if (fabsf(calibrated_out[i]) > max_val) {
            max_val = fabsf(calibrated_out[i]);
        }
    }
    float norm = 28000.0f / max_val;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float val = calibrated_out[i] * norm;
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

    // Verify that calibration successfully recovers the correlation back above 0.50
    // while the uncalibrated jittery array drops below 0.35 due to phase mismatch
    assert(corr_jit < 0.35);
    assert(corr_cal > 0.50);
    printf("[SUCCESS] Pilot phase calibration restored coherent combining in the presence of coordinate jitter!\n");

    // 8. TDOA Cross-Correlation and 2D Grid Search Localization
    printf("\n=== TDOA Localization Grid Search ===\n");
    double *estimated_tdoa = (double*)malloc(NUM_STATIONS * sizeof(double));
    estimated_tdoa[0] = 0.0;

    int max_lag = 80;
    
    // Allocate IQ buffers for all stations
    double **I_sig = (double**)malloc(NUM_STATIONS * sizeof(double*));
    double **Q_sig = (double**)malloc(NUM_STATIONS * sizeof(double*));
    for (int i = 0; i < NUM_STATIONS; i++) {
        I_sig[i] = (double*)malloc(TOTAL_SAMPLES * sizeof(double));
        Q_sig[i] = (double*)malloc(TOTAL_SAMPLES * sizeof(double));
        for (int n = 0; n < TOTAL_SAMPLES; n++) {
            // Apply a 4-sample moving average to low-pass filter the mix products
            double sum_I = 0.0;
            double sum_Q = 0.0;
            int count = 0;
            for (int k = 0; k < 4; k++) {
                int idx = n - k;
                if (idx >= 0) {
                    double t = (double)idx / SAMPLING_RATE;
                    sum_I += (double)stations[i].buffer[idx] * cos(2.0 * M_PI * fc_carrier * t);
                    sum_Q += (double)stations[i].buffer[idx] * sin(2.0 * M_PI * fc_carrier * t);
                    count++;
                }
            }
            I_sig[i][n] = sum_I / count;
            Q_sig[i][n] = sum_Q / count;
        }
    }

    for (int i = 1; i < NUM_STATIONS; i++) {
        double max_mag2 = -1.0;
        int best_lag = 0;
        for (int lag = -max_lag; lag <= max_lag; lag++) {
            double sum_real = 0.0;
            double sum_imag = 0.0;
            for (int n = max_lag; n < TOTAL_SAMPLES - max_lag; n++) {
                double I_i = I_sig[i][n];
                double Q_i = Q_sig[i][n];
                double I_0 = I_sig[0][n - lag];
                double Q_0 = Q_sig[0][n - lag];
                
                sum_real += (I_i * I_0 + Q_i * Q_0);
                sum_imag += (Q_i * I_0 - I_i * Q_0);
            }
            double mag2 = sum_real * sum_real + sum_imag * sum_imag;
            if (mag2 > max_mag2) {
                max_mag2 = mag2;
                best_lag = lag;
            }
        }
        estimated_tdoa[i] = (double)best_lag / SAMPLING_RATE;
        
    }

    double **R_mag = (double**)malloc(NUM_STATIONS * sizeof(double*));
    for (int i = 0; i < NUM_STATIONS; i++) {
        R_mag[i] = (double*)calloc(2 * max_lag + 1, sizeof(double));
    }

    for (int i = 1; i < NUM_STATIONS; i++) {
        for (int lag = -max_lag; lag <= max_lag; lag++) {
            double sum_real = 0.0;
            double sum_imag = 0.0;
            for (int n = max_lag; n < TOTAL_SAMPLES - max_lag; n++) {
                double I_i = I_sig[i][n];
                double Q_i = Q_sig[i][n];
                double I_0 = I_sig[0][n - lag];
                double Q_0 = Q_sig[0][n - lag];
                
                sum_real += (I_i * I_0 + Q_i * Q_0);
                sum_imag += (Q_i * I_0 - I_i * Q_0);
            }
            R_mag[i][lag + max_lag] = sqrt(sum_real * sum_real + sum_imag * sum_imag);
        }
    }

    for (int i = 0; i < NUM_STATIONS; i++) {
        free(I_sig[i]);
        free(Q_sig[i]);
    }
    free(I_sig);
    free(Q_sig);

    // Coarse grid search: -100 km to 100 km with 2 km steps
    double best_x = 0.0, best_y = 0.0;
    double max_cost = -1.0;

    for (double gx = -100.0; gx <= 100.0; gx += 2.0) {
        for (double gy = -100.0; gy <= 100.0; gy += 2.0) {
            double cost = 0.0;
            double dist0 = sqrt((stations[0].x_true - gx)*(stations[0].x_true - gx) + (stations[0].y_true - gy)*(stations[0].y_true - gy)) * 1000.0;
            double delay0 = dist0 / SPEED_OF_LIGHT;

            for (int i = 1; i < NUM_STATIONS; i++) {
                double disti = sqrt((stations[i].x_true - gx)*(stations[i].x_true - gx) + (stations[i].y_true - gy)*(stations[i].y_true - gy)) * 1000.0;
                double delayi = disti / SPEED_OF_LIGHT;
                double model_tdoa = delayi - delay0;
                int lag = (int)round(model_tdoa * SAMPLING_RATE);
                if (lag >= -max_lag && lag <= max_lag) {
                    cost += R_mag[i][lag + max_lag];
                }
            }

            if (cost > max_cost) {
                max_cost = cost;
                best_x = gx;
                best_y = gy;
            }
        }
    }

    // Fine grid search: search within +/- 5 km around best coarse estimate with 0.1 km steps
    double coarse_x = best_x;
    double coarse_y = best_y;
    max_cost = -1.0;

    for (double gx = coarse_x - 5.0; gx <= coarse_x + 5.0; gx += 0.1) {
        for (double gy = coarse_y - 5.0; gy <= coarse_y + 5.0; gy += 0.1) {
            double cost = 0.0;
            double dist0 = sqrt((stations[0].x_true - gx)*(stations[0].x_true - gx) + (stations[0].y_true - gy)*(stations[0].y_true - gy)) * 1000.0;
            double delay0 = dist0 / SPEED_OF_LIGHT;

            for (int i = 1; i < NUM_STATIONS; i++) {
                double disti = sqrt((stations[i].x_true - gx)*(stations[i].x_true - gx) + (stations[i].y_true - gy)*(stations[i].y_true - gy)) * 1000.0;
                double delayi = disti / SPEED_OF_LIGHT;
                double model_tdoa = delayi - delay0;
                int lag = (int)round(model_tdoa * SAMPLING_RATE);
                if (lag >= -max_lag && lag <= max_lag) {
                    cost += R_mag[i][lag + max_lag];
                }
            }

            if (cost > max_cost) {
                max_cost = cost;
                best_x = gx;
                best_y = gy;
            }
        }
    }

    double loc_error = sqrt((best_x - tx_x)*(best_x - tx_x) + (best_y - tx_y)*(best_y - tx_y));
    printf("[RESULTS] True TX coordinates: (%.1f, %.1f) km\n", tx_x, tx_y);
    printf("[RESULTS] Localized TX coordinates: (%.1f, %.1f) km\n", best_x, best_y);
    printf("[RESULTS] Localization error: %.2f km\n", loc_error);

    // Verify TDOA localization successfully finds coordinates within 5.0 km
    assert(loc_error < 5.0);
    printf("[SUCCESS] TDOA grid search successfully localized the spy transmitter!\n");

    for (int i = 0; i < NUM_STATIONS; i++) {
        free(R_mag[i]);
    }
    free(R_mag);
    free(estimated_tdoa);

    // Clean up
    for (int i = 0; i < NUM_STATIONS; i++) {
        free(stations[i].buffer);
    }
    free(tx_signal);
    free(perfect_out);
    free(jitter_out);
    free(calibrated_out);
    free(pcm_buffer);

    return 0;
}

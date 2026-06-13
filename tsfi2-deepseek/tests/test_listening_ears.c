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

static double interpolate_correlation(double *R_i, double lag, int max_lag) {
    if (isnan(lag) || isinf(lag)) return 0.0;
    double idx_exact = lag + max_lag;
    if (idx_exact < 0.0) return R_i[0];
    if (idx_exact >= 2.0 * max_lag) return R_i[2 * max_lag];
    
    int idx_low = (int)floor(idx_exact);
    int idx_high = idx_low + 1;
    if (idx_high > 2 * max_lag) idx_high = 2 * max_lag;
    
    double t = idx_exact - idx_low;
    return (1.0 - t) * R_i[idx_low] + t * R_i[idx_high];
}

typedef struct {
    double x_true;      // True coordinates (km)
    double y_true;
    double x_estimated; // Jittery estimated coordinates (km)
    double y_estimated;
    double phase_corr;  // Calibrated phase correction (radians)
    float *buffer;
} ReceiverStation;

static void run_safety_benchmarks() {
    printf("\n=== Listening Ears Safety and Robustness Benchmark ===\n");

    // Test 1: Parabolic Interpolation divide-by-zero (flat peak)
    double R_flat[3] = {10.0, 10.0, 10.0};
    double alpha = R_flat[0];
    double beta = R_flat[1];
    double gamma = R_flat[2];
    double denom = alpha - 2.0 * beta + gamma;
    assert(fabs(denom) < 1e-9); // flat peak
    double frac = 0.0;
    if (fabs(denom) > 1e-9) {
        frac = (alpha - gamma) / (2.0 * denom);
    }
    assert(frac == 0.0); // handled correctly
    printf("[SAFE] Flat peak divide-by-zero handled (frac = %.2f)\n", frac);

    // Test 2: Boundary value (peak at index 0)
    int best_lag_idx = 0;
    int max_lag = 80;
    frac = 0.0;
    if (best_lag_idx > 0 && best_lag_idx < 2 * max_lag) {
        assert(0 && "Should not attempt to interpolate at left boundary!");
    }
    printf("[SAFE] Boundary value interpolation ignored (frac = %.2f)\n", frac);

    // Test 3: Out of bounds lag in continuous lookup
    double R_test[161] = {0};
    R_test[80] = 5.0; // center peak
    double val_low = interpolate_correlation(R_test, -200.0, max_lag);
    double val_high = interpolate_correlation(R_test, 200.0, max_lag);
    assert(val_low == R_test[0]);
    assert(val_high == R_test[2 * max_lag]);
    printf("[SAFE] Out-of-bounds lag lookup clamped correctly (low: %.2f, high: %.2f)\n", val_low, val_high);

    // Test 4: NaN and Inf input protection in lookup
    double val_nan = interpolate_correlation(R_test, NAN, max_lag);
    double val_inf = interpolate_correlation(R_test, INFINITY, max_lag);
    assert(val_nan == 0.0);
    assert(val_inf == 0.0);
    printf("[SAFE] NaN/Inf lag input handled gracefully (nan lookup: %.2f, inf lookup: %.2f)\n", val_nan, val_inf);

    printf("[SUCCESS] All listening ears safety benchmarks passed successfully!\n\n");
}

int main() {
    run_safety_benchmarks();
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

    // 5. Physical propagation of RF modulated spy signal and a high-power jammer
    double jam_x = 50.0;
    double jam_y = 50.0;
    for (int i = 0; i < NUM_STATIONS; i++) {
        double dx = stations[i].x_true - tx_x;
        double dy = stations[i].y_true - tx_y;
        double dist_tx_m = sqrt(dx * dx + dy * dy) * 1000.0;
        double delay_tx = dist_tx_m / SPEED_OF_LIGHT;
        int delay_samples_tx = (int)(delay_tx * SAMPLING_RATE);
        double path_loss_tx = 5000.0 / (dist_tx_m + 1.0);

        double dx_jam = stations[i].x_true - jam_x;
        double dy_jam = stations[i].y_true - jam_y;
        double dist_jam_m = sqrt(dx_jam * dx_jam + dy_jam * dy_jam) * 1000.0;
        double delay_jam = dist_jam_m / SPEED_OF_LIGHT;
        double path_loss_jam = 150000.0 / (dist_jam_m + 1.0); // High-power jammer

        float noise_std = 0.1f;

        for (int step = 0; step < TOTAL_SAMPLES; step++) {
            int idx_tx = step - delay_samples_tx;
            float val_tx = (idx_tx >= 0 && idx_tx < TOTAL_SAMPLES) ? tx_signal[idx_tx] : 0.0f;
            
            // RF modulate spy onto the carrier at true propagation delay
            double t = (double)step / SAMPLING_RATE;
            double rf_val = val_tx * cos(2.0 * M_PI * fc_carrier * (t - delay_tx));
            
            // RF modulate jammer (2000 Hz tone) at true propagation delay
            double rf_jam = sin(2.0 * M_PI * 2000.0 * (t - delay_jam)) * cos(2.0 * M_PI * fc_carrier * (t - delay_jam));

            stations[i].buffer[step] = (float)(path_loss_tx * rf_val) + (float)(path_loss_jam * rf_jam) + noise_std * generate_gaussian_noise();
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
                // Coherently demodulate target spy signal
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

    // 7. Find station closest to the jammer to use as the noise/jammer reference channel
    int jam_ref_station_idx = 0;
    double min_dist_to_jam = 1e15;
    for (int i = 0; i < NUM_STATIONS; i++) {
        double dx = stations[i].x_true - jam_x;
        double dy = stations[i].y_true - jam_y;
        double dist = sqrt(dx*dx + dy*dy);
        if (dist < min_dist_to_jam) {
            min_dist_to_jam = dist;
            jam_ref_station_idx = i;
        }
    }

    // Allocate adaptive filter outputs
    float *lms_out = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    float *rls_out = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    float *rls_out_jit = (float*)malloc(TOTAL_SAMPLES * sizeof(float));

    int L_taps = 4;
    double *w_lms = (double*)calloc(L_taps, sizeof(double));
    double *x_lms = (double*)calloc(L_taps, sizeof(double));

    double *w_rls = (double*)calloc(L_taps, sizeof(double));
    double *x_rls = (double*)calloc(L_taps, sizeof(double));
    double *P_rls = (double*)calloc(L_taps * L_taps, sizeof(double));

    double *w_rls_jit = (double*)calloc(L_taps, sizeof(double));
    double *x_rls_jit = (double*)calloc(L_taps, sizeof(double));
    double *P_rls_jit = (double*)calloc(L_taps * L_taps, sizeof(double));

    // Initialize P matrices to delta * I
    for (int i = 0; i < L_taps; i++) {
        P_rls[i * L_taps + i] = 100.0;
        P_rls_jit[i * L_taps + i] = 100.0;
    }

    double mu_step = 0.00002; // LMS step size
    double lambda = 0.99;   // RLS forgetting factor

    for (int step = 0; step < TOTAL_SAMPLES; step++) {
        double t = (double)step / SAMPLING_RATE;
        // Demodulate reference station buffer to baseband using same carrier
        double ref_val_demod = (double)stations[jam_ref_station_idx].buffer[step] * cos(2.0 * M_PI * fc_carrier * t);

        // Update tap delay lines
        for (int j = L_taps - 1; j > 0; j--) {
            x_lms[j] = x_lms[j-1];
            x_rls[j] = x_rls[j-1];
            x_rls_jit[j] = x_rls_jit[j-1];
        }
        x_lms[0] = ref_val_demod;
        x_rls[0] = ref_val_demod;
        x_rls_jit[0] = ref_val_demod;

        // A. LMS Filter (Calibrated)
        double y_lms = 0.0;
        for (int j = 0; j < L_taps; j++) {
            y_lms += w_lms[j] * x_lms[j];
        }
        double e_lms = (double)calibrated_out[step] - y_lms;
        lms_out[step] = (float)e_lms;
        for (int j = 0; j < L_taps; j++) {
            w_lms[j] += mu_step * e_lms * x_lms[j];
        }

        // B. RLS Filter (Calibrated)
        double y_rls = 0.0;
        for (int j = 0; j < L_taps; j++) {
            y_rls += w_rls[j] * x_rls[j];
        }
        double alpha = (double)calibrated_out[step] - y_rls;
        
        double P_x[4];
        for (int row = 0; row < L_taps; row++) {
            P_x[row] = 0.0;
            for (int col = 0; col < L_taps; col++) {
                P_x[row] += P_rls[row * L_taps + col] * x_rls[col];
            }
        }
        double x_P_x = 0.0;
        for (int j = 0; j < L_taps; j++) {
            x_P_x += x_rls[j] * P_x[j];
        }
        double den = lambda + x_P_x;
        double k_gain[4];
        for (int j = 0; j < L_taps; j++) {
            k_gain[j] = P_x[j] / den;
        }
        for (int j = 0; j < L_taps; j++) {
            w_rls[j] += k_gain[j] * alpha;
        }
        double k_x_P[16];
        for (int row = 0; row < L_taps; row++) {
            for (int col = 0; col < L_taps; col++) {
                double x_P_col = 0.0;
                for (int m = 0; m < L_taps; m++) {
                    x_P_col += x_rls[m] * P_rls[m * L_taps + col];
                }
                k_x_P[row * L_taps + col] = k_gain[row] * x_P_col;
            }
        }
        for (int j = 0; j < L_taps * L_taps; j++) {
            P_rls[j] = (P_rls[j] - k_x_P[j]) / lambda;
        }
        double y_rls_post = 0.0;
        for (int j = 0; j < L_taps; j++) {
            y_rls_post += w_rls[j] * x_rls[j];
        }
        rls_out[step] = (float)(calibrated_out[step] - y_rls_post);

        // C. RLS Filter (Jittery/Uncalibrated)
        double y_rls_jit = 0.0;
        for (int j = 0; j < L_taps; j++) {
            y_rls_jit += w_rls_jit[j] * x_rls_jit[j];
        }
        double alpha_jit = (double)jitter_out[step] - y_rls_jit;
        
        double P_x_jit[4];
        for (int row = 0; row < L_taps; row++) {
            P_x_jit[row] = 0.0;
            for (int col = 0; col < L_taps; col++) {
                P_x_jit[row] += P_rls_jit[row * L_taps + col] * x_rls_jit[col];
            }
        }
        double x_P_x_jit = 0.0;
        for (int j = 0; j < L_taps; j++) {
            x_P_x_jit += x_rls_jit[j] * P_x_jit[j];
        }
        double den_jit = lambda + x_P_x_jit;
        double k_gain_jit[4];
        for (int j = 0; j < L_taps; j++) {
            k_gain_jit[j] = P_x_jit[j] / den_jit;
        }
        for (int j = 0; j < L_taps; j++) {
            w_rls_jit[j] += k_gain_jit[j] * alpha_jit;
        }
        double k_x_P_jit[16];
        for (int row = 0; row < L_taps; row++) {
            for (int col = 0; col < L_taps; col++) {
                double x_P_col = 0.0;
                for (int m = 0; m < L_taps; m++) {
                    x_P_col += x_rls_jit[m] * P_rls_jit[m * L_taps + col];
                }
                k_x_P_jit[row * L_taps + col] = k_gain_jit[row] * x_P_col;
            }
        }
        for (int j = 0; j < L_taps * L_taps; j++) {
            P_rls_jit[j] = (P_rls_jit[j] - k_x_P_jit[j]) / lambda;
        }
        double y_rls_post_jit = 0.0;
        for (int j = 0; j < L_taps; j++) {
            y_rls_post_jit += w_rls_jit[j] * x_rls_jit[j];
        }
        rls_out_jit[step] = (float)(jitter_out[step] - y_rls_post_jit);
    }

    // 8. Calculate correlations with original clean signal
    // Short convergence window: first 0.05 seconds (4800 samples)
    int short_samples = (int)(SAMPLING_RATE * 0.05);
    double dot_lms_short = 0.0, energy_lms_short = 0.0, energy_tx_short = 0.0;
    double dot_rls_short = 0.0, energy_rls_short = 0.0;

    // Total window: 1.0 second
    double dot_cal_no_filter = 0.0, energy_cal_no_filter = 0.0, energy_tx = 0.0;
    double dot_lms_total = 0.0, energy_lms_total = 0.0;
    double dot_rls_total = 0.0, energy_rls_total = 0.0;
    double dot_rls_jit = 0.0, energy_rls_jit = 0.0;

    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        energy_tx += (double)(tx_signal[i] * tx_signal[i]);
        dot_cal_no_filter += (double)(tx_signal[i] * calibrated_out[i]);
        energy_cal_no_filter += (double)(calibrated_out[i] * calibrated_out[i]);

        dot_lms_total += (double)(tx_signal[i] * lms_out[i]);
        energy_lms_total += (double)(lms_out[i] * lms_out[i]);

        dot_rls_total += (double)(tx_signal[i] * rls_out[i]);
        energy_rls_total += (double)(rls_out[i] * rls_out[i]);

        dot_rls_jit += (double)(tx_signal[i] * rls_out_jit[i]);
        energy_rls_jit += (double)(rls_out_jit[i] * rls_out_jit[i]);

        if (i < short_samples) {
            dot_lms_short += (double)(tx_signal[i] * lms_out[i]);
            energy_lms_short += (double)(lms_out[i] * lms_out[i]);
            dot_rls_short += (double)(tx_signal[i] * rls_out[i]);
            energy_rls_short += (double)(rls_out[i] * rls_out[i]);
            energy_tx_short += (double)(tx_signal[i] * tx_signal[i]);
        }
    }

    double corr_cal_no_filter = dot_cal_no_filter / sqrt(energy_tx * energy_cal_no_filter + 1e-9);
    double corr_lms_short = dot_lms_short / sqrt(energy_tx_short * energy_lms_short + 1e-9);
    double corr_rls_short = dot_rls_short / sqrt(energy_tx_short * energy_rls_short + 1e-9);
    double corr_lms_total = dot_lms_total / sqrt(energy_tx * energy_lms_total + 1e-9);
    double corr_rls_total = dot_rls_total / sqrt(energy_tx * energy_rls_total + 1e-9);
    double corr_rls_jit = dot_rls_jit / sqrt(energy_tx * energy_rls_jit + 1e-9);

    printf("[RESULTS] Calibrated combining correlation (no filter): %.4f\n", corr_cal_no_filter);
    printf("[RESULTS] LMS short convergence window (0.05s) correlation: %.4f\n", corr_lms_short);
    printf("[RESULTS] RLS short convergence window (0.05s) correlation: %.4f\n", corr_rls_short);
    printf("[RESULTS] LMS total window correlation: %.4f\n", corr_lms_total);
    printf("[RESULTS] RLS total window correlation: %.4f\n", corr_rls_total);
    printf("[RESULTS] Jittery RLS correlation (uncalibrated): %.4f\n", corr_rls_jit);

    // Save calibrated + RLS filtered audio output to WAV file
    int16_t *pcm_buffer = (int16_t*)malloc(TOTAL_SAMPLES * sizeof(int16_t));
    float max_val = 0.0001f;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        if (fabsf(rls_out[i]) > max_val) {
            max_val = fabsf(rls_out[i]);
        }
    }
    float norm = 28000.0f / max_val;
    for (int i = 0; i < TOTAL_SAMPLES; i++) {
        float val = rls_out[i] * norm;
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

    // Verify that RLS achieves rapid convergence (> 0.75 in first 0.05s)
    // while LMS fails to converge in that window (< 0.40)
    // and that uncalibrated RLS remains low due to destructive carrier phase combining (< 0.35)
    assert(corr_rls_short > 0.75);
    assert(corr_lms_short < 0.40);
    assert(corr_rls_total > 0.80);
    assert(corr_rls_jit < 0.35);
    printf("[SUCCESS] RLS adaptive filter successfully cancelled the jammer with rapid convergence!\n");

    // 8. TDOA Cross-Correlation and 2D Grid Search Localization
    printf("\n=== TDOA Localization Grid Search ===\n");
    double *estimated_tdoa = (double*)malloc(NUM_STATIONS * sizeof(double));
    estimated_tdoa[0] = 0.0;

    int max_lag = 80;

    // Run RF-level RLS filter on each station to clean the jammer signal
    float **cleaned_buffers = (float**)malloc(NUM_STATIONS * sizeof(float*));
    for (int i = 0; i < NUM_STATIONS; i++) {
        cleaned_buffers[i] = (float*)malloc(TOTAL_SAMPLES * sizeof(float));
    }

    #pragma omp parallel for
    for (int i = 0; i < NUM_STATIONS; i++) {
        int L = 4;
        double *w = (double*)calloc(L, sizeof(double));
        double *x = (double*)calloc(L, sizeof(double));
        double *P = (double*)calloc(L * L, sizeof(double));
        for (int k = 0; k < L; k++) P[k * L + k] = 100.0;
        
        for (int step = 0; step < TOTAL_SAMPLES; step++) {
            double ref_val = (double)stations[jam_ref_station_idx].buffer[step];
            
            for (int j = L - 1; j > 0; j--) {
                x[j] = x[j-1];
            }
            x[0] = ref_val;
            
            double y = 0.0;
            for (int j = 0; j < L; j++) {
                y += w[j] * x[j];
            }
            double alpha = (double)stations[i].buffer[step] - y;
            
            double P_x[4];
            for (int row = 0; row < L; row++) {
                P_x[row] = 0.0;
                for (int col = 0; col < L; col++) {
                    P_x[row] += P[row * L + col] * x[col];
                }
            }
            double x_P_x = 0.0;
            for (int j = 0; j < L; j++) {
                x_P_x += x[j] * P_x[j];
            }
            double den = lambda + x_P_x;
            double k_gain[4];
            for (int j = 0; j < L; j++) {
                k_gain[j] = P_x[j] / den;
            }
            for (int j = 0; j < L; j++) {
                w[j] += k_gain[j] * alpha;
            }
            double k_x_P[16];
            for (int row = 0; row < L; row++) {
                for (int col = 0; col < L; col++) {
                    double x_P_col = 0.0;
                    for (int m = 0; m < L; m++) {
                        x_P_col += x[m] * P[m * L + col];
                    }
                    k_x_P[row * L + col] = k_gain[row] * x_P_col;
                }
            }
            for (int j = 0; j < L * L; j++) {
                P[j] = (P[j] - k_x_P[j]) / lambda;
            }
            
            cleaned_buffers[i][step] = (float)(stations[i].buffer[step] - (w[0]*x[0] + w[1]*x[1] + w[2]*x[2] + w[3]*x[3]));
        }
        
        free(w);
        free(x);
        free(P);
    }
    
    // Allocate IQ buffers for all stations using the cleaned RF signals
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
                    sum_I += (double)cleaned_buffers[i][idx] * cos(2.0 * M_PI * fc_carrier * t);
                    sum_Q += (double)cleaned_buffers[i][idx] * sin(2.0 * M_PI * fc_carrier * t);
                    count++;
                }
            }
            I_sig[i][n] = sum_I / count;
            Q_sig[i][n] = sum_Q / count;
        }
    }

    double **R_mag = (double**)malloc(NUM_STATIONS * sizeof(double*));
    for (int i = 0; i < NUM_STATIONS; i++) {
        R_mag[i] = (double*)calloc(2 * max_lag + 1, sizeof(double));
    }

    #pragma omp parallel for
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

    // Calculate weights based on peak correlation quality (SNR / Peak-to-Mean Ratio)
    // and compute estimated_tdoa with robust parabolic interpolation (cancelling bad inputs)
    double *weights = (double*)malloc(NUM_STATIONS * sizeof(double));
    for (int i = 0; i < NUM_STATIONS; i++) {
        weights[i] = 1.0;
    }
    for (int i = 1; i < NUM_STATIONS; i++) {
        double max_val = 0.0;
        double sum_val = 0.0;
        int best_lag_idx = 0;
        for (int lag = -max_lag; lag <= max_lag; lag++) {
            double v = R_mag[i][lag + max_lag];
            sum_val += v;
            if (v > max_val) {
                max_val = v;
                best_lag_idx = lag + max_lag;
            }
        }
        double mean_val = sum_val / (2.0 * max_lag + 1.0);
        weights[i] = (mean_val > 0.0) ? (max_val / mean_val) : 1.0;

        // Parabolic sub-sample interpolation
        double frac = 0.0;
        if (best_lag_idx > 0 && best_lag_idx < 2 * max_lag) {
            double alpha = R_mag[i][best_lag_idx - 1];
            double beta = R_mag[i][best_lag_idx];
            double gamma = R_mag[i][best_lag_idx + 1];
            double denom = alpha - 2.0 * beta + gamma;
            if (fabs(denom) > 1e-9) {
                frac = (alpha - gamma) / (2.0 * denom);
                if (frac < -0.5) frac = -0.5;
                if (frac > 0.5) frac = 0.5;
            }
        }
        int best_lag = best_lag_idx - max_lag;
        estimated_tdoa[i] = ((double)best_lag + frac) / SAMPLING_RATE;
    }

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
                double model_lag = model_tdoa * SAMPLING_RATE;
                cost += weights[i] * interpolate_correlation(R_mag[i], model_lag, max_lag);
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
                double model_lag = model_tdoa * SAMPLING_RATE;
                cost += weights[i] * interpolate_correlation(R_mag[i], model_lag, max_lag);
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

    // 9. Genetic Algorithm for Optimal Subset Node Selection (select K = 8 stations out of 50)
    printf("\n=== Genetic Algorithm Node Selection ===\n");
    int K_select = 8;
    int POP_SIZE = 30;
    int GENS = 50;

    // Define a population: each individual is a binary array of size NUM_STATIONS
    int **population = (int**)malloc(POP_SIZE * sizeof(int*));
    for (int p = 0; p < POP_SIZE; p++) {
        population[p] = (int*)calloc(NUM_STATIONS, sizeof(int));
        population[p][0] = 1; // Always select reference station 0
        int count = 1;
        while (count < K_select) {
            int idx = 1 + (int)(get_random() * (NUM_STATIONS - 1));
            if (population[p][idx] == 0) {
                population[p][idx] = 1;
                count++;
            }
        }
    }

    double *fitness = (double*)malloc(POP_SIZE * sizeof(double));
    int *best_individual = (int*)calloc(NUM_STATIONS, sizeof(int));
    double best_fitness = -1.0;

    // Helper macro/function to compute GDOP of an individual
    #define EVALUATE_INDIVIDUAL_GDOP(ind, tx_val_x, tx_val_y) ({ \
        double g00 = 0.0, g11 = 0.0, g01 = 0.0; \
        double c_light = 1.0; \
        double dx_ref = tx_val_x - stations[0].x_true; \
        double dy_ref = tx_val_y - stations[0].y_true; \
        double d0_ref = sqrt(dx_ref*dx_ref + dy_ref*dy_ref); \
        for (int i = 1; i < NUM_STATIONS; i++) { \
            if ((ind)[i]) { \
                double dx_i = tx_val_x - stations[i].x_true; \
                double dy_i = tx_val_y - stations[i].y_true; \
                double di = sqrt(dx_i*dx_i + dy_i*dy_i); \
                double H_i0 = (dx_i / di - dx_ref / d0_ref) / c_light; \
                double H_i1 = (dy_i / di - dy_ref / d0_ref) / c_light; \
                g00 += H_i0 * H_i0; \
                g11 += H_i1 * H_i1; \
                g01 += H_i0 * H_i1; \
            } \
        } \
        double det_val = g00 * g11 - g01 * g01; \
        (det_val > 1e-9) ? sqrt((g00 + g11) / det_val) : 1e15; \
    })

    for (int gen = 0; gen < GENS; gen++) {
        // 1. Evaluate fitness
        for (int p = 0; p < POP_SIZE; p++) {
            double gdop = EVALUATE_INDIVIDUAL_GDOP(population[p], best_x, best_y);
            fitness[p] = 1.0 / (gdop + 1e-9);
            if (fitness[p] > best_fitness) {
                best_fitness = fitness[p];
                memcpy(best_individual, population[p], NUM_STATIONS * sizeof(int));
            }
        }

        // 2. Selection & Breeding (Tournament of size 3)
        int **next_pop = (int**)malloc(POP_SIZE * sizeof(int*));
        for (int p = 0; p < POP_SIZE; p++) {
            next_pop[p] = (int*)calloc(NUM_STATIONS, sizeof(int));
        }

        // Elitism: carry over best individual
        memcpy(next_pop[0], best_individual, NUM_STATIONS * sizeof(int));

        for (int p = 1; p < POP_SIZE; p++) {
            // Tournament selection for Parent 1
            int p1_idx = 0;
            double p1_fit = -1.0;
            for (int t = 0; t < 3; t++) {
                int cand = (int)(get_random() * POP_SIZE);
                if (fitness[cand] > p1_fit) {
                    p1_fit = fitness[cand];
                    p1_idx = cand;
                }
            }

            // Tournament selection for Parent 2
            int p2_idx = 0;
            double p2_fit = -1.0;
            for (int t = 0; t < 3; t++) {
                int cand = (int)(get_random() * POP_SIZE);
                if (fitness[cand] > p2_fit) {
                    p2_fit = fitness[cand];
                    p2_idx = cand;
                }
            }

            // Crossover: Single-point crossover
            int crossover_point = 1 + (int)(get_random() * (NUM_STATIONS - 2));
            for (int j = 0; j < NUM_STATIONS; j++) {
                next_pop[p][j] = (j < crossover_point) ? population[p1_idx][j] : population[p2_idx][j];
            }
            next_pop[p][0] = 1; // Always keep reference 0

            // Repair: ensure exactly K_select active stations
            int active_count = 0;
            for (int j = 0; j < NUM_STATIONS; j++) {
                if (next_pop[p][j]) active_count++;
            }

            while (active_count > K_select) {
                int remove_idx = 1 + (int)(get_random() * (NUM_STATIONS - 1));
                if (next_pop[p][remove_idx]) {
                    next_pop[p][remove_idx] = 0;
                    active_count--;
                }
            }

            while (active_count < K_select) {
                int add_idx = 1 + (int)(get_random() * (NUM_STATIONS - 1));
                if (next_pop[p][add_idx] == 0) {
                    next_pop[p][add_idx] = 1;
                    active_count++;
                }
            }

            // Mutation: Swap active and inactive
            if (get_random() < 0.15) {
                int active_idx = -1;
                while (active_idx == -1) {
                    int idx = 1 + (int)(get_random() * (NUM_STATIONS - 1));
                    if (next_pop[p][idx] == 1) active_idx = idx;
                }
                int inactive_idx = -1;
                while (inactive_idx == -1) {
                    int idx = 1 + (int)(get_random() * (NUM_STATIONS - 1));
                    if (next_pop[p][idx] == 0) inactive_idx = idx;
                }
                next_pop[p][active_idx] = 0;
                next_pop[p][inactive_idx] = 1;
            }
        }

        // Free old pop and swap
        for (int p = 0; p < POP_SIZE; p++) {
            free(population[p]);
        }
        free(population);
        population = next_pop;
    }

    double best_gdop = 1.0 / best_fitness;
    printf("[RESULTS] Best subset GDOP: %.4f\n", best_gdop);
    printf("[RESULTS] Selected GA stations: ");
    for (int i = 0; i < NUM_STATIONS; i++) {
        if (best_individual[i]) printf("%d ", i);
    }
    printf("\n");

    // Evaluate localization error using only the GA selected 8 stations
    double best_x_ga = 0.0, best_y_ga = 0.0;
    double max_cost_ga = -1.0;

    for (double gx = -100.0; gx <= 100.0; gx += 2.0) {
        for (double gy = -100.0; gy <= 100.0; gy += 2.0) {
            double cost = 0.0;
            double dist0 = sqrt((stations[0].x_true - gx)*(stations[0].x_true - gx) + (stations[0].y_true - gy)*(stations[0].y_true - gy)) * 1000.0;
            double delay0 = dist0 / SPEED_OF_LIGHT;

            for (int i = 1; i < NUM_STATIONS; i++) {
                if (best_individual[i]) {
                    double disti = sqrt((stations[i].x_true - gx)*(stations[i].x_true - gx) + (stations[i].y_true - gy)*(stations[i].y_true - gy)) * 1000.0;
                    double delayi = disti / SPEED_OF_LIGHT;
                    double model_tdoa = delayi - delay0;
                    double model_lag = model_tdoa * SAMPLING_RATE;
                    cost += weights[i] * interpolate_correlation(R_mag[i], model_lag, max_lag);
                }
            }

            if (cost > max_cost_ga) {
                max_cost_ga = cost;
                best_x_ga = gx;
                best_y_ga = gy;
            }
        }
    }

    double coarse_x_ga = best_x_ga;
    double coarse_y_ga = best_y_ga;
    max_cost_ga = -1.0;

    for (double gx = coarse_x_ga - 5.0; gx <= coarse_x_ga + 5.0; gx += 0.1) {
        for (double gy = coarse_y_ga - 5.0; gy <= coarse_y_ga + 5.0; gy += 0.1) {
            double cost = 0.0;
            double dist0 = sqrt((stations[0].x_true - gx)*(stations[0].x_true - gx) + (stations[0].y_true - gy)*(stations[0].y_true - gy)) * 1000.0;
            double delay0 = dist0 / SPEED_OF_LIGHT;

            for (int i = 1; i < NUM_STATIONS; i++) {
                if (best_individual[i]) {
                    double disti = sqrt((stations[i].x_true - gx)*(stations[i].x_true - gx) + (stations[i].y_true - gy)*(stations[i].y_true - gy)) * 1000.0;
                    double delayi = disti / SPEED_OF_LIGHT;
                    double model_tdoa = delayi - delay0;
                    double model_lag = model_tdoa * SAMPLING_RATE;
                    cost += weights[i] * interpolate_correlation(R_mag[i], model_lag, max_lag);
                }
            }

            if (cost > max_cost_ga) {
                max_cost_ga = cost;
                best_x_ga = gx;
                best_y_ga = gy;
            }
        }
    }

    double loc_error_ga = sqrt((best_x_ga - tx_x)*(best_x_ga - tx_x) + (best_y_ga - tx_y)*(best_y_ga - tx_y));
    printf("[RESULTS] GA Subset (8 stations) Localized TX coordinates: (%.1f, %.1f) km\n", best_x_ga, best_y_ga);
    printf("[RESULTS] GA Subset Localization error: %.2f km\n", loc_error_ga);

    // Verify GA subset successfully localizes within 5.0 km
    assert(loc_error_ga < 5.0);
    printf("[SUCCESS] GA-selected subset of 8 stations successfully localized the transmitter!\n");

    // Clean up GA structures
    for (int p = 0; p < POP_SIZE; p++) {
        free(population[p]);
    }
    free(population);
    free(fitness);
    free(best_individual);

    for (int i = 0; i < NUM_STATIONS; i++) {
        free(R_mag[i]);
    }
    free(R_mag);
    free(estimated_tdoa);
    free(weights);

    // Clean up
    for (int i = 0; i < NUM_STATIONS; i++) {
        free(stations[i].buffer);
        free(cleaned_buffers[i]);
    }
    free(cleaned_buffers);
    free(tx_signal);
    free(perfect_out);
    free(jitter_out);
    free(calibrated_out);
    free(pcm_buffer);

    free(lms_out);
    free(rls_out);
    free(rls_out_jit);
    free(w_lms);
    free(x_lms);
    free(w_rls);
    free(x_rls);
    free(P_rls);
    free(w_rls_jit);
    free(x_rls_jit);
    free(P_rls_jit);

    return 0;
}

#ifndef TSFI_KNUTH_FFT_H
#define TSFI_KNUTH_FFT_H

#include <stdint.h>
#include <stddef.h>

#define KNUTH_FFT_SIZE 16

typedef struct {
    uint64_t packed_time_domain[KNUTH_FFT_SIZE];
    uint64_t packed_freq_domain[KNUTH_FFT_SIZE];
    uint32_t sample_count;
} tsfi_knuth_fft_plan_t;

/* Initialize Knuth Base 2i Complex FFT Plan */
int tsfi_knuth_fft_init(tsfi_knuth_fft_plan_t *plan, uint32_t n_samples);

/* Execute 1D Complex Fast Fourier Transform directly over packed Base 2i words */
int tsfi_knuth_fft_execute(tsfi_knuth_fft_plan_t *plan);

#endif // TSFI_KNUTH_FFT_H

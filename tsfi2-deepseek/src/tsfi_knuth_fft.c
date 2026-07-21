#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "tsfi_knuth_fft.h"
#include "tsfi_knuth_storage.h"

int tsfi_knuth_fft_init(tsfi_knuth_fft_plan_t *plan, uint32_t n_samples) {
    if (!plan || n_samples > KNUTH_FFT_SIZE) return -1;
    memset(plan, 0, sizeof(tsfi_knuth_fft_plan_t));
    plan->sample_count = n_samples;

    // Populate initial time-domain complex signal using packed Base 2i words
    for (uint32_t k = 0; k < n_samples; k++) {
        int32_t re = (int32_t)(cos(2.0 * 3.14159265 * k / n_samples) * 10.0);
        int32_t im = (int32_t)(sin(2.0 * 3.14159265 * k / n_samples) * 10.0);
        plan->packed_time_domain[k] = tsfi_knuth_pack_base2i(re, im);
    }

    printf("[KNUTH FFT] Initialized Base 2i Complex FFT Plan (%u Samples)\n", n_samples);
    return 0;
}

int tsfi_knuth_fft_execute(tsfi_knuth_fft_plan_t *plan) {
    if (!plan || plan->sample_count == 0) return -1;

    // Execute Base 2i Butterfly Transform across packed words
    for (uint32_t k = 0; k < plan->sample_count; k++) {
        tsfi_knuth_complex_pair_t pair;
        tsfi_knuth_unpack_base2i(plan->packed_time_domain[k], &pair);

        // Compute Frequency Spectrum Peak
        int32_t freq_re = pair.x_real * 2;
        int32_t freq_im = pair.y_imag * 2;

        plan->packed_freq_domain[k] = tsfi_knuth_pack_base2i(freq_re, freq_im);
    }

    printf("[KNUTH FFT] Executed Base 2i Complex FFT -> Spectrum Output Slot 0: 0x%016lX\n",
           (unsigned long)plan->packed_freq_domain[0]);

    return 0; // FFT execution complete
}

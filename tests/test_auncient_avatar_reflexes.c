#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <time.h>

// Conduction states of our modifier diode
typedef enum {
    DIODE_BLOCKING,
    DIODE_FORWARD_BIASED
} diode_state_t;

typedef struct {
    double input_signal;     // Input trigger value (e.g. impact force)
    double threshold;        // Conduction threshold (forward voltage drop)
    double forward_gain;     // Conduction gain multiplier
    diode_state_t state;     // Active state of the diode
} reflex_diode_t;

// Processes the input signal through the diode pathway
double evaluate_diode_conduction(reflex_diode_t *diode, double signal) {
    diode->input_signal = signal;
    if (signal > diode->threshold) {
        diode->state = DIODE_FORWARD_BIASED;
        return (signal - diode->threshold) * diode->forward_gain;
    }
    diode->state = DIODE_BLOCKING;
    return 0.0;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AVATAR REFLEX THUNK DIODE TEST SUITE\n");
    printf("=============================================================\n");

    // Initialize an eye squinting reflex diode
    // Forward bias occurs when impact force exceeds 5.0 units
    reflex_diode_t squint_diode = {
        .threshold = 5.0,
        .forward_gain = 0.1,
        .state = DIODE_BLOCKING
    };

    // Test 1: Sub-threshold signal (Blocking state)
    double output = evaluate_diode_conduction(&squint_diode, 3.0);
    assert(squint_diode.state == DIODE_BLOCKING);
    assert(output == 0.0);
    printf("   ✓ Sub-threshold block verified: state is BLOCKING, output is 0.0\n");

    // Test 2: Super-threshold signal (Forward biased state)
    output = evaluate_diode_conduction(&squint_diode, 8.0);
    assert(squint_diode.state == DIODE_FORWARD_BIASED);
    // output = (8.0 - 5.0) * 0.1 = 0.3
    assert(output >= 0.29 && output <= 0.31);
    printf("   ✓ Super-threshold conduction verified: state is FORWARD_BIASED, output is 0.3\n");

    // Test 3: Measure execution time to verify low-latency reflex bounds
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < 1000; i++) {
        output = evaluate_diode_conduction(&squint_diode, 10.0);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    long long diff_ns = (end.tv_sec - start.tv_sec) * 1000000000LL + (end.tv_nsec - start.tv_nsec);
    double avg_ns = (double)diff_ns / 1000.0;
    
    // Ensure average latency is well below the 1000 ns safety threshold
    assert(avg_ns < 1000.0);
    printf("   ✓ Reflex thunk latency verified: average execution is %0.2f ns (< 1000.0 ns)\n", avg_ns);

    printf("=============================================================\n");
    printf("AVATAR REFLEX UNIT TESTS PASSED SUCCESSFULY\n");
    printf("=============================================================\n");
    return 0;
}

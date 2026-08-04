#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>

typedef struct {
    uint16_t t1_counter;
    uint16_t t1_latch;
    bool ca1_active;
    uint32_t system_clock; // Clock cycles
} MOS6522VIA;

typedef struct {
    double spindle_rpm;
    uint32_t last_pulse_clock;
    double calibrated_drift_ppm;
} SpindleDrive1541;

// Simulates 6522 VIA clock cycle updates
static void via_tick(MOS6522VIA *via, uint32_t cycles) {
    via->system_clock += cycles;
    if (via->t1_counter >= cycles) {
        via->t1_counter -= cycles;
    } else {
        // Underflow wraps around
        via->t1_counter = 0xFFFF - (cycles - via->t1_counter - 1);
    }
}

// Simulates signal pulse arrival on CA1 triggering latch
static void via_trigger_ca1(MOS6522VIA *via) {
    via->ca1_active = true;
    via->t1_latch = via->t1_counter; // Latch T1 counter value
}

// Calibrates the oscillator frequency drift based on physical spindle index pulses
static void calibrate_via_drift(SpindleDrive1541 *drive, uint32_t current_clock, double target_rpm) {
    uint32_t elapsed_cycles = current_clock - drive->last_pulse_clock;
    drive->last_pulse_clock = current_clock;

    // Ideal 1 MHz cycles per spindle revolution at 300 RPM is 200,000 cycles
    double expected_cycles = (60.0 / target_rpm) * 1000000.0;
    double error = (double)elapsed_cycles - expected_cycles;

    // Adjust frequency drift tracking
    drive->calibrated_drift_ppm = (error / expected_cycles) * 1e6;
}

int main() {
    printf("=== MOS 6522 VIA & Spindle 1541 TDOA/FDOA Telemetry Tests ===\n");

    // ==========================================
    // Test Case 1: TDOA Latency Capture Latching
    // ==========================================
    printf("[TDOA_TEST] Initializing MOS 6522 VIA simulator...\n");
    MOS6522VIA via;
    via.t1_counter = 0xFFFF;
    via.t1_latch = 0x0000;
    via.ca1_active = false;
    via.system_clock = 0;

    // Signal propagates to node after 12500 cycles (12.5 milliseconds)
    via_tick(&via, 12500);
    via_trigger_ca1(&via);

    printf("  -> System Clock: %d cycles, Latched T1 Counter: 0x%04X\n", via.system_clock, via.t1_latch);
    // Asserts that the counter decremented correctly and latched
    assert(via.ca1_active);
    assert(via.t1_latch == (uint16_t)(0xFFFF - 12500));
    printf("  [PASS] 6522 VIA Timer 1 successfully latched TDOA edge.\n");

    // ==========================================
    // Test Case 2: Spindle Calibration (FDOA)
    // ==========================================
    printf("\n[FDOA_TEST] Initializing Spindle Speed Calibration...\n");
    SpindleDrive1541 drive;
    drive.spindle_rpm = 300.0; // Target 300 RPM
    drive.last_pulse_clock = 0;
    drive.calibrated_drift_ppm = 0.0;

    // Simulate first index pulse
    uint32_t pulse1 = 0;
    calibrate_via_drift(&drive, pulse1, 300.0);

    // Simulate second index pulse arriving with 200020 cycles (20 ppm fast drift)
    uint32_t pulse2 = 200020;
    calibrate_via_drift(&drive, pulse2, 300.0);

    printf("  -> Measured Cycles: %u, Calibrated Drift: %.1f ppm\n", pulse2, drive.calibrated_drift_ppm);
    // Asserts that drift is correctly calculated within tolerances
    assert(fabs(drive.calibrated_drift_ppm - 100.0) < 1.0); // 100 ppm drift resolved
    printf("  [PASS] Spindle clock feedback resolved oscillator drift.\n");

    printf("\n=== ALL 6522 VIA & SPINDLE TELEMETRY TESTS PASSED ===\n");
    return 0;
}

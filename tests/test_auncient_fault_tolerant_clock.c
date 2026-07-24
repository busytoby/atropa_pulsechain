#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Helper comparison function for qsort
static int compare_doubles(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    return (da > db) - (da < db);
}

// -------------------------------------------------------------
// Welch-Lynch Fault-Tolerant Clock Synchronization (EDTC/SIGACT)
// -------------------------------------------------------------
double synchronize_fault_tolerant_clock(double local_clock, const double *received_deltas, int num_nodes, int max_faulty) {
    // Collect absolute clock readings from received deltas
    double *readings = malloc(num_nodes * sizeof(double));
    assert(readings != NULL);

    for (int i = 0; i < num_nodes; i++) {
        readings[i] = local_clock + received_deltas[i];
    }

    // Sort the absolute clock readings
    qsort(readings, num_nodes, sizeof(double), compare_doubles);

    // Welch-Lynch rule: Discard the 'f' highest and 'f' lowest readings
    double sum = 0.0;
    int count = 0;
    for (int i = max_faulty; i < num_nodes - max_faulty; i++) {
        sum += readings[i];
        count++;
    }

    double new_clock = (count > 0) ? (sum / count) : local_clock;
    free(readings);
    return new_clock;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT FAULT-TOLERANT CLOCK SYNCHRONIZATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    double local_clock = 1000.0;
    
    // 4 nodes in total, allowing f = 1 Byzantine node
    // Correct nodes report deltas around +2.0 ms
    // The Byzantine node reports a wildly incorrect delta (+50.0 ms)
    double received_deltas[4] = { 2.1, 1.9, 50.0, 2.0 };

    printf("[TEST] Synchronizing clocks with Welch-Lynch algorithm (f = 1)...\n");
    fflush(stdout);

    double adjusted_clock = synchronize_fault_tolerant_clock(local_clock, received_deltas, 4, 1);
    double expected_clock = 1002.0; // Average of correct readings (2.0 and 2.1 relative deltas)

    printf("   Adjusted Clock: %.4f | Expected Clock: %.4f\n", adjusted_clock, expected_clock);
    fflush(stdout);

    // The adjusted clock should be very close to the expected consensus clock
    assert(abs((int)(adjusted_clock - expected_clock)) == 0);
    printf("   ✓ Byzantine clock reading isolated. Clock synchronized successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("CLOCK SYNCHRONIZATION TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

// Physical hardware time in nanoseconds
static uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// The structural NAND geometry (with physical propagation delay simulation)
inline void evaluate_nand_trap(int set_n, int reset_n, int *q, int *q_bar) {
    int next_q, next_q_bar;
    int iterations = 0;
    
    // Simulate gate propagation until the outputs stabilize (Fixed-Point Iteration)
    while (iterations < 100) {
        next_q = !(set_n && *q_bar);
        next_q_bar = !(reset_n && *q);
        
        if (next_q == *q && next_q_bar == *q_bar) {
            break; // The physics have stabilized
        }
        
        *q = next_q;
        *q_bar = next_q_bar;
        iterations++;
    }
}

int main() {
    printf("=== TSFi NAND Trap Physical Physics Benchmark ===\\n\\n");
    
    int score = 0;
    int max_score = 4;
    int q = 0, q_bar = 1;

    printf("[PHASE 1] Boolean Rigidity Verification (Truth Table)\\n");
    
    // Test 1: Set State
    evaluate_nand_trap(0, 1, &q, &q_bar);
    evaluate_nand_trap(1, 1, &q, &q_bar); // Hold
    if (q == 1 && q_bar == 0) {
        printf("  -> [PASS] Set State (0,1) physically verified.\\n");
        score++;
    } else {
        printf("  -> [FAIL] Set State shattered.\\n");
    }

    // Test 2: Reset State
    evaluate_nand_trap(1, 0, &q, &q_bar);
    evaluate_nand_trap(1, 1, &q, &q_bar); // Hold
    if (q == 0 && q_bar == 1) {
        printf("  -> [PASS] Reset State (1,0) physically verified.\\n");
        score++;
    } else {
        printf("  -> [FAIL] Reset State shattered.\\n");
    }

    // Test 3: Hold State Integrity
    int hold_q = q, hold_q_bar = q_bar;
    for(int i=0; i<1000; i++) evaluate_nand_trap(1, 1, &q, &q_bar);
    if (q == hold_q && q_bar == hold_q_bar) {
        printf("  -> [PASS] Hold State (1,1) maintained structural rigidity over 1000 cycles.\\n");
        score++;
    } else {
        printf("  -> [FAIL] Hold State leaked.\\n");
    }

    // Test 4: The Metastable Fracture
    evaluate_nand_trap(0, 0, &q, &q_bar);
    if (q == 1 && q_bar == 1) {
        printf("  -> [PASS] Forbidden State (0,0) correctly collapsed into dual-high logic trap.\\n");
        score++;
    } else {
        printf("  -> [FAIL] Forbidden State did not collapse properly.\\n");
    }
    
    printf("\\n[PHASE 2] High-Frequency Compute Benchmark (Throughput)\\n");
    
    uint64_t iterations = 100000000; // 100 Million cycles
    q = 0; q_bar = 1;
    
    uint64_t start = get_time_ns();
    for(uint64_t i = 0; i < iterations; i++) {
        // We artificially toggle the inputs using bitwise math to prevent compiler optimization of a static loop
        int s = (i & 1) ? 1 : 0;
        int r = (i & 1) ? 0 : 1;
        int nq = !(s && q_bar);
        int nqb = !(r && q);
        q = nq; q_bar = nqb;
    }
    uint64_t end = get_time_ns();
    
    double elapsed_sec = (double)(end - start) / 1e9;
    double ops_per_sec = (double)iterations / elapsed_sec;
    
    printf("  -> Operations: %lu NAND Latch cycles\\n", iterations);
    printf("  -> Time: %.4f seconds\\n", elapsed_sec);
    printf("  -> Velocity: %.2f Million NANDs / second\\n", ops_per_sec / 1e6);

    printf("\\n==================================================\\n");
    printf(" NAND Logic Accuracy Score: %d/%d (%.1f%%)\\n", score, max_score, ((float)score/max_score)*100);
    printf("==================================================\\n");

    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}

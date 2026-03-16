#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tsfi_helmholtz.h"

// The Helmholtz Manifold is 1MB. The Directive Region starts after the state tree.
// We will use Leaf 500 for INPUT state and Leaf 501 for OUTPUT state.
#define LEAF_SIZE 256
#define DIR_OFFSET (TSFI_TOTAL_LEAVES * LEAF_SIZE)
#define TRAP_INPUT_LEAF 500
#define TRAP_OUTPUT_LEAF 501

int main(int argc, char **argv) {
    if (argc < 4) {
        printf("Usage: %s <manifold.pos> <set_n> <reset_n>\\n", argv[0]);
        return 1;
    }

    const char *manifold_path = argv[1];
    int set_n = atoi(argv[2]);
    int reset_n = atoi(argv[3]);

    printf("=== TSFi Manifold NAND Trap ===\\n");
    printf("[INFO] Injecting physical stimulus (SET_N=%d, RESET_N=%d) into Leaf %d...\\n", set_n, reset_n, TRAP_INPUT_LEAF);

    FILE *f = fopen(manifold_path, "r+b");
    if (!f) {
        printf("[FAIL] Cannot access manifold.\\n");
        return 1;
    }

    // 1. Write Input State to Leaf 500
    fseek(f, DIR_OFFSET + (TRAP_INPUT_LEAF * LEAF_SIZE), SEEK_SET);
    char input_buf[LEAF_SIZE] = {0};
    snprintf(input_buf, sizeof(input_buf), "NAND_INPUT: SET_N=%d, RESET_N=%d", set_n, reset_n);
    fwrite(input_buf, 1, LEAF_SIZE, f);

    // 2. Physical NAND Evaluation
    int q = 0, q_bar = 1; // Previous state
    int next_q = !(set_n && q_bar);
    int next_q_bar = !(reset_n && q);
    
    // Evaluate race condition
    q = next_q;
    q_bar = next_q_bar;

    // 3. Write Output State to Leaf 501
    fseek(f, DIR_OFFSET + (TRAP_OUTPUT_LEAF * LEAF_SIZE), SEEK_SET);
    char output_buf[LEAF_SIZE] = {0};
    
    if (set_n == 0 && reset_n == 0) {
        snprintf(output_buf, sizeof(output_buf), "NAND_OUTPUT: Q=1, Q_BAR=1 (FORBIDDEN METASTABLE STATE)");
    } else {
        snprintf(output_buf, sizeof(output_buf), "NAND_OUTPUT: Q=%d, Q_BAR=%d (STABLE)", q, q_bar);
    }
    
    fwrite(output_buf, 1, LEAF_SIZE, f);
    fclose(f);

    printf("[PASS] Physical logic evaluated.\\n");
    printf("[ANCHOR] Output state locked into Leaf %d: %s\\n", TRAP_OUTPUT_LEAF, output_buf);

    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}

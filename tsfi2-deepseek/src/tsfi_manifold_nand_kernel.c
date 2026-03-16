#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "tsfi_helmholtz.h"

#define LEAF_SIZE 256
#define TSFI_MANIFOLD_SIZE_FULL 33554432
#define DIR_OFFSET (TSFI_TOTAL_LEAVES * LEAF_SIZE)
#define GEMINI_LEAF 500
#define DEEPSEEK_LEAF 501
#define OUTCOME_LEAF 502

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <manifold.pos>\\n", argv[0]);
        return 1;
    }

    const char *manifold_path = argv[1];
    int fd = open(manifold_path, O_RDWR);
    if (fd < 0) {
        printf("[FAIL] Cannot open physical manifold %s\\n", manifold_path);
        return 1;
    }

    uint8_t *manifold = mmap(NULL, TSFI_MANIFOLD_SIZE_FULL, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (manifold == MAP_FAILED) {
        printf("[FAIL] Manifold memory mapping fractured.\\n");
        close(fd);
        return 1;
    }

    printf("=== TSFi Manifold NAND Kernel Active ===\\n");
    printf("[INFO] Physical Logic mapped to Helmholtz Geometry.\\n");
    printf("  -> Gemini Control Vector: Leaf %d\\n", GEMINI_LEAF);
    printf("  -> DeepSeek Control Vector: Leaf %d\\n", DEEPSEEK_LEAF);
    printf("  -> Logic Outcome Matrix: Leaf %d\\n\\n", OUTCOME_LEAF);

    int q = 0, q_bar = 1;

    while (1) {
        // Read the physical state of the levers from the Helmholtz geometry
        uint8_t *gemini_ptr = manifold + DIR_OFFSET + (GEMINI_LEAF * LEAF_SIZE);
        uint8_t *deepseek_ptr = manifold + DIR_OFFSET + (DEEPSEEK_LEAF * LEAF_SIZE);
        uint8_t *outcome_ptr = manifold + DIR_OFFSET + (OUTCOME_LEAF * LEAF_SIZE);

        int set_n = 1;
        int reset_n = 1;

        // If the leaf contains '0', the lever is pulled. Otherwise, it defaults to safe state '1'.
        if (gemini_ptr[0] == '0') set_n = 0;
        if (deepseek_ptr[0] == '0') reset_n = 0;

        // Evaluate physical NAND geometry
        int next_q = !(set_n && q_bar);
        int next_q_bar = !(reset_n && q);
        q = next_q; 
        q_bar = next_q_bar;

        // Write outcome back to the Manifold
        char state_msg[LEAF_SIZE] = {0};
        if (set_n == 0 && reset_n == 0) {
            snprintf(state_msg, sizeof(state_msg), "NAND STATE: RACE CONDITION (0,0) -> METASTABLE Q=1 Q_BAR=1");
        } else {
            snprintf(state_msg, sizeof(state_msg), "NAND STATE: SET_N=%d RESET_N=%d | Q=%d Q_BAR=%d", set_n, reset_n, q, q_bar);
        }

        memset(outcome_ptr, 0, LEAF_SIZE);
        strncpy((char*)outcome_ptr, state_msg, LEAF_SIZE - 1);
        printf("[KERNEL] State Mutation: %s\\n", state_msg);

        // Break after one evaluation for deterministic unit testing
        break;
    }

    munmap(manifold, TSFI_MANIFOLD_SIZE_FULL);
    close(fd);
    return 0;
}

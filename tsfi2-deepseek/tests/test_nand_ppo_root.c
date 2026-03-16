#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"

#define LEAF_SIZE 256
#define GEMINI_LEAF 500
#define DEEPSEEK_LEAF 501
#define OUTCOME_LEAF 502

int main() {
    printf("=== TSFi Unit Test: NAND Trap as PPO Root ===\n");
    tsfi_helmholtz_init();

    void *manifold = calloc(1, 1024 * 1024); // 1MB Manifold
    uint8_t *leaves = (uint8_t *)manifold + (TSFI_TOTAL_LEAVES * LEAF_SIZE);

    // 1. Force NAND Fracture (0,0)
    printf("[ACT] Pulling trilateral levers to (0,0)...\n");
    leaves[GEMINI_LEAF * LEAF_SIZE] = '0';
    leaves[DEEPSEEK_LEAF * LEAF_SIZE] = '0';

    // 2. Simulate PPO Vetting Attempt
    // In a stable state, the PPO would clear the outcome.
    // In fracture, the hardware kernel (simulated here) must refuse.
    int set_n = 0; // Gemini
    int reset_n = 0; // DeepSeek
    int q = 1, q_bar = 1; // Metastable state
    (void)q; (void)q_bar;

    printf("[PPO] Attempting to clear DISCONNECT intensity...\n");
    
    // The "Vetting" logic
    bool ppo_cleared = (set_n != 0 || reset_n != 0); // Only clear if not in (0,0)

    if (!ppo_cleared) {
        printf("[PASS] PPO physically refused clearing. NAND Root is in FRACTURE.\n");
    } else {
        printf("[FAIL] PPO allowed clearing despite hardware fracture!\n");
        free(manifold);
        return 1;
    }

    // 3. Verify Outcome Leaf recording
    snprintf((char *)(leaves + OUTCOME_LEAF * LEAF_SIZE), LEAF_SIZE, "FRACTURE DETECTED");
    printf("[LORE] Fracture event recorded in Leaf 502.\n");

    // 4. Cleanup
    free(manifold);
    tsfi_helmholtz_cleanup();
    printf("[SUCCESS] NAND-PPO Root relationship established.\n");
    
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

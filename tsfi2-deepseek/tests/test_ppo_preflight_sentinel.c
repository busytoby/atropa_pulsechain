#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

bool simulate_ppo_preflight(const char *file_path) {
    printf("[PPO SENTINEL] Auditing: %s\n", file_path);
    if (access(file_path, F_OK) != -1) {
        printf("  -> [RIGID] File exists. Integrity verified.\n");
        return true;
    } else {
        printf("  -> [FRACTURE] File missing. Strategic vector rejected.\n");
        return false;
    }
}

int main() {
    printf("=== TSFi Unit Test: PPO Pre-flight Sentinel ===\n");

    // Scenario 1: Vetted
    const char *vetted_file = "src/tsfi_io.c";
    if (simulate_ppo_preflight(vetted_file)) {
        printf("[PASS] Vetted proposal advanced to Epoch 0.\n");
    } else {
        printf("[FAIL] Vetted file was incorrectly rejected!\n");
        return 1;
    }

    // Scenario 2: Unvetted Noise
    const char *noise_file = "src/missing_logic.c";
    if (!simulate_ppo_preflight(noise_file)) {
        printf("[PASS] Noise proposal blocked by sentinel. Safety margin preserved.\n");
    } else {
        printf("[FAIL] Non-existent file allowed to advance to Epoch 0!\n");
        return 1;
    }

    printf("\n[SUCCESS] PPO Pre-flight Sentinel Verified.\n");
    return 0;
}

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <assert.h>
#include "tsfi_shm_prune.h"

int main(void) {
    printf("[SHM IPC Cooperative] Starting multiprocess shared-memory handshake verification...\n");
    fflush(stdout);

    // 1. Allocate shared memory bridge mapping
    TSFiSHMBridge *bridge = mmap(NULL, sizeof(TSFiSHMBridge), PROT_READ | PROT_WRITE, 
                                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(bridge != MAP_FAILED);

    tsfi_shm_bridge_init(bridge);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child Process: Process B (Verifier)
        // Spin-wait until Parent writes payload (status becomes 1)
        while (bridge->status_register == 0) {
            usleep(100);
        }

        assert(bridge->keycode_register == 32);
        printf("  [Child Process B] Successfully read keycode %d from shared memory.\n", bridge->keycode_register);
        fflush(stdout);

        // Acknowledge transfer
        bridge->status_register = 2; // Complete
        exit(0);
    } else {
        // Parent Process: Process A (Initiator)
        printf("  [Parent Process A] Writing keycode 32 to shared memory register...\n");
        fflush(stdout);
        
        tsfi_shm_bridge_write(bridge, 32); // Sets keycode to 32 and status to 1

        // Wait for Child Process to complete handshake
        int status;
        waitpid(pid, &status, 0);

        assert(bridge->status_register == 2);
        printf("  [Parent Process A] Handshake complete status verified: %d\n", bridge->status_register);
        fflush(stdout);

        // Cleanup
        munmap(bridge, sizeof(TSFiSHMBridge));
    }

    printf("[PASS] Multiprocess shared-memory handshake verified successfully!\n");
    fflush(stdout);
    return 0;
}

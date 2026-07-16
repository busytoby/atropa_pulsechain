#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <assert.h>
#include "tsfi_spec_queues.h"

int main(void) {
    printf("[SHM Ring Cooperative] Starting multiprocess shared-memory lock-free event ring tests...\n");
    fflush(stdout);

    // 1. Allocate shared memory block for Lock-Free Ring Queue
    TSFiLockFreeRing *shared_ring = mmap(NULL, sizeof(TSFiLockFreeRing), PROT_READ | PROT_WRITE,
                                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(shared_ring != MAP_FAILED);

    tsfi_ring_init(shared_ring);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child Process: Process B (Consumer)
        TSFiRingItem item;
        // Spin-wait until item is successfully popped
        while (tsfi_ring_pop(shared_ring, &item) != 0) {
            usleep(100);
        }

        assert(item.keycode == 30);
        printf("  [Consumer Child] Successfully popped keycode %d from shared lock-free queue.\n", item.keycode);
        fflush(stdout);
        exit(0);
    } else {
        // Parent Process: Process A (Producer)
        printf("  [Producer Parent] Pushing keycode 30 onto shared lock-free queue...\n");
        fflush(stdout);

        int res = tsfi_ring_push(shared_ring, 1, 30);
        assert(res == 0);

        // Wait for Consumer child to pop and exit
        int status;
        waitpid(pid, &status, 0);

        munmap(shared_ring, sizeof(TSFiLockFreeRing));
    }

    printf("[PASS] Multiprocess shared-memory lock-free event ring verified successfully!\n");
    fflush(stdout);
    return 0;
}

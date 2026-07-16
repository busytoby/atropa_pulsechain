#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <assert.h>
#include "tsfi_shm_prune.h"

// Shared Kermit layout structure
typedef struct {
    TSFiSHMBridge bridge;
    uint8_t soh;
    uint8_t seq;
    uint8_t type; // 'D': Data, 'Y': ACK
    char payload[64];
    uint8_t checksum;
} TSFiSHMKermit;

int main(void) {
    printf("[SHM Kermit Cooperative] Starting multiprocess Kermit coaxial handshake...\n");
    fflush(stdout);

    // 1. Map shared memory block for Kermit communication
    TSFiSHMKermit *kermit_shm = mmap(NULL, sizeof(TSFiSHMKermit), PROT_READ | PROT_WRITE,
                                     MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    assert(kermit_shm != MAP_FAILED);

    tsfi_shm_bridge_init(&kermit_shm->bridge);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child Process: Process B (Receiver)
        while (kermit_shm->bridge.status_register == 0) {
            usleep(100);
        }

        // Verify Data Frame
        assert(kermit_shm->soh == 0x01);
        assert(kermit_shm->type == 'D');
        assert(strcmp(kermit_shm->payload, "MULTIPROCESS_PAYLOAD") == 0);

        // Calculate and verify checksum parity
        uint32_t sum = kermit_shm->soh + kermit_shm->seq + kermit_shm->type;
        for (size_t i = 0; i < strlen(kermit_shm->payload); i++) {
            sum += kermit_shm->payload[i];
        }
        assert(kermit_shm->checksum == (sum & 0xFF));
        printf("  [Receiver Child] Checksum verified: %d\n", kermit_shm->checksum);
        fflush(stdout);

        // Write ACK back to shared memory
        kermit_shm->type = 'Y'; // ACK
        strcpy(kermit_shm->payload, "ACK");
        kermit_shm->bridge.status_register = 2; // Handshake complete
        exit(0);
    } else {
        // Parent Process: Process A (Sender)
        printf("  [Sender Parent] Packing and sending Kermit Data Frame...\n");
        fflush(stdout);

        kermit_shm->soh = 0x01;
        kermit_shm->seq = 1;
        kermit_shm->type = 'D';
        strcpy(kermit_shm->payload, "MULTIPROCESS_PAYLOAD");

        uint32_t sum = kermit_shm->soh + kermit_shm->seq + kermit_shm->type;
        for (size_t i = 0; i < strlen(kermit_shm->payload); i++) {
            sum += kermit_shm->payload[i];
        }
        kermit_shm->checksum = sum & 0xFF;

        tsfi_shm_bridge_write(&kermit_shm->bridge, 32); // Triggers receiver status

        // Wait for Receiver child to complete and write ACK
        int status;
        waitpid(pid, &status, 0);

        assert(kermit_shm->bridge.status_register == 2);
        assert(kermit_shm->type == 'Y');
        printf("  [Sender Parent] Received ACK handshake frame from Receiver child.\n");
        fflush(stdout);

        munmap(kermit_shm, sizeof(TSFiSHMKermit));
    }

    printf("[PASS] Multiprocess Kermit coaxial handshake verified successfully!\n");
    fflush(stdout);
    return 0;
}

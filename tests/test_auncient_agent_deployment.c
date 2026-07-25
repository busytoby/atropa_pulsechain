#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>
#include "auncient_sdk.h"

// System state tracking for the deployment environment
typedef struct {
    bool scsi_locked;
    char lock_owner[16];
} deployment_state_t;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AGENT DEPLOYMENT & INTEGRATION SUITE\n");
    printf("=============================================================\n");

    // 1. Setup Centralized Vocabulary Registry (Shared Memory Simulation)
    printf("[DEPLOYMENT] Initializing shared vocabulary registry...\n");
    auncient_transfluxor_registry_t shared_registry;
    auncient_sdk_init_transfluxor_registry(&shared_registry);

    auncient_transfluxor_word_t lock_word;
    bool ok = auncient_sdk_compile_transfluxor_word(
        &lock_word,
        "SPK_LOCK_SCSI",
        1,       // Word ID
        440.0,   // F1
        880.0,   // F2
        0.4,     // Decay
        0x10,    // WMQ Lock
        0x00     // ABI noop
    );
    assert(ok == true);

    ok = auncient_sdk_register_transfluxor_word(&shared_registry, &lock_word);
    assert(ok == true);
    printf("   ✓ Shared vocabulary populated with '%s'.\n", lock_word.name);

    // 2. Setup Coaxial Socket Channel (Inter-Agent Socket Loopback)
    printf("[DEPLOYMENT] Allocating coaxial inter-agent socket pairs...\n");
    int socket_fds[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds) < 0) {
        printf("   ✗ Failed to allocate socketpair.\n");
        return 1;
    }
    printf("   ✓ Socket loops established. FDs: %d <==> %d\n", socket_fds[0], socket_fds[1]);

    // 3. Agent A writes a Transfluxor word to the socket channel
    printf("[AGENT A] Dispatching compiled Transfluxor word over coaxial socket...\n");
    ssize_t written = write(socket_fds[0], &lock_word, sizeof(auncient_transfluxor_word_t));
    assert(written == sizeof(auncient_transfluxor_word_t));

    // 4. Agent B reads from the socket and resolves it against the Shared Registry
    printf("[AGENT B] Ingesting wave packet from coaxial socket...\n");
    auncient_transfluxor_word_t rx_word;
    ssize_t read_bytes = read(socket_fds[1], &rx_word, sizeof(auncient_transfluxor_word_t));
    assert(read_bytes == sizeof(auncient_transfluxor_word_t));

    // Verify properties and name
    assert(strcmp(rx_word.name, "SPK_LOCK_SCSI") == 0);
    assert(rx_word.word_id == 1);

    // 5. Automated Pre-Commit Hash Verification
    printf("[AGENT B] Executing pre-commit hash verification audit...\n");
    uint64_t target_hash = auncient_sdk_calculate_transfluxor_hash(rx_word.f1, rx_word.f2, rx_word.decay, rx_word.word_id);
    uint64_t registry_hash = auncient_sdk_calculate_transfluxor_hash(
        shared_registry.registered_words[0].f1,
        shared_registry.registered_words[0].f2,
        shared_registry.registered_words[0].decay,
        shared_registry.registered_words[0].word_id
    );
    assert(target_hash == registry_hash);
    printf("   ✓ Hash verified: %lu == %lu (100.0%% matching success).\n", target_hash, registry_hash);

    // 6. OS Gating Verification
    double feedback_freq = 0.0;
    bool resolved = auncient_sdk_dispatch_transfluxor_word(&shared_registry, &rx_word, &feedback_freq);
    assert(resolved == true);
    assert(feedback_freq == 523.25); // Success tone
    printf("   ✓ OS dispatch gating resolved successfully. Emitted feedback frequency: %.2fHz\n", feedback_freq);

    // Cleanup sockets
    close(socket_fds[0]);
    close(socket_fds[1]);

    printf("=============================================================\n");
    printf("AGENT DEPLOYMENT & INTEGRATION TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

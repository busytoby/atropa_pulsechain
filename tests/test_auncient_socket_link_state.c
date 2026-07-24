#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>

typedef struct {
    uint64_t sequence_number;
    uint32_t age;
    uint32_t source_node;
    uint32_t link_metric;
} link_state_update_t;

typedef struct {
    uint64_t last_sequence;
    uint32_t link_metric;
    bool sync_requested;
} routing_entry_t;

// -------------------------------------------------------------
// Welch Link-State Validation logic
// -------------------------------------------------------------
bool validate_socket_link_state(routing_entry_t *entry, const link_state_update_t *update) {
    entry->sync_requested = false;

    // Detect erratic sequence jumps
    if (update->sequence_number > entry->last_sequence + 1000) {
        entry->sync_requested = true;
        return false;
    }

    // Monotonically increasing sequence check
    if (update->sequence_number > entry->last_sequence) {
        entry->last_sequence = update->sequence_number;
        entry->link_metric = update->link_metric;
        return true;
    }

    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SOCKET LINK-STATE ROUTING SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    int sv[2];
    int status = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(status == 0);

    routing_entry_t receiver_entry = {
        .last_sequence = 200,
        .link_metric = 50,
        .sync_requested = false
    };

    // 1. Send Valid Update from SV[0] to SV[1]
    link_state_update_t tx_update = {
        .sequence_number = 205,
        .age = 1,
        .source_node = 1,
        .link_metric = 45
    };

    printf("[TEST] Transmitting link-state update over UNIX socket pair...\n");
    fflush(stdout);

    ssize_t sent = write(sv[0], &tx_update, sizeof(tx_update));
    assert(sent == sizeof(tx_update));

    // Receive and process update
    link_state_update_t rx_update;
    ssize_t received = read(sv[1], &rx_update, sizeof(rx_update));
    assert(received == sizeof(rx_update));

    bool ok = validate_socket_link_state(&receiver_entry, &rx_update);
    assert(ok == true);
    assert(receiver_entry.last_sequence == 205);
    assert(receiver_entry.link_metric == 45);
    printf("   ✓ Socket packet received. Welch sequence validation passed.\n");
    fflush(stdout);

    // 2. Send Stale Update
    link_state_update_t stale_tx = {
        .sequence_number = 204, // Stale
        .age = 2,
        .source_node = 1,
        .link_metric = 99
    };

    printf("[TEST] Transmitting stale link-state update over socket...\n");
    fflush(stdout);

    sent = write(sv[0], &stale_tx, sizeof(stale_tx));
    assert(sent == sizeof(stale_tx));

    received = read(sv[1], &rx_update, sizeof(rx_update));
    assert(received == sizeof(rx_update));

    ok = validate_socket_link_state(&receiver_entry, &rx_update);
    assert(ok == false); // Stale discarded
    assert(receiver_entry.last_sequence == 205); // Remains at latest valid
    printf("   ✓ Socket stale update successfully trapped and discarded.\n");
    fflush(stdout);

    close(sv[0]);
    close(sv[1]);

    printf("=============================================================\n");
    printf("SOCKET LINK-STATE TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

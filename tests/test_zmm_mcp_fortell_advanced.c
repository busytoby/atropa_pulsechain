#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_PEERS 4
#define QUEUE_SIZE 8

typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// 1. Wait for Idle Queue (Multi-user Safety)
typedef struct {
    PPN queue[QUEUE_SIZE];
    size_t head;
    size_t tail;
    size_t count;
    bool is_locked;
} ChannelLockManager;

ChannelLockManager g_lock_manager = { { {0, 0} }, 0, 0, 0, false };

bool enqueue_transaction(PPN ppn) {
    if (g_lock_manager.count >= QUEUE_SIZE) {
        return false; // Queue full
    }
    g_lock_manager.queue[g_lock_manager.tail] = ppn;
    g_lock_manager.tail = (g_lock_manager.tail + 1) % QUEUE_SIZE;
    g_lock_manager.count++;
    return true;
}

PPN dequeue_transaction(void) {
    PPN empty = { 0, 0 };
    if (g_lock_manager.count == 0) {
        return empty;
    }
    PPN ppn = g_lock_manager.queue[g_lock_manager.head];
    g_lock_manager.head = (g_lock_manager.head + 1) % QUEUE_SIZE;
    g_lock_manager.count--;
    return ppn;
}

bool request_channel_access(PPN ppn) {
    if (g_lock_manager.is_locked) {
        printf("   [Wait for Idle] Channel busy. Enqueueing transaction from PPN [%u,%u]...\n",
               ppn.project, ppn.programmer);
        enqueue_transaction(ppn);
        return false; // Wait for idle
    }
    g_lock_manager.is_locked = true;
    printf("   [Lock Acquired] Channel locked by PPN [%u,%u].\n", ppn.project, ppn.programmer);
    return true;
}

void release_channel_access(void) {
    printf("   [Lock Released] Channel freed.\n");
    g_lock_manager.is_locked = false;
    if (g_lock_manager.count > 0) {
        PPN next = dequeue_transaction();
        printf("   [Queue Dispatch] Auto-starting queued PPN [%u,%u].\n", next.project, next.programmer);
        g_lock_manager.is_locked = true;
    }
}

// 2. Loopback Echo (Multi-user Teleconferencing verification)
bool perform_loopback_echo_test(PPN peer, uint32_t send_ping, uint32_t *out_echo) {
    // Simulates dynamic loopback test verifying connectivity
    *out_echo = send_ping ^ 0xAAAAAAAA; // Dynamic loop back transform
    printf("   [Loopback Echo] Peer [%u,%u] - Ping: 0x%08X -> Echo: 0x%08X\n",
           peer.project, peer.programmer, send_ping, *out_echo);
    return true;
}

// 3. Cross-Talk Auditor (Safe Multiplexing check)
typedef struct {
    uint8_t peer_segments[MAX_PEERS][64]; // Isolated memory slots
} MultiplexedMemoryLayout;

bool audit_segment_crossover(const MultiplexedMemoryLayout *layout, size_t target_peer, uint8_t fill_val) {
    // Verifies that other peer slots were not crossed-over/written during layout updates
    for (size_t p = 0; p < MAX_PEERS; p++) {
        if (p == target_peer) continue;
        for (size_t i = 0; i < 64; i++) {
            if (layout->peer_segments[p][i] == fill_val) {
                printf("   [CROSS-TALK DETECTED] Leak from Peer %zu to Peer %zu at index %zu!\n",
                       target_peer, p, i);
                return false; // Cross-talk detected
            }
        }
    }
    return true; // Safe
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: FORTELL MULTIPLEXING AND LOCKING TESTS\n");
    printf("=============================================================\n");

    PPN peer1 = { 10, 5 };
    PPN peer2 = { 12, 6 };

    // 1. Test Channel Busy Lock and Queueing
    printf("1. Testing multi-user channel locking safety (Wait for Idle)...\n");
    bool access1 = request_channel_access(peer1);
    assert(access1 == true); // Allowed to grab lock
    assert(g_lock_manager.is_locked == true);

    bool access2 = request_channel_access(peer2);
    assert(access2 == false); // Blocked and queued
    assert(g_lock_manager.count == 1);

    release_channel_access(); // Release peer1 and auto-dispatch peer2
    assert(g_lock_manager.is_locked == true); // Lock held by peer2
    assert(g_lock_manager.count == 0);

    g_lock_manager.is_locked = false; // Reset lock
    printf("   ✓ Multi-user channel queuing completed successfully.\n\n");

    // 2. Test Loopback Teleconferencing Verification
    printf("2. Testing loopback connectivity checks...\n");
    uint32_t echo_val = 0;
    bool conn_ok = perform_loopback_echo_test(peer1, 0x55555555, &echo_val);
    assert(conn_ok == true);
    assert(echo_val == (0x55555555 ^ 0xAAAAAAAA));
    printf("   ✓ Connection loopback test verified.\n\n");

    // 3. Test Cross-Talk Safe Multiplexing Auditor
    printf("3. Testing safe multiplexing cross-talk checks...\n");
    MultiplexedMemoryLayout layout;
    memset(&layout, 0, sizeof(layout));

    // Fill peer 0 segment with test byte 0xFF
    memset(layout.peer_segments[0], 0xFF, 64);

    // Verify peer 1-3 were not affected (Safe)
    bool is_safe = audit_segment_crossover(&layout, 0, 0xFF);
    assert(is_safe == true);
    printf("   ✓ Multiplexed slots isolated: No cross-talk detected.\n");

    // Simulate cross-talk leak
    layout.peer_segments[1][10] = 0xFF; // Leak to peer 1 segment
    is_safe = audit_segment_crossover(&layout, 0, 0xFF);
    assert(is_safe == false);
    printf("   ✓ Cross-talk auditor successfully intercepted boundary overflow.\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT FORTELL ADVANCED TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

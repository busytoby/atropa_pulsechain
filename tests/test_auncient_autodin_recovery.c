#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <arpa/inet.h>

#define MAX_TRANSACTIONS 8

// VM Registers representing the Auncient VM State
typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t chin;
    uint64_t dynamo;
    uint32_t last_seq;
} auncient_registers_t;

// Transaction definition
typedef struct {
    uint32_t seq;
    char payload[32];
} autodin_tx_t;

// Session cache representing the AUTODIN transaction registry
typedef struct {
    uint64_t address_key; // STANAG MAC (48b) + Port (16b)
    auncient_registers_t saved_regs;
    autodin_tx_t tx_log[MAX_TRANSACTIONS];
    uint32_t log_count;
    bool is_active;
} autodin_session_t;

// Global persistent registry mimicking the trie database mapping
static autodin_session_t g_autodin_registry[16];
static uint32_t g_session_count = 0;

// Find or register session in AUTODIN by STANAG address
static autodin_session_t *find_or_create_session(uint64_t address_key) {
    for (uint32_t i = 0; i < g_session_count; i++) {
        if (g_autodin_registry[i].address_key == address_key) {
            return &g_autodin_registry[i];
        }
    }
    assert(g_session_count < 16);
    autodin_session_t *s = &g_autodin_registry[g_session_count++];
    s->address_key = address_key;
    s->is_active = true;
    s->log_count = 0;
    return s;
}

// Log a transaction at the AUTODIN gateway
static void log_autodin_tx(autodin_session_t *s, uint32_t seq, const char *payload) {
    if (s->log_count < MAX_TRANSACTIONS) {
        s->tx_log[s->log_count].seq = seq;
        strncpy(s->tx_log[s->log_count].payload, payload, 31);
        s->tx_log[s->log_count].payload[31] = '\0';
        s->log_count++;
    }
}

// -------------------------------------------------------------
// Dormancy & Recovery Pipeline
// -------------------------------------------------------------

// Save client state in AUTODIN when client goes dormant
void autodin_client_suspend(uint64_t address_key, const auncient_registers_t *regs) {
    autodin_session_t *s = find_or_create_session(address_key);
    s->saved_regs = *regs;
    s->is_active = false; // Client goes dormant
    printf("   [AUTODIN] Saved state for client 0x%016lX (Seq: %d). Client is now dormant.\n",
           address_key, regs->last_seq);
    fflush(stdout);
}

// Recover client state and replay missing transactions upon reconnection
void autodin_client_resume(uint64_t address_key, auncient_registers_t *out_regs) {
    autodin_session_t *s = find_or_create_session(address_key);
    *out_regs = s->saved_regs;
    s->is_active = true;
    
    printf("   [AUTODIN] Restored client registers (Base: %ld, Chin: %ld).\n", 
           out_regs->base, out_regs->chin);
    fflush(stdout);

    // Replay queued transactions since the last processed sequence
    uint32_t last_seq = out_regs->last_seq;
    for (uint32_t i = 0; i < s->log_count; i++) {
        if (s->tx_log[i].seq > last_seq) {
            printf("   [AUTODIN] Replaying transaction Seq %d: %s\n", 
                   s->tx_log[i].seq, s->tx_log[i].payload);
            fflush(stdout);
            
            // Execute state update
            if (strncmp(s->tx_log[i].payload, "SET_CHIN=", 9) == 0) {
                out_regs->chin = strtoull(s->tx_log[i].payload + 9, NULL, 10);
            }
            out_regs->last_seq = s->tx_log[i].seq;
        }
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN TRANSACTION STATE RECOVERY VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    uint64_t client_addr = 0x00112233445503E8ULL; // MAC + Port 1000
    auncient_registers_t client_regs = {
        .base = 500,
        .secret = 12345,
        .signal = 999,
        .chin = 42,
        .dynamo = 777,
        .last_seq = 1
    };

    // 1. Client completes sequence 1 and goes dormant
    printf("[STAGE 1] Suspending active client session...\n");
    fflush(stdout);
    autodin_client_suspend(client_addr, &client_regs);

    // 2. Gateway receives transactions while client is offline
    printf("[STAGE 2] Gateway queuing incoming transactions during dormancy...\n");
    fflush(stdout);
    autodin_session_t *s = find_or_create_session(client_addr);
    log_autodin_tx(s, 2, "SET_CHIN=99");
    log_autodin_tx(s, 3, "SET_CHIN=150");

    // 3. Client reconnects, restores state, and plays back transactions
    printf("[STAGE 3] Reconnecting client and executing recovery...\n");
    fflush(stdout);
    auncient_registers_t recovered_regs;
    autodin_client_resume(client_addr, &recovered_regs);

    // Verify recovery accuracy
    assert(recovered_regs.chin == 150);
    assert(recovered_regs.last_seq == 3);
    printf("   ✓ Recovery verification passed: state synchronized with queue.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN RECOVERY TESTS COMPLETED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

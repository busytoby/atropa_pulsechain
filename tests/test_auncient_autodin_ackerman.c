#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

typedef enum {
    LEVEL_LOW,
    LEVEL_HIGH
} security_level_t;

// Simulated AUTODIN Transaction Packet
typedef struct {
    uint32_t transaction_id;
    security_level_t data_clearance;
    char payload[32];
    char consensus_sig[4]; // e.g. "110", "100"
} autodin_transaction_t;

// -------------------------------------------------------------
// AUTODIN Ackerman Consensus Declassification Gating
// -------------------------------------------------------------
bool route_autodin_transaction(const autodin_transaction_t *tx, security_level_t target_domain, gate_state_t *router_gate) {
    *router_gate = CUTOFF_STATE;

    // Normal secure information flow (no declassification needed)
    if (tx->data_clearance == LEVEL_LOW || target_domain == LEVEL_HIGH) {
        *router_gate = CONDUC_STATE;
        return true;
    }

    // Attempting HIGH -> LOW transfer: Requires Ackerman consensus approvals
    uint32_t approvals = 0;
    if (tx->consensus_sig[0] == '1') approvals++;
    if (tx->consensus_sig[1] == '1') approvals++;
    if (tx->consensus_sig[2] == '1') approvals++;

    // Social-Technical majority check: requires at least 2 of 3 approvals to open the gate
    if (approvals >= 2) {
        *router_gate = CONDUC_STATE;
        return true; // Declassification approved. Routing open.
    }

    return false; // Insufficient consensus. Blocked.
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AUTODIN ACKERMAN DECLASSIFICATION ROUTER SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    gate_state_t router_gate = CUTOFF_STATE;

    // 1. Compliant Case: HIGH -> LOW with majority approvals ("110") -> Should pass
    autodin_transaction_t tx_approved = {
        .transaction_id = 1001,
        .data_clearance = LEVEL_HIGH,
        .payload = "SECRET_DECRYPT_KEY",
        .consensus_sig = "110"
    };

    printf("[TEST] Routing HIGH -> LOW with majority consensus (110)...\n");
    fflush(stdout);
    bool ok = route_autodin_transaction(&tx_approved, LEVEL_LOW, &router_gate);
    assert(ok == true);
    assert(router_gate == CONDUC_STATE);
    printf("   ✓ Transaction approved and routed successfully.\n");
    fflush(stdout);

    // 2. Reject Case: HIGH -> LOW with insufficient approvals ("100") -> Should block
    autodin_transaction_t tx_blocked = {
        .transaction_id = 1002,
        .data_clearance = LEVEL_HIGH,
        .payload = "TOP_SECRET_CORE",
        .consensus_sig = "100"
    };

    printf("[TEST] Routing HIGH -> LOW with insufficient consensus (100)...\n");
    fflush(stdout);
    ok = route_autodin_transaction(&tx_blocked, LEVEL_LOW, &router_gate);
    assert(ok == false);
    assert(router_gate == CUTOFF_STATE);
    printf("   ✓ Security breach attempt blocked. Path cutoff.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AUTODIN ACKERMAN TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// PPN Account Structure representing a peer
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Peer Gas Record
typedef struct {
    PPN ppn;
    uint64_t gas_balance;
} GasRecord;

// Central Server State
GasRecord g_our_server_balances[2];

// Peer Local Servers
GasRecord g_peer_a_local_balances[1]; // Balance of B on A
GasRecord g_peer_b_local_balances[1]; // Balance of A on B

// Proxy dispatcher function to query balances
bool query_proxy_balance(PPN caller, PPN target, const char *server_type, uint64_t *out_balance, const char **out_err) {
    // Deduct gas from caller on OUR central server first (proxy query gas cost = 15)
    const uint64_t proxy_fee = 15;
    bool caller_found = false;
    for (int i = 0; i < 2; i++) {
        if (g_our_server_balances[i].ppn.project == caller.project &&
            g_our_server_balances[i].ppn.programmer == caller.programmer) {
            caller_found = true;
            if (g_our_server_balances[i].gas_balance < proxy_fee) {
                *out_err = "REVERT: INSUFFICIENT_GAS_ON_CENTRAL_SERVER";
                return false;
            }
            g_our_server_balances[i].gas_balance -= proxy_fee;
            break;
        }
    }
    if (!caller_found) {
        *out_err = "REVERT: CALLER_NOT_REGISTERED_ON_CENTRAL_SERVER";
        return false;
    }

    // Route query based on target server type
    if (strcmp(server_type, "central") == 0) {
        for (int i = 0; i < 2; i++) {
            if (g_our_server_balances[i].ppn.project == target.project &&
                g_our_server_balances[i].ppn.programmer == target.programmer) {
                *out_balance = g_our_server_balances[i].gas_balance;
                return true;
            }
        }
    } else if (strcmp(server_type, "peer_local") == 0) {
        // Query Peer A's local server for Peer B's balance
        if (target.project == 1 && target.programmer == 2) { // Target is A (meaning we query B on A)
            if (caller.project == 10 && caller.programmer == 5) {
                *out_balance = g_peer_a_local_balances[0].gas_balance;
                return true;
            }
        }
        // Query Peer B's local server for Peer A's balance
        if (target.project == 10 && target.programmer == 5) { // Target is B (meaning we query A on B)
            if (caller.project == 1 && caller.programmer == 2) {
                *out_balance = g_peer_b_local_balances[0].gas_balance;
                return true;
            }
        }
    }

    *out_err = "REVERT: TARGET_BALANCE_NOT_FOUND";
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: P2P MCP PROXY GAS BALANCE UNIT TESTS\n");
    printf("=============================================================\n");

    PPN peer_a = { .project = 1,  .programmer = 2 };
    PPN peer_b = { .project = 10, .programmer = 5 };

    // 1. Initialize gas allocations on OUR central MCP server (1000 each)
    g_our_server_balances[0] = (GasRecord){ .ppn = peer_a, .gas_balance = 1000 };
    g_our_server_balances[1] = (GasRecord){ .ppn = peer_b, .gas_balance = 1000 };

    // 2. Initialize gas allocations on the peers' local MCP servers
    g_peer_a_local_balances[0] = (GasRecord){ .ppn = peer_b, .gas_balance = 500 }; // B's balance on A's server
    g_peer_b_local_balances[0] = (GasRecord){ .ppn = peer_a, .gas_balance = 300 }; // A's balance on B's server

    uint64_t bal_a_central = 0;
    uint64_t bal_b_central = 0;
    uint64_t bal_a_on_b = 0;
    uint64_t bal_b_on_a = 0;
    const char *err = NULL;

    printf("1. Querying Peer A's balance on OUR central MCP server (requested by B)...\n");
    bool res = query_proxy_balance(peer_b, peer_a, "central", &bal_a_central, &err);
    assert(res == true);
    assert(bal_a_central == 1000);
    printf("   ✓ Balance found: %lu Gas\n\n", bal_a_central);

    printf("2. Querying Peer B's balance on OUR central MCP server (requested by A)...\n");
    res = query_proxy_balance(peer_a, peer_b, "central", &bal_b_central, &err);
    assert(res == true);
    assert(bal_b_central == 985); // 1000 - 15 query fee
    printf("   ✓ Balance found: %lu Gas\n\n", bal_b_central);

    printf("3. Querying Peer A's balance on Peer B's local MCP server (requested by A)...\n");
    res = query_proxy_balance(peer_a, peer_b, "peer_local", &bal_a_on_b, &err);
    assert(res == true);
    assert(bal_a_on_b == 300);
    printf("   ✓ Balance found: %lu Gas\n\n", bal_a_on_b);

    printf("4. Querying Peer B's balance on Peer A's local MCP server (requested by B)...\n");
    res = query_proxy_balance(peer_b, peer_a, "peer_local", &bal_b_on_a, &err);
    assert(res == true);
    assert(bal_b_on_a == 500);
    printf("   ✓ Balance found: %lu Gas\n\n", bal_b_on_a);

    // Verify final caller balance deductions on OUR central server
    // Peer A executed 2 queries (spent 30 gas): balance should be 970
    // Peer B executed 2 queries (spent 30 gas): balance should be 970
    assert(g_our_server_balances[0].gas_balance == 970);
    assert(g_our_server_balances[1].gas_balance == 970);
    printf("5. Verifying central server gas balances after operations...\n");
    printf("   ✓ Peer A Central Balance: %lu\n", g_our_server_balances[0].gas_balance);
    printf("   ✓ Peer B Central Balance: %lu\n\n", g_our_server_balances[1].gas_balance);

    printf("=============================================================\n");
    printf("AUNCIENT P2P PROXY GAS BALANCE TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

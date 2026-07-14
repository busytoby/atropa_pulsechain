#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define NUM_LEDGERS           2
#define MAX_BLOCKS            4
#define BLOCK_DATA_SIZE      32
#define ACCUMULATOR_LIMIT    500

typedef struct {
    uint32_t index;
    uint32_t peer_id;
    char data[BLOCK_DATA_SIZE];
    uint64_t hash;
} Block;

typedef struct {
    Block chain[MAX_BLOCKS];
    size_t length;
    uint64_t accumulated_weight;
} PartitionedLedger;

// Stateful Firewall Rule
typedef struct {
    uint32_t peer_id;
    uint32_t allowed_partition_mask; // Bitmask of allowed partitions (bit 0 = Partition 0, bit 1 = Partition 1)
    bool is_blocked;
} FirewallRule;

// Simple custom hash
uint64_t compute_ledger_hash(const Block *b) {
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + b->index;
    hash = ((hash << 5) + hash) + b->peer_id;
    for (size_t i = 0; i < strlen(b->data); i++) {
        hash = ((hash << 5) + hash) + b->data[i];
    }
    return hash;
}

// Executes firewall check and appends transaction to the targeted ledger partition
bool process_secure_transaction(PartitionedLedger *ledgers, const FirewallRule *rules, size_t rule_count, 
                                 uint32_t partition_id, uint32_t peer_id, const char *data, uint64_t weight) {
    assert(partition_id < NUM_LEDGERS);
    PartitionedLedger *target_ledger = &ledgers[partition_id];

    // 1. Stateful Firewall Inspection (ACL validation)
    bool authorized = false;
    for (size_t i = 0; i < rule_count; i++) {
        if (rules[i].peer_id == peer_id) {
            if (!rules[i].is_blocked && (rules[i].allowed_partition_mask & (1 << partition_id))) {
                authorized = true;
            }
            break;
        }
    }

    if (!authorized) {
        printf("   [Firewall BLOCK] Transaction from Peer %u to Partition %u REJECTED (ACL Violation)\n", 
               peer_id, partition_id);
        return false;
    }

    // 2. Partitioned Accumulator Limit Check
    if (target_ledger->accumulated_weight + weight > ACCUMULATOR_LIMIT) {
        printf("   [Firewall BLOCK] Transaction weight %lu exceeds Partition %u Accumulator limit (Current: %lu)\n", 
               weight, partition_id, target_ledger->accumulated_weight);
        return false;
    }

    // 3. Commit to Partitioned Ledger
    assert(target_ledger->length < MAX_BLOCKS);
    Block b;
    b.index = (uint32_t)target_ledger->length;
    b.peer_id = peer_id;
    strncpy(b.data, data, BLOCK_DATA_SIZE - 1);
    b.data[BLOCK_DATA_SIZE - 1] = '\0';
    b.hash = compute_ledger_hash(&b);

    target_ledger->chain[target_ledger->length] = b;
    target_ledger->length++;
    target_ledger->accumulated_weight += weight;

    printf("   [Firewall ACCEPT] Committed transaction to Partition %u by Peer %u (Accumulated Weight: %lu)\n", 
           partition_id, peer_id, target_ledger->accumulated_weight);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT MULTI-TENANT LEDGER AND CYCLE-LEVEL FIREWALL\n");
    printf("=============================================================\n");

    // Initialize 2 separate ledger partitions
    PartitionedLedger ledgers[NUM_LEDGERS];
    memset(ledgers, 0, sizeof(ledgers));

    // Initialize Firewall ACL Rules
    // Peer 500: Allowed to write to Partition 0 only
    // Peer 501: Allowed to write to both Partition 0 and 1
    // Peer 502: Explicitly blocked
    FirewallRule rules[] = {
        {.peer_id = 500, .allowed_partition_mask = 1 << 0, .is_blocked = false},
        {.peer_id = 501, .allowed_partition_mask = (1 << 0) | (1 << 1), .is_blocked = false},
        {.peer_id = 502, .allowed_partition_mask = (1 << 0), .is_blocked = true}
    };
    size_t rule_count = sizeof(rules) / sizeof(rules[0]);

    // 1. Peer 500 writes to Partition 0 (Authorized)
    printf("1. Peer 500 writing to Partition 0...\n");
    bool tx1 = process_secure_transaction(ledgers, rules, rule_count, 0, 500, "Tx Block A", 100);
    assert(tx1 == true);
    assert(ledgers[0].length == 1);
    printf("   ✓ Write accepted.\n\n");

    // 2. Peer 500 writes to Partition 1 (Unauthorized)
    printf("2. Peer 500 writing to Partition 1 (Expected Firewall Reject)...\n");
    bool tx2 = process_secure_transaction(ledgers, rules, rule_count, 1, 500, "Tx Block B", 100);
    assert(tx2 == false);
    assert(ledgers[1].length == 0);
    printf("   ✓ Mismatched partition write blocked by firewall.\n\n");

    // 3. Peer 501 writes to Partition 1 (Authorized)
    printf("3. Peer 501 writing to Partition 1...\n");
    bool tx3 = process_secure_transaction(ledgers, rules, rule_count, 1, 501, "Tx Block C", 200);
    assert(tx3 == true);
    assert(ledgers[1].length == 1);
    printf("   ✓ Write accepted.\n\n");

    // 4. Blocked peer writes to Partition 0 (Expected Block)
    printf("4. Blocked Peer 502 writing to Partition 0 (Expected Firewall Reject)...\n");
    bool tx4 = process_secure_transaction(ledgers, rules, rule_count, 0, 502, "Tx Block D", 50);
    assert(tx4 == false);
    printf("   ✓ Blocked peer rejected successfully.\n\n");

    // 5. Check isolated accumulator limit
    // Peer 501 attempts to write weight = 400 to Partition 1 (exceeds limit 500 since current is 200)
    printf("5. Peer 501 writing heavy transaction to Partition 1 (Expected Accumulator Reject)...\n");
    bool tx5 = process_secure_transaction(ledgers, rules, rule_count, 1, 501, "Tx Block E", 400);
    assert(tx5 == false);
    printf("   ✓ Partitioned accumulator preemption verified.\n\n");

    printf("=============================================================\n");
    printf("MULTI-TENANT FIREWALL & LEDGER TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

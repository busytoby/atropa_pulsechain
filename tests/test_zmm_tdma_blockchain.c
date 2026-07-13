#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define NUM_PEERS 4
#define BLOCK_DATA_SIZE 64

typedef struct {
    uint32_t index;
    uint32_t slot_id;
    uint32_t validator_peer_id;
    char data[BLOCK_DATA_SIZE];
    uint64_t prev_hash;
    uint64_t hash;
} Block;

typedef struct {
    Block chain[16];
    size_t length;
} ImmutableLedger;

// Simple custom cryptographic hash simulation (DJB2 variations)
uint64_t calculate_block_hash(const Block *b) {
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + b->index;
    hash = ((hash << 5) + hash) + b->slot_id;
    hash = ((hash << 5) + hash) + b->validator_peer_id;
    hash = ((hash << 5) + hash) + b->prev_hash;
    for (size_t i = 0; i < strlen(b->data); i++) {
        hash = ((hash << 5) + hash) + b->data[i];
    }
    return hash;
}

// Appends block to the immutable ledger, enforcing TDMA schedule constraints
bool append_block(ImmutableLedger *ledger, uint32_t slot_id, uint32_t peer_id, const char *data) {
    // TDMA constraint: Peer is only authorized to validate blocks if it owns the current slot
    uint32_t expected_peer_id = 200 + (slot_id % NUM_PEERS);
    if (peer_id != expected_peer_id) {
        printf("   [Ledger Reject] Peer %u is unauthorized to propose block for Slot %u (Expected Peer %u)\n", 
               peer_id, slot_id, expected_peer_id);
        return false;
    }

    Block b;
    b.index = (uint32_t)ledger->length;
    b.slot_id = slot_id;
    b.validator_peer_id = peer_id;
    strncpy(b.data, data, BLOCK_DATA_SIZE - 1);
    b.data[BLOCK_DATA_SIZE - 1] = '\0';
    b.prev_hash = (ledger->length == 0) ? 0 : ledger->chain[ledger->length - 1].hash;
    b.hash = calculate_block_hash(&b);

    ledger->chain[ledger->length] = b;
    ledger->length++;
    
    printf("   [Ledger Accept] Block #%u committed by Peer %u at Slot %u (Hash: 0x%lX)\n", 
           b.index, peer_id, slot_id, b.hash);
    return true;
}

// Audits ledger integrity: verifies block hashes and parent-child links
bool verify_ledger_integrity(const ImmutableLedger *ledger) {
    for (size_t i = 0; i < ledger->length; i++) {
        const Block *b = &ledger->chain[i];
        
        // 1. Recalculate block hash and compare
        if (calculate_block_hash(b) != b->hash) {
            return false;
        }

        // 2. Verify parent hash match (except genesis block)
        if (i > 0) {
            if (b->prev_hash != ledger->chain[i - 1].hash) {
                return false;
            }
        }
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TDMA-COORDINATED IMMUTABLE BLOCK STORAGE\n");
    printf("=============================================================\n");

    ImmutableLedger ledger;
    ledger.length = 0;

    // 1. Validate block proposals matching TDMA schedule
    printf("1. Proposing blocks under TDMA schedule constraints...\n");
    // Slot 0 -> Peer 200 (Valid)
    bool s0 = append_block(&ledger, 0, 200, "Transaction Block 0");
    assert(s0 == true);

    // Slot 1 -> Peer 202 (Invalid attempt: Slot 1 is allocated to Peer 201)
    bool s1_fail = append_block(&ledger, 1, 202, "Malicious Block 1");
    assert(s1_fail == false);

    // Slot 1 -> Peer 201 (Valid)
    bool s1 = append_block(&ledger, 1, 201, "Transaction Block 1");
    assert(s1 == true);

    // Slot 2 -> Peer 202 (Valid)
    bool s2 = append_block(&ledger, 2, 202, "Transaction Block 2");
    assert(s2 == true);

    // Assert that the ledger has committed exactly 3 valid blocks
    assert(ledger.length == 3);
    printf("   ✓ Block proposal validation succeeded.\n\n");

    // 2. Audit ledger integrity
    printf("2. Auditing chain integrity...\n");
    bool integrity = verify_ledger_integrity(&ledger);
    assert(integrity == true);
    printf("   ✓ Chain integrity verified successfully.\n\n");

    // 3. Test Immutability (Simulating tampering)
    printf("3. Simulating block tampering on Block 1...\n");
    // Modify data in Block 1
    strcpy(ledger.chain[1].data, "Tampered Transaction Data");
    
    // Integrity check must fail
    bool tampered_integrity = verify_ledger_integrity(&ledger);
    assert(tampered_integrity == false);
    printf("   ✓ Tampering detected: Chain invalidation confirmed.\n\n");

    printf("=============================================================\n");
    printf("IMMUTABLE BLOCK STORAGE TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

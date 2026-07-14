#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_FRAMES 8

typedef struct {
    uint32_t frame_index;
    uint8_t hpos_players[4];     // Player positions (Thor, Thyra, Merlin, Questor)
    uint8_t collision_registers; // GTIA collision mask values
    uint64_t prev_hash;
    uint64_t hash;
} PmgFrameBlock;

typedef struct {
    PmgFrameBlock ledger[MAX_FRAMES];
    size_t length;
} PmgImmutableChain;

// Simple custom cryptographic hash for PMG state blocks
uint64_t compute_pmg_frame_hash(const PmgFrameBlock *b) {
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + b->frame_index;
    hash = ((hash << 5) + hash) + b->collision_registers;
    hash = ((hash << 5) + hash) + b->prev_hash;
    for (int i = 0; i < 4; i++) {
        hash = ((hash << 5) + hash) + b->hpos_players[i];
    }
    return hash;
}

// Appends a new PMG state frame to the immutable ledger
void commit_pmg_frame(PmgImmutableChain *chain, const uint8_t positions[4], uint8_t collisions) {
    assert(chain->length < MAX_FRAMES);

    PmgFrameBlock b;
    b.frame_index = (uint32_t)chain->length;
    memcpy(b.hpos_players, positions, 4);
    b.collision_registers = collisions;
    b.prev_hash = (chain->length == 0) ? 0 : chain->ledger[chain->length - 1].hash;
    b.hash = compute_pmg_frame_hash(&b);

    chain->ledger[chain->length] = b;
    chain->length++;

    printf("   [PMG Commit] Frame #%u committed. Positions: [%u, %u, %u, %u], Collisions: 0x%X (Hash: 0x%lX)\n", 
           b.frame_index, b.hpos_players[0], b.hpos_players[1], b.hpos_players[2], b.hpos_players[3], 
           b.collision_registers, b.hash);
}

// Audits the PMG history ledger for integrity
bool verify_pmg_chain_integrity(const PmgImmutableChain *chain) {
    for (size_t i = 0; i < chain->length; i++) {
        const PmgFrameBlock *b = &chain->ledger[i];

        // 1. Recalculate block hash
        if (compute_pmg_frame_hash(b) != b->hash) {
            return false;
        }

        // 2. Verify parent hash link
        if (i > 0) {
            if (b->prev_hash != chain->ledger[i - 1].hash) {
                return false;
            }
        }
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT IMMUTABLE PLAYER-MISSILE GRAPHICS LEDGER\n");
    printf("=============================================================\n");

    PmgImmutableChain chain = {
        .length = 0
    };

    // 1. Commit active frame states (simulating game loop progression)
    printf("1. Committing PMG coordinate frames to ledger...\n");
    uint8_t pos_f0[4] = {100, 110, 120, 130};
    commit_pmg_frame(&chain, pos_f0, 0x00);

    uint8_t pos_f1[4] = {102, 110, 120, 130}; // Thor moves
    commit_pmg_frame(&chain, pos_f1, 0x01);    // Collision active

    uint8_t pos_f2[4] = {104, 111, 120, 130}; // Thor and Valkyrie move
    commit_pmg_frame(&chain, pos_f2, 0x00);

    assert(chain.length == 3);
    printf("   ✓ Game frames committed.\n\n");

    // 2. Audit ledger integrity
    printf("2. Auditing PMG ledger integrity...\n");
    bool integrity = verify_pmg_chain_integrity(&chain);
    assert(integrity == true);
    printf("   ✓ Ledger integrity verified successfully.\n\n");

    // 3. Test Immutability by tampering with historical frame coordinates
    printf("3. Tampering with Thor's position in Frame #1...\n");
    chain.ledger[1].hpos_players[0] = 99; // Change coordinate value from 102 to 99

    bool tampered_integrity = verify_pmg_chain_integrity(&chain);
    assert(tampered_integrity == false);
    printf("   ✓ Tampering detected: PMG ledger invalidation confirmed.\n\n");

    printf("=============================================================\n");
    printf("IMMUTABLE PMG LEDGER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

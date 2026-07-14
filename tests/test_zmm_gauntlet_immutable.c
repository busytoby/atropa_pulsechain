#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

#define MAX_STATES 16

typedef struct {
    uint32_t level;
    uint32_t score;
    int32_t health;
    uint8_t player_x;
    uint8_t player_y;
    uint8_t collision_mask;
} GauntletState;

typedef struct {
    uint32_t block_index;
    GauntletState game_state;
    uint64_t prev_hash;
    uint64_t hash;
} GauntletBlock;

typedef struct {
    GauntletBlock ledger[MAX_STATES];
    size_t length;
} GauntletImmutableLedger;

// Custom quick hash for Gauntlet game states
uint64_t compute_gauntlet_state_hash(const GauntletBlock *b) {
    uint64_t hash = 5381;
    hash = ((hash << 5) + hash) + b->block_index;
    hash = ((hash << 5) + hash) + b->prev_hash;
    hash = ((hash << 5) + hash) + b->game_state.level;
    hash = ((hash << 5) + hash) + b->game_state.score;
    hash = ((hash << 5) + hash) + b->game_state.health;
    hash = ((hash << 5) + hash) + b->game_state.player_x;
    hash = ((hash << 5) + hash) + b->game_state.player_y;
    hash = ((hash << 5) + hash) + b->game_state.collision_mask;
    return hash;
}

// Appends a snapshot of the Gauntlet engine state to the immutable chain ledger
void commit_gauntlet_state(GauntletImmutableLedger *ledger, const GauntletState *state) {
    assert(ledger->length < MAX_STATES);

    GauntletBlock b;
    b.block_index = (uint32_t)ledger->length;
    b.game_state = *state;
    b.prev_hash = (ledger->length == 0) ? 0 : ledger->ledger[ledger->length - 1].hash;
    b.hash = compute_gauntlet_state_hash(&b);

    ledger->ledger[ledger->length] = b;
    ledger->length++;

    printf("   [Gauntlet Commit] Block #%u committed. Level: %u, Score: %u, Health: %d, Pos: (%u, %u), Collisions: 0x%X (Hash: 0x%lX)\n", 
           b.block_index, b.game_state.level, b.game_state.score, b.game_state.health, 
           b.game_state.player_x, b.game_state.player_y, b.game_state.collision_mask, b.hash);
}

// Audits the Gauntlet history ledger for integrity
bool verify_gauntlet_ledger_integrity(const GauntletImmutableLedger *ledger) {
    for (size_t i = 0; i < ledger->length; i++) {
        const GauntletBlock *b = &ledger->ledger[i];

        // 1. Recalculate block hash
        if (compute_gauntlet_state_hash(b) != b->hash) {
            return false;
        }

        // 2. Verify parent hash link
        if (i > 0) {
            if (b->prev_hash != ledger->ledger[i - 1].hash) {
                return false;
            }
        }
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT IMMUTABLE GAUNTLET GAME STATE LEDGER\n");
    printf("=============================================================\n");

    GauntletImmutableLedger ledger = {
        .length = 0
    };

    // Initialize state
    GauntletState s = {
        .level = 1,
        .score = 0,
        .health = 2000,
        .player_x = 32,
        .player_y = 32,
        .collision_mask = 0x00
    };

    // 1. Commit snapshots to ledger
    printf("1. Committing initial state (Genesis block)...\n");
    commit_gauntlet_state(&ledger, &s);

    printf("2. Committing combat state block...\n");
    s.score = 50;
    s.health = 1850; // Took damage
    s.player_x = 35;
    s.collision_mask = 0x01; // Collision trigger
    commit_gauntlet_state(&ledger, &s);

    printf("3. Committing level clear state block...\n");
    s.level = 2;
    s.score = 250;
    s.health = 2000; // Restored health
    s.player_x = 32;
    s.player_y = 32;
    s.collision_mask = 0x00;
    commit_gauntlet_state(&ledger, &s);

    assert(ledger.length == 3);
    printf("   ✓ Game state blocks committed successfully.\n\n");

    // 2. Audit ledger integrity
    printf("4. Auditing chain integrity...\n");
    bool integrity = verify_gauntlet_ledger_integrity(&ledger);
    assert(integrity == true);
    printf("   ✓ Chain integrity verified successfully.\n\n");

    // 3. Test Immutability by simulating memory tampering on Block 1 (Score hack)
    printf("5. Simulating score hack on historical block 1...\n");
    ledger.ledger[1].game_state.score = 99999; // Tamper score from 50 to 99999

    bool tampered_integrity = verify_gauntlet_ledger_integrity(&ledger);
    assert(tampered_integrity == false);
    printf("   ✓ Tampering detected: Immutable ledger block invalidation confirmed.\n\n");

    printf("=============================================================\n");
    printf("IMMUTABLE GAUNTLET LEDGER TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

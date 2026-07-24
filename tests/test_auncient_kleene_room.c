#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define DEFAULT_LIMIT 5

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// 2-3 Merkle Tree Node (Sandbox State Register)
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// Kleene Room Sandbox Context
typedef struct {
    uint32_t iteration_counter;
    uint32_t iteration_limit;
    gate_state_t write_gate;
    TwoThreeNode *state_reg;
} kleene_room_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// Helper to create leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
    return node;
}

// -------------------------------------------------------------
// Kleene Room Star (alpha*) Execution step
// -------------------------------------------------------------
bool kleene_room_step_star(kleene_room_t *room, uint8_t tag) {
    room->write_gate = CUTOFF_STATE;

    if (tag != TAG_KERNEL) return false;

    // Wortman Bounds Check: Verify iteration counter does not exceed safety limits
    if (room->iteration_counter >= room->iteration_limit) {
        return false; // Bounds violation: execution halted to prevent infinite loops
    }

    // Increment iteration counter and set conduction
    room->iteration_counter++;
    room->write_gate = CONDUC_STATE;

    // Update sandbox BASED state register values in the 2-3 tree DAT
    snprintf(room->state_reg->values[0], sizeof(room->state_reg->values[0]), "ITER:%u", room->iteration_counter);

    // Recompute Merkle root hash of the sandbox register
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", room->state_reg->keys[0], room->state_reg->values[0]);
    sha256(temp, len, room->state_reg->node_hash);

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT KLEENE ROOM SANDBOX VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *state_reg = create_leaf(777, "ITER:0");
    kleene_room_t room = {
        .iteration_counter = 0,
        .iteration_limit = DEFAULT_LIMIT,
        .write_gate = CUTOFF_STATE,
        .state_reg = state_reg
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, state_reg->node_hash, HASH_SIZE);

    // 1. Run iterations up to limit (5 steps) -> Should succeed
    printf("[TEST] Running Kleene Star steps up to limit (5 steps)...\n");
    fflush(stdout);
    for (int i = 0; i < DEFAULT_LIMIT; i++) {
        bool ok = kleene_room_step_star(&room, TAG_KERNEL);
        assert(ok == true);
        assert(room.write_gate == CONDUC_STATE);
        assert(room.iteration_counter == (uint32_t)(i + 1));
    }
    assert(strcmp(state_reg->values[0], "ITER:5") == 0);
    assert(memcmp(initial_hash, state_reg->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Sandbox executed %u steps successfully. Hash updated.\n", room.iteration_counter);
    fflush(stdout);

    // Save hash state
    uint8_t limit_hash[HASH_SIZE];
    memcpy(limit_hash, state_reg->node_hash, HASH_SIZE);

    // 2. Exceed limit (step 6) -> Should fail/cutoff
    printf("[TEST] Attempting execution beyond iteration limit...\n");
    fflush(stdout);
    bool ok = kleene_room_step_star(&room, TAG_KERNEL);
    assert(ok == false);
    assert(room.write_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(state_reg->values[0], "ITER:5") == 0); // Unaltered
    assert(memcmp(limit_hash, state_reg->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Sandbox overflow blocked successfully by bounds checker.\n");
    fflush(stdout);

    free(state_reg);
    printf("=============================================================\n");
    printf("KLEENE ROOM SANDBOX TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

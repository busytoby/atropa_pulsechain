#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define VOCAB_SIZE 3

// System state tracking
typedef struct {
    bool scsi_locked;
    char scsi_owner[16];
    uint32_t ledger_writes;
} system_state_t;

// Transfluxor Word definition
typedef struct {
    char name[32];
    uint64_t hash_key;
} control_word_t;

static control_word_t vocabulary[VOCAB_SIZE];

// Calculate acoustic hash
uint64_t calc_coord_hash(double f1, double f2, double decay) {
    uint64_t f1_val = (uint64_t)round(f1);
    uint64_t f2_val = (uint64_t)round(f2);
    uint64_t decay_val = (uint64_t)round(decay * 100.0);
    uint64_t mixed = (f1_val * 1000ULL) + (f2_val * 10ULL) + decay_val;
    return mixed % MOTZKIN_PRIME;
}

// Initialize vocabulary keys
void init_coord_vocabulary(void) {
    // 1. LOCK_SCSI word: F1=440Hz, F2=880Hz, Decay=0.4
    vocabulary[0].hash_key = calc_coord_hash(440.0, 880.0, 0.4);
    strcpy(vocabulary[0].name, "SPK_LOCK_SCSI");

    // 2. WRITE_LEDGER word: F1=350Hz, F2=700Hz, Decay=0.2
    vocabulary[1].hash_key = calc_coord_hash(350.0, 700.0, 0.2);
    strcpy(vocabulary[1].name, "SPK_WRITE_LEDGER");

    // 3. RELEASE_SCSI word: F1=880Hz, F2=1760Hz, Decay=0.4
    vocabulary[2].hash_key = calc_coord_hash(880.0, 1760.0, 0.4);
    strcpy(vocabulary[2].name, "SPK_RELEASE_SCSI");
}

// Process agent wave dispatch through the OS gating logic
bool process_agent_dispatch(system_state_t *state, const char *agent_name, double f1, double f2, double decay) {
    uint64_t query_hash = calc_coord_hash(f1, f2, decay);
    control_word_t *word = NULL;

    for (int i = 0; i < VOCAB_SIZE; i++) {
        if (vocabulary[i].hash_key == query_hash) {
            word = &vocabulary[i];
            break;
        }
    }

    if (!word) {
        printf("   [OS ERROR] Unrecognized wave pattern from agent '%s'.\n", agent_name);
        return false;
    }

    printf("   [OS RECEIVER] Processing '%s' from agent '%s'...\n", word->name, agent_name);

    if (strcmp(word->name, "SPK_LOCK_SCSI") == 0) {
        if (state->scsi_locked) {
            printf("      [BLOCKED] SCSI bus is already locked by owner '%s'.\n", state->scsi_owner);
            return false;
        }
        state->scsi_locked = true;
        strcpy(state->scsi_owner, agent_name);
        printf("      [SUCCESS] SCSI bus lock granted to '%s'.\n", agent_name);
        return true;
    } 
    
    if (strcmp(word->name, "SPK_WRITE_LEDGER") == 0) {
        if (state->scsi_locked && strcmp(state->scsi_owner, agent_name) != 0) {
            printf("      [BLOCKED] Write rejected. SCSI bus is locked by owner '%s'.\n", state->scsi_owner);
            return false;
        }
        state->ledger_writes++;
        printf("      [SUCCESS] Ledger write verified for '%s'. Total: %u\n", agent_name, state->ledger_writes);
        return true;
    }

    if (strcmp(word->name, "SPK_RELEASE_SCSI") == 0) {
        if (!state->scsi_locked) {
            printf("      [BLOCKED] Release rejected. SCSI bus is not locked.\n");
            return false;
        }
        if (strcmp(state->scsi_owner, agent_name) != 0) {
            printf("      [BLOCKED] Release rejected. Owner is '%s', not '%s'.\n", state->scsi_owner, agent_name);
            return false;
        }
        state->scsi_locked = false;
        state->scsi_owner[0] = '\0';
        printf("      [SUCCESS] SCSI bus lock released by '%s'.\n", agent_name);
        return true;
    }

    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AGENT COORDINATION LOCKING SUITE\n");
    printf("=============================================================\n");

    init_coord_vocabulary();
    system_state_t state = { .scsi_locked = false, .scsi_owner = "", .ledger_writes = 0 };

    // 1. Agent A locks the SCSI bus
    printf("[TEST] Agent A sending lock command wave...\n");
    bool success = process_agent_dispatch(&state, "Agent_A", 440.0, 880.0, 0.4);
    assert(success == true);
    assert(state.scsi_locked == true);
    assert(strcmp(state.scsi_owner, "Agent_A") == 0);

    // 2. Agent B tries to write to ledger but fails due to Agent A's lock
    printf("[TEST] Agent B attempting write command wave during lock...\n");
    success = process_agent_dispatch(&state, "Agent_B", 350.0, 700.0, 0.2);
    assert(success == false);
    assert(state.ledger_writes == 0);

    // 3. Agent A successfully writes to ledger
    printf("[TEST] Agent A sending write command wave...\n");
    success = process_agent_dispatch(&state, "Agent_A", 350.0, 700.0, 0.2);
    assert(success == true);
    assert(state.ledger_writes == 1);

    // 4. Agent B tries to release Agent A's lock but fails
    printf("[TEST] Agent B attempting release command wave on Agent A's lock...\n");
    success = process_agent_dispatch(&state, "Agent_B", 880.0, 1760.0, 0.4);
    assert(success == false);
    assert(state.scsi_locked == true);

    // 5. Agent A releases the lock
    printf("[TEST] Agent A sending release command wave...\n");
    success = process_agent_dispatch(&state, "Agent_A", 880.0, 1760.0, 0.4);
    assert(success == true);
    assert(state.scsi_locked == false);

    // 6. Agent B can now claim the lock and write to the ledger
    printf("[TEST] Agent B claiming lock and writing...\n");
    success = process_agent_dispatch(&state, "Agent_B", 440.0, 880.0, 0.4);
    assert(success == true);
    success = process_agent_dispatch(&state, "Agent_B", 350.0, 700.0, 0.2);
    assert(success == true);
    assert(state.ledger_writes == 2);

    printf("=============================================================\n");
    printf("AGENT COORDINATION LOCK TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

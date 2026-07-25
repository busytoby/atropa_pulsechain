#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MOTZKIN_PRIME 953467954114363ULL

// WinchesterMQ command register mappings
typedef enum {
    WMQ_CMD_LOCK_SCSI = 0x10,
    WMQ_CMD_RELEASE_SCSI = 0x11,
    WMQ_CMD_NOOP = 0x00
} wmq_cmd_t;

// ABI Accessor opcodes
typedef enum {
    ABI_OP_WRITE_LEDGER = 0x02,
    ABI_OP_NOOP = 0x00
} abi_op_t;

// System state tracking
typedef struct {
    bool scsi_locked;
    uint32_t ledger_writes_count;
    double feedback_frequency;
} system_state_t;

// Transfluxor Word mapping formant frequency hashes to system operations
typedef struct {
    char name[32];
    uint64_t hash_key;
    wmq_cmd_t wmq_action;
    abi_op_t abi_action;
} control_word_t;

static control_word_t control_vocab[2];

// Hash function
uint64_t calc_integration_hash(double f1, double f2, double decay) {
    uint64_t f1_val = (uint64_t)round(f1);
    uint64_t f2_val = (uint64_t)round(f2);
    uint64_t decay_val = (uint64_t)round(decay * 100.0);
    uint64_t mixed = (f1_val * 1000ULL) + (f2_val * 10ULL) + decay_val;
    return mixed % MOTZKIN_PRIME;
}

// Initialize the integration vocabulary
void init_integration_vocab(void) {
    // 1. Word "SPK_LOCK_SCSI" (F1=440, F2=880, Decay=0.4) -> WinchesterMQ Lock SCSI
    control_vocab[0].hash_key = calc_integration_hash(440.0, 880.0, 0.4);
    strcpy(control_vocab[0].name, "SPK_LOCK_SCSI");
    control_vocab[0].wmq_action = WMQ_CMD_LOCK_SCSI;
    control_vocab[0].abi_action = ABI_OP_NOOP;

    // 2. Word "SPK_WRITE_LEDGER" (F1=350, F2=700, Decay=0.2) -> ABI Write Ledger
    control_vocab[1].hash_key = calc_integration_hash(350.0, 700.0, 0.2);
    strcpy(control_vocab[1].name, "SPK_WRITE_LEDGER");
    control_vocab[1].wmq_action = WMQ_CMD_NOOP;
    control_vocab[1].abi_action = ABI_OP_WRITE_LEDGER;
}

// OS Receiver: Detects, resolves, and executes actions from spoken waveforms
bool os_process_spoken_word(system_state_t *state, double f1, double f2, double decay, double amplitude) {
    // 1. Wheeler Jump check (Latch gating)
    if (amplitude < 4.5) {
        printf("   [OS RECEIVER] Blocked. Missing Wheeler Jump transient trigger.\n");
        return false;
    }
    printf("   [OS RECEIVER] Wheeler Jump detected! Opening logic bus.\n");

    // 2. Resolve word name via hash lookup
    uint64_t query_hash = calc_integration_hash(f1, f2, decay);
    control_word_t *resolved_word = NULL;
    
    for (int i = 0; i < 2; i++) {
        if (control_vocab[i].hash_key == query_hash) {
            resolved_word = &control_vocab[i];
            break;
        }
    }

    if (!resolved_word) {
        printf("   [OS RECEIVER] Resolution failed: Unknown acoustic pattern.\n");
        state->feedback_frequency = 110.0; // Failure drone
        return false;
    }
    printf("   [OS RECEIVER] Resolved spoken word: '%s'\n", resolved_word->name);

    // 3. Dispatch system actions and generate outcome
    if (resolved_word->wmq_action == WMQ_CMD_LOCK_SCSI) {
        state->scsi_locked = true;
        state->feedback_frequency = 523.25; // Success Tone
        printf("   [WMQ DISPATCH] SCSI Channel locked successfully.\n");
    } else if (resolved_word->abi_action == ABI_OP_WRITE_LEDGER) {
        state->ledger_writes_count++;
        state->feedback_frequency = 523.25; // Success Tone
        printf("   [ABI DISPATCH] Ledger write transaction count incremented to %u.\n", 
               state->ledger_writes_count);
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATION SPEECH CONTROL SUITE\n");
    printf("=============================================================\n");

    init_integration_vocab();
    system_state_t state = {
        .scsi_locked = false,
        .ledger_writes_count = 0,
        .feedback_frequency = 0.0
    };

    // 1. Agent speaks "SPK_LOCK_SCSI"
    printf("[TEST] Agent speaking 'SPK_LOCK_SCSI' (F1=440Hz, F2=880Hz, Decay=0.4, Amp=5.0)...\n");
    bool processed = os_process_spoken_word(&state, 440.0, 880.0, 0.4, 5.0);
    assert(processed == true);
    assert(state.scsi_locked == true);
    assert(state.feedback_frequency == 523.25);
    printf("   ✓ Spoken WMQ lock command executed with correct system state changes.\n");

    // 2. Agent speaks "SPK_WRITE_LEDGER"
    printf("[TEST] Agent speaking 'SPK_WRITE_LEDGER' (F1=350Hz, F2=700Hz, Decay=0.2, Amp=5.0)...\n");
    processed = os_process_spoken_word(&state, 350.0, 700.0, 0.2, 5.0);
    assert(processed == true);
    assert(state.ledger_writes_count == 1);
    assert(state.feedback_frequency == 523.25);
    printf("   ✓ Spoken ABI write command executed with correct system state changes.\n");

    // 3. Agent speaks unregistered word
    printf("[TEST] Agent speaking unrecognized word...\n");
    processed = os_process_spoken_word(&state, 500.0, 1000.0, 0.5, 5.0);
    assert(processed == false);
    assert(state.feedback_frequency == 110.0);
    printf("   ✓ Unrecognized spoken command correctly rejected with failure feedback.\n");

    printf("=============================================================\n");
    printf("INTEGRATION SPEECH CONTROL TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

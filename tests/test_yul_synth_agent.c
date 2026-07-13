#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Opcodes
typedef enum {
    YUL_OP_SYNTH_PLAY = 0x65,
    YUL_OP_QUERY_READ = 0x30
} YulOpcode;

// ACL Level definitions
typedef enum {
    ACL_ANONYMOUS = 0,
    ACL_USER = 1,
    ACL_ADMIN = 2
} AclLevel;

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Simulated Bionika Synthesizer Hardware State
typedef struct {
    double   frequency_hz;
    uint32_t active_note_count;
    PPN      last_controller;
} BionikaSynthHardware;

BionikaSynthHardware g_synth;

// Registry tracking ACL permissions
#define MAX_KEYS 4
typedef struct {
    uint32_t key_id;
    PPN      ppn;
    AclLevel acl;
} KeyEntry;

KeyEntry g_keys[MAX_KEYS];

// Global gas tracking
uint64_t g_balances[100];
const uint64_t UNIVERSAL_GAS_FEE = 15;

uint32_t get_ppn_slot(PPN ppn) {
    return (ppn.project + ppn.programmer) % 100;
}

void init_synth_system(void) {
    memset(&g_synth, 0, sizeof(g_synth));
    memset(g_balances, 0, sizeof(g_balances));
    memset(g_keys, 0, sizeof(g_keys));
    
    // Seed Keys
    g_keys[0] = (KeyEntry){ .key_id = 11, .ppn = {1, 2},  .acl = ACL_ADMIN };
    g_keys[1] = (KeyEntry){ .key_id = 22, .ppn = {10, 5}, .acl = ACL_USER };
    g_keys[2] = (KeyEntry){ .key_id = 33, .ppn = {30, 1}, .acl = ACL_ANONYMOUS };
}

AclLevel get_key_acl(uint32_t key_id) {
    for (int i = 0; i < MAX_KEYS; i++) {
        if (g_keys[i].key_id == key_id) {
            return g_keys[i].acl;
        }
    }
    return ACL_ANONYMOUS;
}

// Simulated Yul Agent Contract execution
bool yul_synth_agent_play(PPN caller, double freq, const char **out_err) {
    // Verify frequency boundary (e.g. standard Auncient audio scale 20Hz - 20kHz)
    if (freq < 20.0 || freq > 20000.0) {
        *out_err = "REVERT: FREQUENCY_OUT_OF_RANGE";
        return false;
    }
    
    g_synth.frequency_hz = freq;
    g_synth.active_note_count++;
    g_synth.last_controller = caller;
    
    printf("   [Yul Synth Agent] Played note frequency: %.2f Hz (Total Notes: %u)\n",
           freq, g_synth.active_note_count);
    return true;
}

// Coaxial execution pipeline
bool dispatch_synth_transaction(uint32_t key_id, PPN ppn, YulOpcode op, double freq, const char **out_err) {
    uint32_t slot = get_ppn_slot(ppn);
    if (g_balances[slot] < UNIVERSAL_GAS_FEE) {
        *out_err = "REVERT: INSUFFICIENT_GAS_BALANCE";
        return false;
    }
    g_balances[slot] -= UNIVERSAL_GAS_FEE;

    // Enforce ACL check: Anonymous keys are blocked from controlling the synthesizer
    AclLevel acl = get_key_acl(key_id);
    if (acl < ACL_USER) {
        *out_err = "REVERT: ACL_PERMISSION_DENIED_FOR_SYNTH_ACCESS";
        return false;
    }

    switch (op) {
        case YUL_OP_SYNTH_PLAY: {
            return yul_synth_agent_play(ppn, freq, out_err);
        }
        default:
            *out_err = "REVERT: INVALID_OPCODE";
            return false;
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: YUL SYNTHESIZER AGENT TESTS\n");
    printf("=============================================================\n");

    init_synth_system();
    
    PPN admin_ppn = { .project = 1, .programmer = 2 };
    PPN user_ppn = { .project = 10, .programmer = 5 };
    PPN anon_ppn = { .project = 30, .programmer = 1 };
    
    g_balances[get_ppn_slot(admin_ppn)] = 100;
    g_balances[get_ppn_slot(user_ppn)] = 100;
    g_balances[get_ppn_slot(anon_ppn)] = 100;

    const char *err = NULL;

    // 1. Test Case 1: Anonymous Peer attempts to play synth (Blocked)
    printf("1. Anonymous peer 0x33 attempting to play synthesizer note...\n");
    bool res = dispatch_synth_transaction(33, anon_ppn, YUL_OP_SYNTH_PLAY, 440.0, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: ACL_PERMISSION_DENIED_FOR_SYNTH_ACCESS") == 0);
    printf("   ✓ Anonymous synthesizer access denied correctly.\n\n");

    // 2. Test Case 2: User Bob executes Yul Synth Agent playing A4 (Allowed)
    printf("2. User Bob PPN [10,5] playing note A4 (440.0 Hz)...\n");
    res = dispatch_synth_transaction(22, user_ppn, YUL_OP_SYNTH_PLAY, 440.0, &err);
    assert(res == true);
    assert(g_synth.frequency_hz == 440.0);
    assert(g_synth.active_note_count == 1);
    printf("   ✓ Note played successfully.\n\n");

    // 3. Test Case 3: Admin executes Yul Synth Agent playing C5 (Allowed)
    printf("3. Admin PPN [1,2] playing note C5 (523.25 Hz)...\n");
    res = dispatch_synth_transaction(11, admin_ppn, YUL_OP_SYNTH_PLAY, 523.25, &err);
    assert(res == true);
    assert(g_synth.frequency_hz == 523.25);
    assert(g_synth.active_note_count == 2);
    printf("   ✓ Note played successfully.\n\n");

    // 4. Test Case 4: Frequency out of range check (e.g. 0 Hz)
    printf("4. Attempting to play frequency outside human hearing range (0 Hz)...\n");
    res = dispatch_synth_transaction(11, admin_ppn, YUL_OP_SYNTH_PLAY, 0.0, &err);
    assert(res == false);
    assert(strcmp(err, "REVERT: FREQUENCY_OUT_OF_RANGE") == 0);
    printf("   ✓ Out of range frequency caught successfully: %s\n\n", err);

    printf("=============================================================\n");
    printf("AUNCIENT YUL SYNTHESIZER AGENT TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

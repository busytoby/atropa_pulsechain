#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// PPN Account Structure
typedef struct {
    uint16_t project;
    uint16_t programmer;
} PPN;

// Immutable Sound Log Node structure
typedef struct {
    uint64_t timestamp;
    PPN      author;
    double   frequency;
    uint8_t  prev_root[32];  // Previous Merkle/Hash state root
    uint8_t  state_root[32]; // Current state root containing this node data
} SoundLogNode;

#define MAX_SOUNDS 16
SoundLogNode g_sound_chain[MAX_SOUNDS];
size_t g_sound_count = 0;

// Simple non-cryptographic hash mapping for state root simulation (Rule 11/12 compatibility)
void compute_state_root(SoundLogNode *node) {
    uint32_t val = (uint32_t)node->timestamp +
                   (node->author.project + node->author.programmer) +
                   (uint32_t)(node->frequency * 100);
                   
    // Seed hash with node values
    memset(node->state_root, 0, 32);
    memcpy(node->state_root, &val, sizeof(val));
    
    // XOR previous root bytes into the state root to build the chain
    for (int i = 0; i < 32; i++) {
        node->state_root[i] ^= node->prev_root[i];
    }
}

// Append a sound transaction to the immutable log
bool append_sound_to_log(PPN author, double freq, const char **out_err) {
    if (g_sound_count >= MAX_SOUNDS) {
        *out_err = "REVERT: SOUND_LOG_LIMIT_EXCEEDED";
        return false;
    }
    
    SoundLogNode *node = &g_sound_chain[g_sound_count];
    node->timestamp = 1000 + g_sound_count; // Jiffies
    node->author = author;
    node->frequency = freq;
    
    // Link previous root hash
    if (g_sound_count == 0) {
        memset(node->prev_root, 0, 32); // Genesis state root
    } else {
        memcpy(node->prev_root, g_sound_chain[g_sound_count - 1].state_root, 32);
    }
    
    compute_state_root(node);
    g_sound_count++;
    
    printf("   [Sound Log] Appended Note %.2f Hz | State Root: 0x%02x%02x%02x%02x...\n",
           freq, node->state_root[0], node->state_root[1], node->state_root[2], node->state_root[3]);
    return true;
}

// Verify the integrity of the entire state chain
bool verify_sound_log_integrity(void) {
    uint8_t expected_prev[32];
    memset(expected_prev, 0, 32);
    
    for (size_t i = 0; i < g_sound_count; i++) {
        SoundLogNode *node = &g_sound_chain[i];
        
        // 1. Verify link to previous node root hash
        if (memcmp(node->prev_root, expected_prev, 32) != 0) {
            return false; // Chain broken
        }
        
        // 2. Recalculate and verify current node state root
        uint8_t stored_root[32];
        memcpy(stored_root, node->state_root, 32);
        
        compute_state_root(node);
        if (memcmp(node->state_root, stored_root, 32) != 0) {
            return false; // Data modified
        }
        
        memcpy(expected_prev, node->state_root, 32);
    }
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: IMMUTABLE SYNTHESIZER SOUND LOG VERIFICATION\n");
    printf("=============================================================\n");

    PPN user_alice = { .project = 10, .programmer = 5 };
    const char *err = NULL;

    // 1. Log a sequence of notes
    printf("1. Logging audio note sequences...\n");
    assert(append_sound_to_log(user_alice, 440.0, &err) == true);   // Note A4
    assert(append_sound_to_log(user_alice, 523.25, &err) == true);  // Note C5
    assert(append_sound_to_log(user_alice, 659.25, &err) == true);  // Note E5
    printf("   ✓ Notes successfully appended and hashed.\n\n");

    // 2. Verify chain integrity under normal conditions
    printf("2. Verifying sound log chain integrity (Expected: Pass)...\n");
    bool integrity = verify_sound_log_integrity();
    assert(integrity == true);
    printf("   ✓ Integrity check PASSED. State root chain is consistent.\n\n");

    // 3. Attempt modification by an attacker (simulating database alteration)
    printf("3. Simulating an attacker modifying Note 1 frequency from 440Hz to 880Hz...\n");
    g_sound_chain[0].frequency = 880.0;
    
    // Verify that the integrity check catches the alteration
    printf("4. Verifying sound log chain integrity after attack (Expected: Fail)...\n");
    integrity = verify_sound_log_integrity();
    assert(integrity == false);
    printf("   ✓ Integrity check FAILED. Sound tampering successfully detected!\n\n");

    printf("=============================================================\n");
    printf("AUNCIENT IMMUTABLE SOUND LOG TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

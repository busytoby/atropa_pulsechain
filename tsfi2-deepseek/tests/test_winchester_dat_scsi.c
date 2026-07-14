#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "tsfi_dat.h"
#include "tsfi_trie.h"

// MotzkinPrime system constant mapping from glossary
#define MOTZKIN_PRIME 953467954114363ULL

// Helper function for modular exponentiation: (base^exp) % mod
static unsigned long long mod_pow(unsigned long long base, unsigned long long exp, unsigned long long mod) {
    unsigned long long res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) {
            // Avoid overflow by casting to unsigned __int128 if available
            unsigned __int128 temp = (unsigned __int128)res * base;
            res = (unsigned long long)(temp % mod);
        }
        unsigned __int128 temp = (unsigned __int128)base * base;
        base = (unsigned long long)(temp % mod);
        exp >>= 1;
    }
    return res;
}

int main(void) {
    printf("=== TSFi Auncient WinchesterMQ SCSI Handshake DAT Verification ===\n");

    // 1. Build and compile the WinchesterMQ handshake stages into a Double-Array Trie
    printf("[WinchesterMQ] Compiling SCSI Handshake state transitions to DAT...\n");
    tsfi_trie_node *trie = tsfi_trie_create_node('\0');
    tsfi_trie_insert(trie, "FORM", "STATE_FORM");
    tsfi_trie_insert(trie, "CONJUGATE", "STATE_CONJUGATE");
    tsfi_trie_insert(trie, "SATURATE", "STATE_SATURATE");
    tsfi_trie_insert(trie, "MAGNETIZE", "STATE_MAGNETIZE");

    tsfi_dat *handshake_dat = tsfi_dat_compile(trie);
    assert(handshake_dat != NULL);
    printf("  [PASS] Handshake DAT compiled successfully.\n");

    // 2. Initialize low-level registers as described in glossary
    unsigned long long base_reg = 2; // Base B for modular calculations
    unsigned long long secret_reg = 137;
    unsigned long long signal_reg = 440;
    unsigned long long identity_reg = 99;
    unsigned long long element_reg = 512; // Derived spacing spacing

    // State outcomes
    unsigned long long channel_reg = 0;
    unsigned long long pole_reg = 0;
    unsigned long long foundation_reg = 0;
    unsigned long long dynamo_reg = 0;

    // 3. Simulate the 4-Step WinchesterMQ SCSI Handshake loop using DAT transitions
    printf("[WinchesterMQ] Initiating Auncient SCSI handshake sequence...\n");

    // STEP 1: FORM
    const char *step1 = tsfi_dat_search(handshake_dat, "FORM");
    assert(step1 != NULL && strcmp(step1, "STATE_FORM") == 0);
    // Formula: Channel = Base^Signal mod MotzkinPrime
    channel_reg = mod_pow(base_reg, signal_reg, MOTZKIN_PRIME);
    printf("  [STEP 1: FORM] Resolved: %s | Channel Register: %llu\n", step1, channel_reg);

    // STEP 2: CONJUGATE (or Polarize)
    const char *step2 = tsfi_dat_search(handshake_dat, "CONJUGATE");
    assert(step2 != NULL && strcmp(step2, "STATE_CONJUGATE") == 0);
    // Formula: Pole = Base^Secret mod MotzkinPrime
    pole_reg = mod_pow(base_reg, secret_reg, MOTZKIN_PRIME);
    printf("  [STEP 2: CONJUGATE] Resolved: %s | Pole Register: %llu\n", step2, pole_reg);

    // STEP 3: SATURATE (or Conify)
    const char *step3 = tsfi_dat_search(handshake_dat, "SATURATE");
    assert(step3 != NULL && strcmp(step3, "STATE_SATURATE") == 0);
    // Formula: Foundation = Base^Identity mod MotzkinPrime
    foundation_reg = mod_pow(base_reg, identity_reg, MOTZKIN_PRIME);
    printf("  [STEP 3: SATURATE] Resolved: %s | Foundation Register: %llu\n", step3, foundation_reg);

    // STEP 4: MAGNETIZE (or Bond)
    const char *step4 = tsfi_dat_search(handshake_dat, "MAGNETIZE");
    assert(step4 != NULL && strcmp(step4, "STATE_MAGNETIZE") == 0);
    // Formula: Dynamo = Base^Signal mod Element
    dynamo_reg = mod_pow(base_reg, signal_reg, element_reg);
    printf("  [STEP 4: MAGNETIZE] Resolved: %s | Dynamo Register: %llu\n", step4, dynamo_reg);

    // Verify mathematical bounds
    assert(channel_reg < MOTZKIN_PRIME);
    assert(pole_reg < MOTZKIN_PRIME);
    assert(foundation_reg < MOTZKIN_PRIME);
    assert(dynamo_reg < element_reg);
    printf("  [PASS] All register values evaluated within valid mathematical modulo bounds!\n");

    // Clean up
    tsfi_trie_destroy(trie);
    tsfi_dat_destroy(handshake_dat);

    printf("=== ALL AUNCIENT WINCHESTERMQ SCSI DAT HANDSHAKE TESTS PASSED ===\n");
    return 0;
}

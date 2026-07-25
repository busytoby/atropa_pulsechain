#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "auncient_sdk.h"

// Simulate an Acoustic Cryptographic Handshake
bool verify_acoustic_handshake(const auncient_transfluxor_word_t *challenge, const auncient_transfluxor_word_t *response) {
    // 1. Calculate hashes for both words
    uint64_t challenge_hash = auncient_sdk_calculate_transfluxor_hash(
        challenge->f1, challenge->f2, challenge->decay, challenge->word_id
    );
    uint64_t response_hash = auncient_sdk_calculate_transfluxor_hash(
        response->f1, response->f2, response->decay, response->word_id
    );

    // 2. Wave interaction check: Verify that response formants complete the challenge
    // In our acoustic cipher, the response F1 must exactly complete the challenge F1 to reach the target sum (1000Hz)
    double mixed_f1 = challenge->f1 + response->f1;
    double mixed_f2 = challenge->f2 + response->f2;
    
    printf("   [HANDSHAKE AUDIT] Challenge Hash: %lu | Response Hash: %lu\n", challenge_hash, response_hash);
    printf("                     Mixed F1: %.1fHz (Target: 1000.0) | Mixed F2: %.1fHz (Target: 2000.0)\n", 
           mixed_f1, mixed_f2);

    if (fabs(mixed_f1 - 1000.0) < 1.0 && fabs(mixed_f2 - 2000.0) < 1.0) {
        // Enforce temporal decay constraints
        if (response->decay <= challenge->decay) {
            printf("   [SECURITY LATCH] Handshake verified! Access granted.\n");
            return true;
        } else {
            printf("   [SECURITY VIOLATION] Response decay window exceeded challenge limits.\n");
        }
    }

    printf("   [SECURITY LATCH] Verification failed. Intruder detected.\n");
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ACOUSTIC CRYPTOGRAPHIC HANDSHAKE SUITE\n");
    printf("=============================================================\n");

    // 1. Compile Challenge Word (F1=400Hz, F2=800Hz, Decay=0.5, ID=10)
    auncient_transfluxor_word_t challenge;
    bool ok = auncient_sdk_compile_transfluxor_word(
        &challenge,
        "CHALLENGE_WAVE",
        10,
        400.0,
        800.0,
        0.5,
        0x00,
        0x00
    );
    assert(ok == true);

    // 2. Compile VALID Response Word (F1=600Hz, F2=1200Hz, Decay=0.4, ID=11)
    // Completed F1: 400 + 600 = 1000Hz | Completed F2: 800 + 1200 = 2000Hz
    auncient_transfluxor_word_t valid_response;
    ok = auncient_sdk_compile_transfluxor_word(
        &valid_response,
        "RESPONSE_VALID",
        11,
        600.0,
        1200.0,
        0.4, // Decays before challenge window closes (secure)
        0x00,
        0x00
    );
    assert(ok == true);

    // 3. Compile INVALID Response Word (F1=500Hz, F2=1000Hz)
    auncient_transfluxor_word_t invalid_response;
    ok = auncient_sdk_compile_transfluxor_word(
        &invalid_response,
        "RESPONSE_INVALID",
        12,
        500.0,
        1000.0,
        0.4,
        0x00,
        0x00
    );
    assert(ok == true);

    // Verify handshakes
    printf("[TEST] Verifying valid acoustic response...\n");
    bool result = verify_acoustic_handshake(&challenge, &valid_response);
    assert(result == true);
    printf("   ✓ Valid handshake verified successfully.\n");

    printf("[TEST] Verifying invalid acoustic response...\n");
    result = verify_acoustic_handshake(&challenge, &invalid_response);
    assert(result == false);
    printf("   ✓ Invalid handshake correctly blocked.\n");

    printf("=============================================================\n");
    printf("ACOUSTIC CRYPTOGRAPHIC HANDSHAKE TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

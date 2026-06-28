#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#define KERMIT_SOH 0x01
#define MAX_PAYLOAD 90

// Auncient YI State Registers
typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t motzkin_prime;
} YiState;

static YiState yi_state = {
    .base = 1234567,
    .secret = 9876543,
    .signal = 5555555,
    .motzkin_prime = 953467954114363ULL
};

// YI react signature derivation
static uint64_t yi_react(uint64_t nonce, bool is_rod) {
    uint64_t signature = yi_state.base;
    if (is_rod) {
        signature = (signature ^ yi_state.secret ^ nonce) % yi_state.motzkin_prime;
    } else {
        signature = (signature ^ yi_state.signal ^ nonce) % yi_state.motzkin_prime;
    }
    return signature;
}

int main() {
    printf("=== Auncient Nonce-Collision Signature Verification Test ===\n");
    
    uint16_t addr_a = 0xAA01; // Rod Node Address
    uint16_t addr_b = 0xBB02; // Cone Node Address
    
    uint64_t test_index = 42; // Same step index
    
    printf("\n[SCENARIO 1] Naive Index Nonce Signing (No Address Binding)\n");
    uint64_t raw_nonce_a = test_index;
    uint64_t raw_nonce_b = test_index;
    
    uint64_t sig_a1 = yi_react(raw_nonce_a, true);
    uint64_t sig_b1 = yi_react(raw_nonce_b, false);
    
    printf("  Node A Raw Nonce: %lu -> YI.react: %lu\n", raw_nonce_a, sig_a1);
    printf("  Node B Raw Nonce: %lu -> YI.react: %lu\n", raw_nonce_b, sig_b1);
    
    printf("\n[SCENARIO 2] Address-Bound Nonce Signing (Preventing Collisions)\n");
    // Bind host address to the input nonce: (index << 16) | address
    uint64_t bound_nonce_a = (test_index << 16) | addr_a;
    uint64_t bound_nonce_b = (test_index << 16) | addr_b;
    
    uint64_t sig_a2 = yi_react(bound_nonce_a, true);
    uint64_t sig_b2 = yi_react(bound_nonce_b, false);
    
    printf("  Node A Bound Nonce: 0x%016lx -> YI.react: %lu\n", bound_nonce_a, sig_a2);
    printf("  Node B Bound Nonce: 0x%016lx -> YI.react: %lu\n", bound_nonce_b, sig_b2);
    
    if (sig_a2 != sig_b2) {
        printf("\n[RESULT] [SUCCESS] Nonce signature collision resolved! Secondary signatures are distinct.\n");
    } else {
        printf("\n[RESULT] [FAILED] Nonces collided.\n");
    }
    
    return 0;
}

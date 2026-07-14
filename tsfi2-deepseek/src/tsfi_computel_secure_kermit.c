#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/*
 * Auncient Computel Secure Kermit & Seed-Based Key Exchange
 * 
 * Implements public parameter derivation, session key negotiation,
 * and payload encryption over simulated decadic pulse coaxial lines.
 */

#define MOTZKIN_PRIME 953467954114363ULL

typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t shared_key;
    bool is_handshake_complete;
} SecureKermitState;

// Modular multiplication to prevent 64-bit overflow
static uint64_t secure_mod_mul(uint64_t a, uint64_t b, uint64_t m) {
    uint64_t res = 0;
    a %= m;
    while (b > 0) {
        if (b & 1) res = (res + a) % m;
        a = (a * 2) % m;
        b /= 2;
    }
    return res;
}

// Modular exponentiation
static uint64_t secure_mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) res = secure_mod_mul(res, base, mod);
        base = secure_mod_mul(base, base, mod);
        exp /= 2;
    }
    return res;
}

/* Performs Seed transition initializing the Base, Secret, and Signal registers */
void execute_secure_kermit_seed(SecureKermitState *state, uint64_t seed_val) {
    if (!state) return;
    // Generate deterministic values from seed
    state->base = (seed_val ^ 0x5555555555555555ULL) % 9999991ULL + 2;
    state->secret = (seed_val ^ 0xAAAAAAAAAAAAAAAAULL) % 9999991ULL + 2;
    state->signal = (seed_val ^ 0xF0F0F0F0F0F0F0F0ULL) % 9999991ULL + 2;
    state->shared_key = 0;
    state->is_handshake_complete = false;
}

/* Derives the public parameter value to be sent to the peer */
uint64_t derive_public_parameter(const SecureKermitState *state) {
    if (!state) return 0;
    return secure_mod_pow(state->base, state->secret, MOTZKIN_PRIME);
}

/* Derives the shared session key using the peer's public parameter */
void compute_shared_session_key(SecureKermitState *state, uint64_t peer_public) {
    if (!state) return;
    state->shared_key = secure_mod_pow(peer_public, state->secret, MOTZKIN_PRIME);
    state->is_handshake_complete = true;
}

/* Encrypts or decrypts a buffer in-place using the derived shared session key */
void crypt_kermit_payload(const SecureKermitState *state, uint8_t *payload, uint8_t len) {
    if (!state || !state->is_handshake_complete || !payload || len == 0) return;

    // XOR keystream generator based on shared key bytes
    uint64_t temp_key = state->shared_key;
    for (uint8_t i = 0; i < len; i++) {
        uint8_t key_byte = (uint8_t)((temp_key >> ((i % 8) * 8)) & 0xFF);
        payload[i] ^= key_byte;
        if (i % 8 == 7) {
            // Permute key to extend keystream
            temp_key = secure_mod_mul(temp_key, 0x5851F42D4C957F2DULL, 0xFFFFFFFFFFFFFFFFULL);
        }
    }
}

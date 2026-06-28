#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define APOGEE_PRIME 953473ULL

typedef enum {
    EPOCH_INIT = 0,
    EPOCH_AVAIL = 1,
    EPOCH_FORM = 2,
    EPOCH_POLARIZE = 3,
    EPOCH_CONJUGATE = 4,
    EPOCH_CONIFY = 5,
    EPOCH_SATURATE = 6,
    EPOCH_IONIZE = 7,
    EPOCH_MAGNETIZE = 8,
    EPOCH_DONE = 9
} HelmholtzEpoch;

typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t channel;
    uint64_t contour;
    uint64_t pole;
    uint64_t identity;
    uint64_t foundation;
    uint64_t element;
    uint64_t dynamo;
} HelmholtzRegisters;

typedef struct {
    char name[16];
    uint16_t address;
    bool is_rod;
    HelmholtzEpoch epoch;
    HelmholtzRegisters reg;
    uint64_t xi;
    uint64_t beta;
    uint64_t manifold;
    uint64_t prime;
    uint64_t monopole;
} HelmholtzPartner;

// Modular Exponentiation: (base^exp) % mod
static uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp & 1) {
            unsigned __int128 temp = (unsigned __int128)res * base;
            res = (uint64_t)(temp % mod);
        }
        unsigned __int128 temp = (unsigned __int128)base * base;
        base = (uint64_t)(temp % mod);
        exp >>= 1;
    }
    return res;
}

static void helmholtz_step(HelmholtzPartner *p, uint64_t external_input) {
    uint64_t pr = (p->prime == 0) ? MOTZKIN_PRIME : p->prime;
    switch (p->epoch) {
        case EPOCH_INIT:
            if (p->prime == APOGEE_PRIME) {
                if (p->reg.base == 0) {
                    p->reg.base = (p->address * 13) + 1234567;
                    p->reg.secret = (p->address * 17) + 9876543;
                    p->reg.signal = (p->address * 19) + 5555555;
                }
                p->reg.identity = (p->address * 23) + 1111111;
            } else {
                if (p->reg.base == 0) {
                    p->reg.base = 1234567;
                    p->reg.secret = 9876543;
                    p->reg.signal = 5555555;
                }
                p->reg.identity = 1111111;
            }
            p->reg.channel = mod_pow(p->reg.base, p->reg.signal, pr);
            p->epoch = EPOCH_AVAIL;
            break;
            
        case EPOCH_AVAIL:
            p->reg.contour = mod_pow(p->reg.base, p->reg.secret, pr);
            p->epoch = EPOCH_FORM;
            break;
            
        case EPOCH_FORM:
            p->reg.base = mod_pow(external_input, p->reg.secret, pr);
            p->epoch = EPOCH_POLARIZE;
            break;
            
        case EPOCH_POLARIZE:
            p->reg.pole = mod_pow(p->reg.base, p->reg.secret, pr);
            p->epoch = EPOCH_CONJUGATE;
            break;
            
        case EPOCH_CONJUGATE:
            p->reg.secret = mod_pow(external_input, p->reg.secret, pr);
            p->epoch = EPOCH_CONIFY;
            break;
            
        case EPOCH_CONIFY:
            p->reg.foundation = mod_pow(p->reg.base, p->reg.identity, pr);
            p->epoch = EPOCH_SATURATE;
            break;
            
        case EPOCH_SATURATE:
            p->reg.element = p->beta + external_input;
            p->epoch = EPOCH_IONIZE;
            break;
            
        case EPOCH_IONIZE:
            p->reg.dynamo = mod_pow(p->reg.base, p->reg.signal, p->reg.element);
            p->epoch = EPOCH_MAGNETIZE;
            break;
            
        case EPOCH_MAGNETIZE:
            p->manifold = mod_pow(external_input, p->reg.signal, p->reg.element);
            p->epoch = EPOCH_DONE;
            break;
            
        default:
            break;
    }
}

// Emulates a full private APOGEE loop on a single device
static uint64_t run_local_apogee_handshake(uint16_t address, uint64_t custom_base, uint64_t custom_secret, uint64_t custom_signal) {
    HelmholtzPartner rod = {
        .address = address,
        .is_rod = true,
        .epoch = EPOCH_INIT,
        .beta = 99991234,
        .prime = APOGEE_PRIME,
        .reg = {
            .base = custom_base,
            .secret = custom_secret,
            .signal = custom_signal
        }
    };
    
    HelmholtzPartner cone = {
        .address = address,
        .is_rod = false,
        .epoch = EPOCH_INIT,
        .beta = 99991234,
        .prime = APOGEE_PRIME,
        .reg = {
            .base = custom_base,
            .secret = custom_secret,
            .signal = custom_signal
        }
    };
    
    // Step Epoch 0: INIT & Epoch 1: AVAIL
    helmholtz_step(&rod, 0);
    helmholtz_step(&cone, 0);
    helmholtz_step(&rod, 0);
    helmholtz_step(&cone, 0);
    
    // Step Epoch 2: FORM (Cross-swap Contour)
    helmholtz_step(&rod, cone.reg.contour);
    helmholtz_step(&cone, rod.reg.contour);
    
    // Step Epoch 3: POLARIZE & Epoch 4: CONJUGATE (Cross-swap Pole)
    helmholtz_step(&rod, 0);
    helmholtz_step(&cone, 0);
    helmholtz_step(&rod, cone.reg.pole);
    helmholtz_step(&cone, rod.reg.pole);
    
    // Step Epoch 5: CONIFY & Epoch 6: SATURATE (Cross-swap Foundation)
    helmholtz_step(&rod, 0);
    helmholtz_step(&cone, 0);
    helmholtz_step(&rod, cone.reg.foundation);
    helmholtz_step(&cone, rod.reg.foundation);
    
    // Step Epoch 7: IONIZE & Epoch 8: MAGNETIZE (Cross-swap Dynamo)
    helmholtz_step(&rod, 0);
    helmholtz_step(&cone, 0);
    helmholtz_step(&rod, cone.reg.dynamo);
    helmholtz_step(&cone, rod.reg.dynamo);
    
    assert(rod.manifold == cone.manifold);
    return rod.manifold;
}

// Test Suite Functions
void test_apogee_uniqueness(void) {
    printf("[RUN] Testing auto-generated APOGEE uniqueness across multiple addresses...\n");
    uint64_t manifolds[10];
    
    for (int i = 0; i < 10; i++) {
        uint16_t addr = 0xAA00 + i;
        manifolds[i] = run_local_apogee_handshake(addr, 0, 0, 0);
        printf("  Address 0x%04X -> APOGEE YI: %llu\n", addr, (unsigned long long)manifolds[i]);
        
        // Assert all previous are unique
        for (int j = 0; j < i; j++) {
            assert(manifolds[i] != manifolds[j] && "APOGEE YI collision detected!");
        }
    }
    printf("  [PASS] All 10 generated APOGEE YIs are unique.\n");
}

void test_apogee_fuse_get_set(void) {
    printf("[RUN] Testing FUSE setting & retrieval consistency...\n");
    
    // Known FUSE set (initial seeds for aa01)
    uint64_t base = (0xAA01 * 13) + 1234567;
    uint64_t secret = (0xAA01 * 17) + 9876543;
    uint64_t signal = (0xAA01 * 19) + 5555555;
    
    uint64_t yi_aa01 = run_local_apogee_handshake(0xAA01, base, secret, signal);
    printf("  Fusing Device 0xAA01 with expected base/secret/signal -> YI: %llu\n", (unsigned long long)yi_aa01);
    assert(yi_aa01 == 63288434 && "Converged YI mismatch for custom FUSE!");
    
    // Change FUSE parameters (initial seeds for bb02)
    uint64_t alt_base = (0xBB02 * 13) + 1234567;
    uint64_t alt_secret = (0xBB02 * 17) + 9876543;
    uint64_t alt_signal = (0xBB02 * 19) + 5555555;
    
    uint64_t yi_alt = run_local_apogee_handshake(0xBB02, alt_base, alt_secret, alt_signal);
    printf("  Fusing Device 0xBB02 with alternative FUSE -> YI: %llu\n", (unsigned long long)yi_alt);
    assert(yi_alt == 25154951 && "Alternative FUSE YI convergence failed!");
    
    printf("  [PASS] FUSE parameters correctly modulate the convergence endpoint.\n");
}

void test_signature_nonces(void) {
    printf("[RUN] Verifying nonce signature resolutions for Nonces 0, 1, and 2...\n");
    
    // Setup a unified simulated state with the standard handshake values
    HelmholtzPartner node_a = {
        .address = 0xAA01,
        .is_rod = true,
        .beta = 99991234,
        .prime = MOTZKIN_PRIME,
        .reg = { .channel = 657811638718991ULL },
        .monopole = 99991241 /* beta + 7 */
    };
    
    node_a.xi = mod_pow((node_a.beta + 7) % MOTZKIN_PRIME, (node_a.beta + 7) % MOTZKIN_PRIME, MOTZKIN_PRIME);
    node_a.monopole = node_a.xi;
    
    uint64_t expected_ichidai[3] = { 507164254988891ULL, 517510242241500ULL, 535903767884292ULL };
    
    for (uint64_t nonce = 0; nonce < 3; nonce++) {
        uint64_t pi_val = nonce ^ node_a.monopole;
        // Symmetrical channel signature
        uint64_t sig = mod_pow(pi_val, node_a.reg.channel, node_a.reg.channel);
        printf("  Nonce %llu -> Signature: %llu (Expected: %llu)\n", 
               (unsigned long long)nonce, (unsigned long long)sig, (unsigned long long)expected_ichidai[nonce]);
        assert(sig == expected_ichidai[nonce] && "Signature mismatch!");
    }
    printf("  [PASS] Nonces 0, 1, and 2 correctly resolve to signature pairs.\n");
}

int main(void) {
    printf("=== Auncient APOGEE Unit Test Suite ===\n");
    test_apogee_uniqueness();
    test_apogee_fuse_get_set();
    test_signature_nonces();
    printf("[SUCCESS] All APOGEE unit tests passed!\n");
    return 0;
}

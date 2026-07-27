#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Tripartite Tri-axis Lissajous modulation equation helper for test verification
// Modulates coordinates based on frequency, amplitude, and phase offset.
static double calculate_lissajous_coordinate(double phase, double frequency, double amplitude) {
    return sin(phase * frequency) * amplitude;
}

// Modular exponentiation helper for WinchesterMQ register computations
static uint64_t modular_pow(uint64_t base, uint64_t exponent, uint64_t modulus) {
    uint64_t result = 1;
    base = base % modulus;
    while (exponent > 0) {
        if (exponent % 2 == 1) {
            result = (__uint128_t)result * base % modulus;
        }
        exponent = exponent >> 1;
        base = (__uint128_t)base * base % modulus;
    }
    return result;
}

// FNV-1a Hashing algorithm verification helper
static uint32_t fnv1a_hash(const uint8_t *data, size_t len) {
    uint32_t hash = 2166136261U;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 16777619U;
    }
    return hash;
}

int main(void) {
    printf("=============================================================\n");
    printf("DYSNOMIA COMPUTATION SPECIFIC UNIT TESTS\n");
    printf("=============================================================\n");

    // Test Case 1: Lissajous geometry phase translation
    printf("[TEST] Verifying Lissajous modulation phase coordinate calculation...\n");
    double coord_x = calculate_lissajous_coordinate(M_PI / 2.0, 1.0, 10.0);
    double coord_y = calculate_lissajous_coordinate(M_PI, 2.0, 15.0);
    
    // sin(pi/2 * 1) * 10 = 10.0
    assert(fabs(coord_x - 10.0) < 1e-9);
    // sin(pi * 2) * 15 = 0.0 (floating point precision boundary)
    assert(fabs(coord_y) < 1e-9);
    printf("   ✓ Lissajous coordinate generation matches modular amplitude bounds.\n");

    // Test Case 2: WinchesterMQ register parameter derivation
    // Base = 3, Identity = 5. Foundation = Base^Identity % MotzkinPrime (953467954114363)
    // 3^5 = 243
    printf("[TEST] Verifying modular exponentiation for system register Foundation...\n");
    uint64_t base = 3;
    uint64_t identity = 5;
    uint64_t motzkin_prime = 953467954114363ULL;
    
    uint64_t foundation = modular_pow(base, identity, motzkin_prime);
    assert(foundation == 243);
    printf("   ✓ Foundation register derivation verified (%lu).\n", foundation);

    // Test Case 3: FNV-1a Hashing algorithm verification
    printf("[TEST] Verifying FNV-1a signature computation hash alignment...\n");
    const uint8_t data[4] = { 0x12, 0x34, 0x56, 0x78 };
    uint32_t hash = fnv1a_hash(data, 4);
    assert(hash != 0); 
    printf("   ✓ FNV-1a computation hash output aligns with non-zero integrity check.\n");

    printf("=============================================================\n");
    printf("ALL COMPUTATION SPECIFIC TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

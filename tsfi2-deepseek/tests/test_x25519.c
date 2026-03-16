#include <stdio.h>
#include <stdint.h>
#include <string.h>

// Standard X25519 Test Vector from RFC 7748
// Alice's private key: 77076d0a7318a57d3c16c17251b26645df4c2f87ebc0992ab177fba51db92c2a
// Alice's public key:  8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a

extern void tsfi_x25519_ladder(const uint8_t *scalar, const uint8_t *point, uint8_t *out);

int main() {
    uint8_t priv[32] = {0x77,0x07,0x6d,0x0a,0x73,0x18,0xa5,0x7d,0x3c,0x16,0xc1,0x72,0x51,0xb2,0x66,0x45,0xdf,0x4c,0x2f,0x87,0xeb,0xc0,0x99,0x2a,0xb1,0x77,0xfb,0xa5,0x1d,0xb9,0x2c,0x2a};
    uint8_t base[32] = {9};
    uint8_t pub[32];
    uint8_t expected[32] = {0x85,0x20,0xf0,0x09,0x89,0x30,0xa7,0x54,0x74,0x8b,0x7d,0xdc,0xb4,0x3e,0xf7,0x5a,0x0d,0xbf,0x3a,0x0d,0x26,0x38,0x1a,0xf4,0xeb,0xa4,0xa9,0x8e,0xaa,0x9b,0x4e,0x6a};

    printf("[TSFI] Verifying X25519 RFC 7748 Compliance...\n");
    tsfi_x25519_ladder(priv, base, pub);

    printf("Actual:   ");
    for(int i=0; i<32; i++) printf("%02x", pub[i]);
    printf("\nExpected: ");
    for(int i=0; i<32; i++) printf("%02x", expected[i]);
    printf("\n");

    if (memcmp(pub, expected, 32) == 0) {
        printf("[SUCCESS] X25519 Math Verified.\n");
        return 0;
    } else {
        printf("[FAILURE] X25519 Math Mismatch!\n");
        return 1;
    }
}

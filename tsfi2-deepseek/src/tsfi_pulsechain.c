#include "tsfi_pulsechain.h"
#include <string.h>

// Keccak-256 specific constants
#define KECCAK_ROUNDS 24
#define KECCAK_RATE_BYTES 136 // 1088 bits / 8

static const uint64_t RC[KECCAK_ROUNDS] = {
    0x0000000000000001ULL, 0x0000000000008082ULL, 0x800000000000808aULL,
    0x8000000080008000ULL, 0x000000000000808bULL, 0x0000000080000001ULL,
    0x8000000080008081ULL, 0x8000000000008009ULL, 0x000000000000008aULL,
    0x0000000000000088ULL, 0x0000000080008009ULL, 0x000000008000000aULL,
    0x000000008000808bULL, 0x800000000000008bULL, 0x8000000000008089ULL,
    0x8000000000008003ULL, 0x8000000000008002ULL, 0x8000000000000080ULL,
    0x000000000000800aULL, 0x800000008000000aULL, 0x8000000080008081ULL,
    0x8000000000008080ULL, 0x0000000080000001ULL, 0x8000000080008008ULL
};

static const int ROTC[24] = {
    1,  3,  6,  10, 15, 21, 28, 36, 45, 55, 2,  14,
    27, 41, 56, 8,  25, 43, 62, 18, 39, 61, 20, 44
};

static const int PILN[24] = {
    10, 7,  11, 17, 18, 3,  5,  16, 8,  21, 24, 4,
    15, 23, 19, 13, 12, 2,  20, 14, 22, 9,  6,  1
};

#define ROL64(a, offset) (((a) << (offset)) ^ ((a) >> (64 - (offset))))

static void keccakf(uint64_t st[25]) {
    int i, j, round;
    uint64_t t, bc[5];

    for (round = 0; round < KECCAK_ROUNDS; round++) {
        for (i = 0; i < 5; i++)
            bc[i] = st[i] ^ st[i + 5] ^ st[i + 10] ^ st[i + 15] ^ st[i + 20];

        for (i = 0; i < 5; i++) {
            t = bc[(i + 4) % 5] ^ ROL64(bc[(i + 1) % 5], 1);
            for (j = 0; j < 25; j += 5)
                st[j + i] ^= t;
        }

        t = st[1];
        for (i = 0; i < 24; i++) {
            j = PILN[i];
            bc[0] = st[j];
            st[j] = ROL64(t, ROTC[i]);
            t = bc[0];
        }

        for (j = 0; j < 25; j += 5) {
            for (i = 0; i < 5; i++)
                bc[i] = st[j + i];
            for (i = 0; i < 5; i++)
                st[j + i] ^= (~bc[(i + 1) % 5]) & bc[(i + 2) % 5];
        }

        st[0] ^= RC[round];
    }
}

void tsfi_pulse_keccak256(const uint8_t *in, size_t in_len, TsfiPulseHash *out) {
    uint64_t st[25] = {0};
    uint8_t *st8 = (uint8_t *)st;
    size_t in_pos = 0;

    while (in_len >= KECCAK_RATE_BYTES) {
        for (size_t i = 0; i < KECCAK_RATE_BYTES; i++) {
            st8[i] ^= in[in_pos + i];
        }
        keccakf(st);
        in_len -= KECCAK_RATE_BYTES;
        in_pos += KECCAK_RATE_BYTES;
    }

    for (size_t i = 0; i < in_len; i++) {
        st8[i] ^= in[in_pos + i];
    }
    
    // Keccak-256 uses 0x01 padding
    st8[in_len] ^= 0x01;
    st8[KECCAK_RATE_BYTES - 1] ^= 0x80;

    keccakf(st);

    for (size_t i = 0; i < 32; i++) {
        out->data[i] = st8[i];
    }
}
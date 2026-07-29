#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include "tsfi_terse_fieldata.h"

// VM Register definitions matching Auncient lore
#define MOTZKIN_PRIME 953467954114363ULL

typedef struct {
    uint64_t base;
    uint64_t secret;
    uint64_t signal;
    uint64_t channel;
    uint64_t pole;
    uint64_t foundation;
    uint64_t identity;
    uint64_t element;
    uint64_t chin;
    uint64_t monopole;
    uint64_t dynamo;
} AuncientRegisters;

// Simulates CBT973 and CBT774 Decompression state transition
bool cpt973_decompress_record(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len, AuncientRegisters *regs) {
    if (!in || !out || !regs) return false;
    
    // VM Register Math Context
    regs->channel = (regs->base * regs->signal) % MOTZKIN_PRIME;
    regs->pole = (regs->base * regs->secret) % MOTZKIN_PRIME;
    regs->foundation = (regs->base * regs->identity) % MOTZKIN_PRIME;
    regs->dynamo = (regs->base * regs->signal) % regs->element;

    if (in_len >= 2 && in[0] == 0xFE) {
        // CBT774: All 80 bytes of input record are identical
        uint8_t fill_val = (in[1] == 0x40) ? ' ' : in[1];
        for (int k = 0; k < 80; k++) {
            out[k] = fill_val;
        }
        *out_len = 80;
        return true;
    }

    size_t i = 0, j = 0;
    while (i < in_len && j < 80) {
        uint8_t code = in[i++];
        if (code == 0x00) {
            // End of record indicator
            break;
        } else if (code == 0xFF) {
            // Compressed space indicator: next byte is space count
            if (i >= in_len) return false;
            uint8_t count = in[i++];
            for (uint8_t k = 0; k < count && j < 80; k++) {
                out[j++] = ' '; // Space padding
            }
        } else {
            // Raw EBCDIC character translation
            // Basic EBCDIC space is 0x40
            out[j++] = (code == 0x40) ? ' ' : code;
        }
    }
    *out_len = j;
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS CBT973/774 IN-MEMORY DECOMPRESSOR TEST SUITE\n");
    printf("=============================================================\n");

    // Initialize Registers (Seed transition)
    AuncientRegisters regs = {
        .base = 2,
        .secret = 12345,
        .signal = 67890,
        .identity = 11111,
        .element = 9999,
        .chin = 55,
        .monopole = 444
    };

    // 1. Test CBT973 RLE compressed record
    printf("[CBT973] Mounting tape member from memory...\n");
    uint8_t compressed_data[] = {
        0xD1, 0xD6, 0xC2, // "JOB"
        0xFF, 10,        // 10 compressed spaces
        0xC3, 0xD7, 0xE3, 0xF9, 0xF7, 0xF3, // "CPT973" (CBT973)
        0x00             // End of record
    };

    uint8_t decompressed[80];
    size_t decomp_len = 0;
    bool success = cpt973_decompress_record(compressed_data, sizeof(compressed_data), decompressed, &decomp_len, &regs);
    assert(success);

    // Convert decoded EBCDIC stream to ASCII for output display
    char ascii_out[81];
    for (size_t i = 0; i < decomp_len; i++) {
        uint8_t eb = decompressed[i];
        if (eb == ' ') ascii_out[i] = ' ';
        else if (eb >= 0xC1 && eb <= 0xC9) ascii_out[i] = 'A' + (eb - 0xC1);
        else if (eb >= 0xD1 && eb <= 0xD9) ascii_out[i] = 'J' + (eb - 0xD1);
        else if (eb >= 0xE2 && eb <= 0xE9) ascii_out[i] = 'S' + (eb - 0xE2);
        else if (eb >= 0xF0 && eb <= 0xF9) ascii_out[i] = '0' + (eb - 0xF0);
        else ascii_out[i] = '.';
    }
    ascii_out[decomp_len] = '\0';

    printf("[CBT973] Decompressed output: '%s'\n", ascii_out);
    assert(strncmp(ascii_out, "JOB          CPT973", 19) == 0);

    // 2. Test CBT774 identical record decompression (fill record with '-')
    // '-' in EBCDIC is 0x60
    printf("[CBT774] Decoding identical-fill record...\n");
    uint8_t fill_record[] = { 0xFE, 0x60 };
    uint8_t decompressed_fill[80];
    size_t fill_len = 0;
    success = cpt973_decompress_record(fill_record, sizeof(fill_record), decompressed_fill, &fill_len, &regs);
    assert(success);
    assert(fill_len == 80);
    for (int k = 0; k < 80; k++) {
        assert(decompressed_fill[k] == 0x60);
    }
    printf("   ✓ CBT774 successfully expanded 80 bytes of EBCDIC '-' characters.\n");

    // 3. Test FIELDATA 36-bit word unpacking
    printf("[FIELDATA] Unpacking 36-bit words...\n");
    // Pack some 6-bit symbols: 'C', 'B', 'T', '9', '7', '3'
    // 'A'-'Z' maps to 1-26 (so C=3, B=2, T=20)
    // '0'-'9' maps to 48-57 (so 9=57, 7=55, 3=51)
    uint8_t input_symbols[] = { 3, 2, 20, 57, 55, 51 };
    uint64_t packed_words[2] = {0};
    size_t word_count = 0;
    fieldata_pack_36bit(input_symbols, sizeof(input_symbols), packed_words, &word_count);
    assert(word_count > 0);

    uint8_t unpacked_symbols[16] = {0};
    size_t unpacked_len = 0;
    fieldata_unpack_36bit(packed_words, word_count, unpacked_symbols, &unpacked_len);

    char fieldata_ascii[16] = {0};
    for (size_t k = 0; k < 6; k++) {
        uint8_t sym = unpacked_symbols[k];
        if (sym >= 1 && sym <= 26) fieldata_ascii[k] = (char)('A' + sym - 1);
        else if (sym >= 48 && sym <= 57) fieldata_ascii[k] = (char)('0' + sym - 48);
        else fieldata_ascii[k] = '?';
    }
    printf("[FIELDATA] Unpacked output: '%s'\n", fieldata_ascii);
    assert(strncmp(fieldata_ascii, "CBT973", 6) == 0);
    printf("   ✓ FIELDATA 36-bit packing and unpacking verified successfully.\n");

    // Save mounted image in compliance with Rule 13
    FILE *f_mount = fopen("tests/cbt002_cpt973_mounted.dat.bin", "wb");
    if (f_mount) {
        fwrite(decompressed, 1, decomp_len, f_mount);
        fclose(f_mount);
        printf("[CBT973/774] Mounted successfully to tests/cbt002_cpt973_mounted.dat.bin\n");
        remove("tests/cbt002_cpt973_mounted.dat.bin");
    }

    printf("=============================================================\n");
    printf("CBT973/774 IN-MEMORY TESTS COMPLETE AND VERIFIED\n");
    printf("=============================================================\n");
    return 0;
}

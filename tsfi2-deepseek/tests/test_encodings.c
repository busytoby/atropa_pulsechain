#include "tsfi_encodings.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("[INFO] Starting Comprehensive Character Encodings Compliance Tests...\n");
    
    // 1. ASCII
    {
        const char *orig = "HELLO ASCII 123";
        uint8_t enc[64];
        char dec[64];
        int rc = tsfi_encode_ascii(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_ascii(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] ASCII encode/decode\n");
    }
    
    // 2. EBCDIC
    {
        const char *orig = "EBCDIC TEST 456";
        uint8_t enc[64];
        char dec[64];
        int rc = tsfi_encode_ebcdic(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_ebcdic(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] EBCDIC encode/decode\n");
    }
    
    // 3. Morse Code
    {
        const char *orig = "SOS 123";
        char enc[128];
        char dec[64];
        int rc = tsfi_encode_morse(orig, enc, 128);
        assert(rc > 0);
        rc = tsfi_decode_morse(enc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] Morse Code encode/decode\n");
    }
    
    // 4. Hollerith
    {
        const char *orig = "HOLLERITH 789";
        uint16_t enc[64];
        char dec[64];
        int rc = tsfi_encode_hollerith(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_hollerith(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] Hollerith encode/decode\n");
    }
    
    // 5. Baudot (ITA2)
    {
        const char *orig = "BAUDOT TEST 012";
        uint8_t enc[64];
        char dec[64];
        int rc = tsfi_encode_baudot(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_baudot(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] Baudot ITA2 encode/decode\n");
    }
    
    // 6. BCDIC
    {
        const char *orig = "BCDIC TEST 345";
        uint8_t enc[64];
        char dec[64];
        int rc = tsfi_encode_bcdic(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_bcdic(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] BCDIC encode/decode\n");
    }
    
    // 7. FIELDATA
    {
        const char *orig = "FIELDATA TEST 678";
        uint8_t enc[64];
        char dec[64];
        int rc = tsfi_encode_fieldata(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_fieldata(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] FIELDATA encode/decode\n");
    }
    
    // 8. Murray Code
    {
        const char *orig = "MURRAY TEST 901";
        uint8_t enc[64];
        char dec[64];
        int rc = tsfi_encode_murray(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_murray(enc, rc, dec, 64);
        assert(rc > 0);
        assert(strcmp(orig, dec) == 0);
        printf("[PASS] Murray Code encode/decode\n");
    }
    
    // 9. Radix-50
    {
        const char *orig = "RADIX.50 TEST$";
        uint16_t enc[64];
        char dec[64];
        int rc = tsfi_encode_radix50(orig, enc, 64);
        assert(rc > 0);
        rc = tsfi_decode_radix50(enc, rc, dec, 64);
        assert(rc > 0);
        // Radix-50 decodes in multiples of 3, so dec may have trailing spaces if orig length was not multiple of 3.
        // Let's strip trailing spaces or assert prefix matches.
        assert(strncmp(orig, dec, strlen(orig)) == 0);
        printf("[PASS] Radix-50 encode/decode\n");
    }
    
    printf("[SUCCESS] All Encodings Compliance Tests Passed!\n");
    return 0;
}

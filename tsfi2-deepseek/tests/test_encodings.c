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
    // Test 10: Oregon Trail (OT) Baudot (Baud) LLM-Tokenized .dat.bin (DAT)
    {
        const char *bin_path = "tmp/ot_baud_test.dat.bin";
        int rc = tsfi_ot_baud_llm_dat(bin_path);
        assert(rc == 0);
        
        // Open and verify content
        FILE *f = fopen(bin_path, "rb");
        assert(f != NULL);
        uint32_t count = 0;
        assert(fread(&count, sizeof(uint32_t), 1, f) == 1);
        assert(count > 0);
        
        uint32_t tokens[128];
        assert(fread(tokens, sizeof(uint32_t), count, f) == count);
        fclose(f);
        
        // Convert tokens back to Baudot and decode
        uint8_t baud_buf[128];
        for (uint32_t i = 0; i < count; i++) {
            baud_buf[i] = (uint8_t)tokens[i];
        }
        
        char status_dec[128];
        rc = tsfi_decode_baudot(baud_buf, count, status_dec, 128);
        assert(rc > 0);
        assert(strstr(status_dec, "MILES") != NULL);
        assert(strstr(status_dec, "FOOD") != NULL);
        assert(strstr(status_dec, "OXEN") != NULL);
        
        remove(bin_path);
        printf("[PASS] Oregon Trail Baudot LLM DAT integration\n");
    }
    
    printf("[SUCCESS] All Encodings Compliance Tests Passed!\n");
    return 0;
}

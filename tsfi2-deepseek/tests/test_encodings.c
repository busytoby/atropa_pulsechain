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
        extern bool g_gguf_acab_found;
        extern uint8_t g_gguf_acab_root[32];
        
        // Test Path A: Without ACAB Root
        g_gguf_acab_found = false;
        const char *bin_path = "tmp/ot_baud_test.dat.bin";
        int rc = tsfi_ot_baud_llm_dat(bin_path);
        assert(rc == 0);
        
        FILE *f = fopen(bin_path, "rb");
        assert(f != NULL);
        uint32_t count = 0;
        assert(fread(&count, sizeof(uint32_t), 1, f) == 1);
        assert(count > 0);
        uint32_t tokens[128];
        assert(fread(tokens, sizeof(uint32_t), count, f) == count);
        fclose(f);
        remove(bin_path);
        
        // Test Path B: With ACAB Root active
        g_gguf_acab_found = true;
        memset(g_gguf_acab_root, 0xAB, 32);
        rc = tsfi_ot_baud_llm_dat(bin_path);
        assert(rc == 0);
        
        f = fopen(bin_path, "rb");
        assert(f != NULL);
        assert(fread(&count, sizeof(uint32_t), 1, f) == 1);
        assert(count > 0);
        assert(fread(tokens, sizeof(uint32_t), count, f) == count);
        fclose(f);
        
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
        g_gguf_acab_found = false; // reset
        printf("[PASS] Oregon Trail Baudot LLM DAT on the ACAB integration\n");
    }
    
    // Test 11: ER and EER database bridging from basic model
    {
        const char *bin_path = "tmp/ot_basic_model.dat.bin";
        int rc = tsfi_ot_baud_llm_dat(bin_path);
        assert(rc == 0);
        
        TSFiEerDatabase eer_db;
        rc = tsfi_eer_bridge_ot_acab(&eer_db, bin_path);
        assert(rc == 0);
        
        // Assertions verifying ER entities and relations
        assert(eer_db.incident_count == 1);
        assert(eer_db.agency_count == 2);
        assert(eer_db.channel_count == 1);
        assert(eer_db.channels[0].channel_id == 0x0200); // Tapped ACAB channel
        
        // Assertions verifying EER specializations
        assert(eer_db.responds_count == 1);
        if (eer_db.incidents[0].type == 1) {
            assert(eer_db.responds[0].agency_id == 101); // NORAD responds to NuclearAlert
        } else {
            assert(eer_db.responds[0].agency_id == 102); // IRS responds to TaxAuditConflict
        }
        
        remove(bin_path);
        printf("[PASS] ER & EER bridge integration tests\n");
    }
    
    // Test 12: Encodings and Demodulation Improvements
    {
        // 12.1 Hamming(7,4) FEC test
        const uint8_t raw_in[4] = {0x12, 0x34, 0x56, 0x78};
        uint8_t coded[8];
        uint8_t raw_out[4];
        tsfi_encode_hamming74(raw_in, 4, coded);
        
        // Corrupt one bit in a codeword (syndrome error correction check)
        coded[0] ^= 1; 
        
        tsfi_decode_hamming74(coded, 8, raw_out);
        assert(raw_out[0] == 0x12);
        assert(raw_out[1] == 0x34);
        assert(raw_out[2] == 0x56);
        assert(raw_out[3] == 0x78);
        printf("[PASS] Hamming(7,4) FEC recovery\n");
        
        // 12.2 Dynamic read-write Tone Wheel test
        uint16_t wheel[80];
        memset(wheel, 0, sizeof(wheel));
        int p_rc = tsfi_punch_tone_wheel(wheel, 80, 5, 'A');
        assert(p_rc == 0);
        assert(wheel[5] == 0x801); // 12-zone + row 1
        printf("[PASS] Dynamic Tone Wheel punch\n");
        
        // 12.3 EER Declarative Datalog Specialization test
        TSFiEerDatabase db;
        tsfi_eer_db_init(&db);
        int assigned = tsfi_eer_datalog_specialization(&db, "NuclearAlert", "Critical");
        assert(assigned == 1);
        assert(db.agency_count == 1);
        assert(db.agencies[0].agency_id == 101); // NORAD assigned
        printf("[PASS] Declarative Datalog EER specialization\n");
        
        // 12.4 Vulkan-based PLL visualization test
        float ndc_x = 0.0f, ndc_y = 0.0f;
        tsfi_pll_vulkan_project(2.5f, -1.57f, &ndc_x, &ndc_y);
        assert(ndc_x > 0.0f && ndc_y < 0.0f);
        printf("[PASS] Vulkan-based PLL project\n");
    }
    
    printf("[SUCCESS] All Encodings Compliance Tests Passed!\n");
    return 0;
}

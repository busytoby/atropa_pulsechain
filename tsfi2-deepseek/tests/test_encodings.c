#include "tsfi_encodings.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void mock_sap_handler(TSFiEerDatabase *db, const uint8_t *payload, int len) {
    (void)payload; (void)len;
    tsfi_eer_insert_incident(db, 8888, 1, 1782000000U, 1);
}

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
    
    // Test 13: OT Accumulator (OT) Baudot (Baud) LLM-Tokenized DAT on ACAB
    {
        const char *bin_path = "tmp/ot_accum_basic.dat.bin";
        int rc = tsfi_ot_accum_baud_llm_dat(bin_path);
        assert(rc == 0);
        
        TSFiEerDatabase db;
        rc = tsfi_eer_bridge_ot_accum_acab(&db, bin_path);
        assert(rc == 0);
        
        // Assertions verifying ER/EER details
        assert(db.incident_count == 1);
        assert(db.agency_count == 2);
        assert(db.channel_count == 1);
        assert(db.channels[0].channel_id == 0x0200); // Tapped ACAB channel
        
        remove(bin_path);
        printf("[PASS] OT Accumulator Baud LLM DAT on ACAB bridge tests\n");
    }
    
    // Test 14: Extended System Improvements (SECDED, SPSC, Datalog Cascade, PI PLL)
    {
        // 14.1 SECDED Hamming(8,4) test
        const uint8_t raw_in[4] = {0xA5, 0x5A, 0x12, 0x34};
        uint8_t coded[8];
        uint8_t raw_out[4];
        tsfi_encode_hamming84(raw_in, 4, coded);
        
        // Corrupt 1 bit: SECDED must correct it
        coded[0] ^= 2; 
        int err = tsfi_decode_hamming84(coded, 8, raw_out);
        assert(err == 0);
        assert(raw_out[0] == 0xA5);
        
        // Corrupt 2 bits: SECDED must detect double error
        coded[0] ^= 4; 
        err = tsfi_decode_hamming84(coded, 8, raw_out);
        assert(err == 1); // Double error detected
        printf("[PASS] SECDED Hamming(8,4) double-error detection\n");
        
        // 14.2 Lock-free SPSC Accumulator queue test
        TSFiOTAccumulator acc;
        tsfi_ot_accumulator_init(&acc);
        int push_rc = tsfi_ot_accumulator_spsc_push(&acc, "coord_lock_free", 88.5f);
        assert(push_rc == 0);
        assert(acc.count == 1);
        assert(acc.cumulative_potential == 88.5f);
        printf("[PASS] Lock-free SPSC Accumulator queue push\n");
        
        // 14.3 Cascading Datalog rules EER resolver test
        TSFiEerDatabase db;
        tsfi_eer_db_init(&db);
        int cascade_rc = tsfi_eer_datalog_cascade(&db, "CriticalOutage", "PowerOut", "BatteryLow");
        assert(cascade_rc == 1);
        assert(db.agency_count == 1);
        assert(db.agencies[0].agency_id == 101); // NORAD assigned
        printf("[PASS] Cascading Datalog EER specialization\n");
        
        // 14.4 PI PLL Loop Tuning test
        float out_v = 0.0f, next_int = 0.0f;
        tsfi_pll_pi_tune(1.2f, 0.5f, 2.0f, 0.5f, 0.1f, &out_v, &next_int);
        assert(out_v > 0.0f);
        assert(next_int > 0.61f && next_int < 0.63f);
        printf("[PASS] PI PLL Loop Tuning\n");
    }
    
    // Test 15: Third-Generation Systems Improvements
    {
        // 15.1 Adaptive Baudot Dictionary Shifting test
        uint8_t custom_ltrs[32] = {
            ' ', 'E', '\n', 'A', ' ', 'S', 'I', 'U', '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
            'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 'O', 'B', 'G', 0, 'M', 'X', 'V', 'Z'
        };
        uint8_t custom_figs[32] = {
            ' ', '3', '\n', '-', ' ', '\'', '8', '7', '\r', '$', '4', '\'', ',', '!', ':', '(',
            '5', '+', ')', '2', '#', '6', '0', '1', '9', '?', '&', 0, '.', '/', '=', 0
        };
        int u_rc = tsfi_baudot_update_maps(custom_ltrs, custom_figs);
        assert(u_rc == 0);
        printf("[PASS] Adaptive Baudot map update\n");
        
        // 15.2 BCH(15,7) DEC-TED test
        const uint8_t raw_msg[4] = {0x0A, 0x1F, 0x5C, 0x7E};
        uint16_t bch_coded[4];
        uint8_t bch_decoded[4];
        tsfi_encode_bch15_7(raw_msg, 4, bch_coded);
        
        // Introduce 2 bit errors in one codeword: BCH must still correct it (DEC)
        bch_coded[0] ^= 0x03;
        int uncor = tsfi_decode_bch15_7(bch_coded, 4, bch_decoded);
        assert(uncor == 0);
        assert(bch_decoded[0] == 0x0A);
        printf("[PASS] BCH(15,7) double-error correction\n");
        
        // 15.3 PID PLL with AGC Tuning test
        float pid_v = 0.0f, next_pi_i = 0.0f, next_pi_e = 0.0f, agc_g = 0.0f;
        tsfi_pll_pid_agc_tune(1.5f, 0.5f, 1.0f, 2.0f, 0.5f, 0.1f, 0.1f, 0.5f, &pid_v, &next_pi_i, &next_pi_e, &agc_g);
        assert(pid_v > 0.0f);
        assert(agc_g == 2.0f); // target 1.0 / amp 0.5
        printf("[PASS] PID PLL with AGC Tuning\n");
        
        // 15.4 EER Referential Integrity Cascade Delete test
        TSFiEerDatabase db;
        tsfi_eer_db_init(&db);
        tsfi_eer_insert_incident(&db, 5001, 2, 1782000000U, 1);
        tsfi_eer_insert_agency(&db, 101, "NORAD", 1, 1);
        tsfi_eer_link_response(&db, 101, 5001);
        
        assert(db.incident_count == 1);
        assert(db.responds_count == 1);
        
        int del_rc = tsfi_eer_delete_incident(&db, 5001);
        assert(del_rc == 0);
        assert(db.incident_count == 0);
        assert(db.responds_count == 0); // Cascaded delete verified!
        printf("[PASS] EER database referential integrity cascade deletion\n");
    }
    
    // Test 16: Cryptographic Oblivious Transfer (OT) Baudot LLM DAT on ACAB
    {
        const char *bin_path = "tmp/ot_crypto_basic.dat.bin";
        int rc = tsfi_ot_crypto_baud_llm_dat(bin_path, 1, "SECRET_A", "SECRET_B");
        assert(rc == 0);
        
        TSFiEerDatabase db;
        rc = tsfi_eer_bridge_ot_crypto_acab(&db, bin_path);
        assert(rc == 0);
        
        assert(db.incident_count == 1);
        assert(db.incidents[0].incident_id == 4001);
        assert(db.agency_count == 2);
        assert(db.channel_count == 1);
        
        remove(bin_path);
        printf("[PASS] Cryptographic Oblivious Transfer Baud LLM DAT on ACAB bridge tests\n");
    }
    
    // Test 17: STANAG 5066 Framed Cryptographic Oblivious Transfer (OT)
    {
        const char *bin_path = "tmp/ot_crypto_stanag.dat.bin";
        int rc = tsfi_ot_crypto_stanag_baud_llm_dat(bin_path, 0, "SECRET_A", "SECRET_B");
        assert(rc == 0);
        
        TSFiEerDatabase db;
        rc = tsfi_eer_bridge_ot_crypto_stanag_acab(&db, bin_path);
        assert(rc == 0);
        
        assert(db.incident_count == 1);
        assert(db.incidents[0].incident_id == 5000); // Choice 0 mapped to 5000
        assert(db.agency_count == 2);
        assert(db.channel_count == 1);
        assert(db.channels[0].encryption_type == 3); // STANAG 5066
        
        remove(bin_path);
        printf("[PASS] STANAG 5066 Framed Oblivious Transfer Baud LLM DAT on ACAB bridge tests\n");
    }
    
    // Test 18: Fourth-Generation Systems Improvements
    {
        int reg_rc = tsfi_stanag_register_route(0x7A, mock_sap_handler);
        assert(reg_rc == 0);
        
        TSFiEerDatabase db;
        tsfi_eer_db_init(&db);
        int route_rc = tsfi_stanag_route_frame(&db, 0x7A, (const uint8_t *)"hello", 5);
        assert(route_rc == 0);
        assert(db.incident_count == 1);
        assert(db.incidents[0].incident_id == 8888);
        printf("[PASS] Dynamic STANAG SAP routing\n");
        
        // 18.2 Galois Field GF(2^8) Reed-Solomon(15,11) test
        const uint8_t rs_in[11] = {'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd'};
        uint8_t rs_coded[15];
        uint8_t rs_out[11];
        tsfi_encode_rs15_11(rs_in, 11, rs_coded);
        
        // Corrupt 1 byte: Reed-Solomon must correct it
        rs_coded[3] ^= 0xFF;
        int rs_dec_rc = tsfi_decode_rs15_11(rs_coded, 15, rs_out);
        assert(rs_dec_rc == 0);
        assert(memcmp(rs_in, rs_out, 11) == 0);
        printf("[PASS] Reed-Solomon(15,11) single-byte error correction\n");
        
        // 18.3 Kalman Filter Noise Estimation test
        float state = 0.0f;
        float cov = 1.0f;
        tsfi_pll_kalman_estimate(1.5f, &state, &cov, 0.1f, 0.5f);
        assert(state > 0.0f);
        printf("[PASS] Kalman filter noise estimation\n");
        
        // 18.4 EER Declarative Relational Invariant Audits test
        tsfi_eer_db_init(&db);
        tsfi_eer_insert_incident(&db, 9999, 1, 1782000000U, 1); // DEFCON 1
        int audit_fail = tsfi_eer_audit_invariants(&db);
        assert(audit_fail == -2); // Failed because NORAD is not responding
        
        tsfi_eer_insert_agency(&db, 101, "NORAD", 1, 1);
        tsfi_eer_link_response(&db, 101, 9999);
        int audit_pass = tsfi_eer_audit_invariants(&db);
        assert(audit_pass == 0); // Invariants validated successfully!
        printf("[PASS] EER database relational invariant audits\n");
    }
    
    printf("[SUCCESS] All Encodings Compliance Tests Passed!\n");
    return 0;
}

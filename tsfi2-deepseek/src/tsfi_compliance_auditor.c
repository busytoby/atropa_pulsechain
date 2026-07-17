#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_mainframe_fips.h"

int main(int argc, char **argv) {
    printf("=== Auncient Mainframe FIPS Compliance Auditor ===\n");
    if (argc < 2) {
        printf("Usage:\n");
        printf("  %s --doc <file_path>           Audit FIPS 38 layout compliance\n", argv[0]);
        printf("  %s --power <nominal> <actual>  Audit FIPS 94 power quality\n", argv[0]);
        printf("  %s --numeric <string>          Audit FIPS 69 string format\n", argv[0]);
        printf("  %s --label <80_char_block>     Audit FIPS 79 tape labels\n", argv[0]);
        printf("  %s --auth <user> <lau> <pin>   Verify FIPS 48 user badge auth\n", argv[0]);
        printf("  %s --basic <file_path>         Execute FIPS 68 Minimal BASIC script\n", argv[0]);
        printf("  %s --cipher <plain_text>       Run FIPS 81 CBC mode validation\n", argv[0]);
        printf("  %s --app <record> <value>      Audit FIPS 73 application controls\n", argv[0]);
        printf("  %s --ascii-ebcdic <ascii_str>  Translate character sets (FIPS 1-1)\n", argv[0]);
        printf("  %s --serial-parity <byte_val>  Check serial bits & parity (FIPS 16-1)\n", argv[0]);
        printf("  %s --access-control <role> <act> Check dataset authorization (FIPS 41)\n", argv[0]);
        printf("  %s --mac <plain_text>          Generate DES Data Auth MAC (FIPS 113)\n", argv[0]);
        printf("  %s --x25 <payload_str> <chan>  Encapsulate X.25 packet frame (FIPS 100)\n", argv[0]);
        printf("  %s --password <pass_str>       Validate password complexity (FIPS 112)\n", argv[0]);
        printf("  %s --gks <cmd_byte> <pts_cnt>  Parse graphics primitives (FIPS 120)\n", argv[0]);
        printf("  %s --agency <agency_code>      Resolve federal agency code (FIPS 95)\n", argv[0]);
        printf("  %s --datetime <date> <time>    Validate date & time interchange (FIPS 4-1 / 58-1)\n", argv[0]);
        return 0;
    }

    if (strcmp(argv[1], "--doc") == 0 && argc >= 3) {
        FILE *f = fopen(argv[2], "r");
        if (!f) {
            printf("Error: Could not open document %s\n", argv[2]);
            return 1;
        }
        char buf[4096];
        size_t len = fread(buf, 1, sizeof(buf) - 1, f);
        buf[len] = '\0';
        fclose(f);

        int completeness = 0;
        tsfi_fips38_audit_document(buf, &completeness);
        printf("[FIPS 38 AUDIT] Completeness Score: %d%%\n", completeness);
        if (completeness == 100) {
            printf("RESULT: FULLY COMPLIANT\n");
        } else {
            printf("RESULT: NON-COMPLIANT\n");
        }
    } else if (strcmp(argv[1], "--power") == 0 && argc >= 4) {
        double nominal = atof(argv[2]);
        double actual = atof(argv[3]);
        tsfi_fips94_monitor mon;
        tsfi_fips94_power_init(&mon);
        int fault = 0;
        int res = tsfi_fips94_audit_voltage(&mon, nominal, actual, &fault);
        printf("[FIPS 94 AUDIT] Nominal: %.1fV, Actual: %.1fV\n", nominal, actual);
        if (res == 0) {
            printf("RESULT: SAFE POWER LEVEL\n");
        } else {
            printf("RESULT: FAULT DETECTED (Type: %d)\n", fault);
        }
    } else if (strcmp(argv[1], "--numeric") == 0 && argc >= 3) {
        double parsed = 0.0;
        int res = tsfi_fips69_parse_numeric(argv[2], &parsed);
        printf("[FIPS 69 AUDIT] Testing string: '%s'\n", argv[2]);
        if (res == 0) {
            printf("RESULT: COMPLIANT (Value: %.6f)\n", parsed);
        } else {
            printf("RESULT: FORMAT VIOLATION (Error Code: %d)\n", res);
        }
    } else if (strcmp(argv[1], "--label") == 0 && argc >= 3) {
        char file_id[17];
        uint32_t serial = 0;
        int block_count = 0;
        int res = tsfi_fips79_parse_label((const uint8_t*)argv[2], file_id, &serial, &block_count);
        printf("[FIPS 79 AUDIT] Testing label block: '%s'\n", argv[2]);
        if (res == 0) {
            printf("RESULT: VALID LABEL (ID: %s, Serial: %u, Blocks: %d)\n", file_id, serial, block_count);
        } else {
            printf("RESULT: INVALID LABEL STRUCTURE (Error Code: %d)\n", res);
        }
    } else if (strcmp(argv[1], "--auth") == 0 && argc >= 5) {
        tsfi_fips48_authenticator auth;
        tsfi_fips48_init(&auth);
        uint32_t lau = (uint32_t)strtoul(argv[3], NULL, 0);
        uint16_t pin = (uint16_t)atoi(argv[4]);
        tsfi_fips48_register_lau_badge(&auth, argv[2], lau, pin);
        int out_status = 0;
        int res = tsfi_fips48_authenticate(&auth, lau, pin, &out_status);
        printf("[FIPS 48 AUDIT] User: %s, Badge ID: %u\n", argv[2], lau);
        if (res == 0 && out_status == 0) {
            printf("RESULT: AUTHENTICATION GRANTED\n");
        } else {
            printf("RESULT: ACCESS DENIED\n");
        }
    } else if (strcmp(argv[1], "--basic") == 0 && argc >= 3) {
        FILE *f = fopen(argv[2], "r");
        if (!f) {
            printf("Error: Could not open BASIC file %s\n", argv[2]);
            return 1;
        }
        char code[1024];
        size_t len = fread(code, 1, sizeof(code) - 1, f);
        code[len] = '\0';
        fclose(f);

        tsfi_fips68_basic interpreter;
        tsfi_fips68_basic_init(&interpreter);
        char out_buf[512];
        tsfi_fips68_basic_run(&interpreter, code, out_buf, sizeof(out_buf));
        printf("[FIPS 68 AUDIT] Executing minimal BASIC program:\n%s", out_buf);
    } else if (strcmp(argv[1], "--cipher") == 0 && argc >= 3) {
        tsfi_crypto_subsystem crypto;
        tsfi_crypto_init(&crypto);
        tsfi_crypto_load_master_key(&crypto, 0x1122334455667788ULL);
        uint8_t iv[8] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08};
        uint8_t plain[16] = {0};
        snprintf((char*)plain, sizeof(plain), "%s", argv[2]);
        uint8_t cipher[16] = {0};
        uint8_t decrypted[16] = {0};
        tsfi_fips81_encrypt_cbc(&crypto, plain, cipher, 2, iv, 1);
        tsfi_fips81_decrypt_cbc(&crypto, cipher, decrypted, 2, iv, 1);
        printf("[FIPS 81 AUDIT] Testing Plaintext: '%s'\n", argv[2]);
        printf("RESULT: Decrypted recovered string: '%.16s'\n", (char*)decrypted);
    } else if (strcmp(argv[1], "--app") == 0 && argc >= 4) {
        tsfi_fips73_auditor auditor;
        tsfi_fips73_audit_init(&auditor);
        int val = atoi(argv[3]);
        int res = tsfi_fips73_audit_transaction(&auditor, argv[2], val);
        printf("[FIPS 73 AUDIT] Record: '%s', Payload: %d\n", argv[2], val);
        if (res == 0) {
            printf("RESULT: VALID TRANSACTION\n");
        } else {
            printf("RESULT: VALIDATION FAILURE\n");
        }
    } else if (strcmp(argv[1], "--ascii-ebcdic") == 0 && argc >= 3) {
        uint8_t ebcdic[256];
        char ascii[256];
        int len = strlen(argv[2]);
        if (len > 250) len = 250;
        tsfi_fips1_ascii_to_ebcdic(argv[2], ebcdic, len);
        tsfi_fips1_ebcdic_to_ascii(ebcdic, ascii, len);
        ascii[len] = '\0';
        printf("[FIPS 1-1 AUDIT] Original: '%s'\n", argv[2]);
        printf("RESULT: Translated back: '%s'\n", ascii);
    } else if (strcmp(argv[1], "--serial-parity") == 0 && argc >= 3) {
        uint8_t byte = (uint8_t)atoi(argv[2]);
        uint16_t serialized = 0;
        uint8_t deserialized = 0;
        tsfi_fips16_serialize(byte, 1, &serialized);
        int check = tsfi_fips16_deserialize(serialized, 1, &deserialized);
        printf("[FIPS 16-1 AUDIT] Byte: %d, Serialized Bits: 0x%03X\n", byte, serialized);
        if (check == 0) {
            printf("RESULT: PARITY CHECK PASSED\n");
        } else {
            printf("RESULT: PARITY CHECK FAILED\n");
        }
    } else if (strcmp(argv[1], "--access-control") == 0 && argc >= 4) {
        int res = tsfi_fips41_authorize(argv[2], argv[3]);
        printf("[FIPS 41 AUDIT] Role: %s, Action: %s\n", argv[2], argv[3]);
        if (res == 0) {
            printf("RESULT: AUTHORIZED\n");
        } else {
            printf("RESULT: ACCESS DENIED\n");
        }
    } else if (strcmp(argv[1], "--mac") == 0 && argc >= 3) {
        tsfi_crypto_subsystem crypto;
        tsfi_crypto_init(&crypto);
        tsfi_crypto_load_master_key(&crypto, 0x1122334455667788ULL);
        uint8_t mac[8];
        int len = strlen(argv[2]);
        tsfi_fips113_generate_mac(&crypto, (const uint8_t*)argv[2], len, mac, 1);
        printf("[FIPS 113 AUDIT] Data: '%s'\n", argv[2]);
        printf("RESULT: Generated MAC: 0x%02X%02X%02X%02X%02X%02X%02X%02X\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], mac[6], mac[7]);
    } else if (strcmp(argv[1], "--x25") == 0 && argc >= 4) {
        uint8_t packet[256];
        int packet_len = 0;
        uint16_t chan = (uint16_t)atoi(argv[3]);
        int len = strlen(argv[2]);
        tsfi_fips100_encapsulate((const uint8_t*)argv[2], len, chan, packet, &packet_len);
        printf("[FIPS 100 AUDIT] Encapsulated X.25 frame on channel %d: length %d\n", chan, packet_len);
        printf("RESULT: Frame header bytes: 0x%02X 0x%02X 0x%02X\n", packet[0], packet[1], packet[2]);
    } else if (strcmp(argv[1], "--password") == 0 && argc >= 3) {
        int score = 0;
        int res = tsfi_fips112_validate_password(argv[2], &score);
        printf("[FIPS 112 AUDIT] Password input: '%s'\n", argv[2]);
        if (res == 0) {
            printf("RESULT: VALID (Complexity Score: %d/4)\n", score);
        } else {
            printf("RESULT: INVALID PASSWORD FORMAT\n");
        }
    } else if (strcmp(argv[1], "--gks") == 0 && argc >= 4) {
        uint8_t stream[8];
        stream[0] = (uint8_t)atoi(argv[2]);
        stream[1] = (uint8_t)atoi(argv[3]);
        // Dummy point data
        stream[2] = 0; stream[3] = 0; stream[4] = 0; stream[5] = 0;
        int prim_type = 0, pts_cnt = 0;
        int res = tsfi_fips120_parse_gks_primitive(stream, 6, &prim_type, &pts_cnt);
        printf("[FIPS 120 AUDIT] Type code: %d, Points: %d\n", stream[0], stream[1]);
        if (res == 0) {
            printf("RESULT: VALID GKS PRIMITIVE\n");
        } else {
            printf("RESULT: FORMAT ERROR\n");
        }
    } else if (strcmp(argv[1], "--agency") == 0 && argc >= 3) {
        char name[128];
        int res = tsfi_fips95_resolve_agency(argv[2], name, sizeof(name));
        printf("[FIPS 95 AUDIT] Code: '%s'\n", argv[2]);
        if (res == 0) {
            printf("RESULT: Resolved Agency: %s\n", name);
        } else {
            printf("RESULT: UNRESOLVED CODE\n");
        }
    } else if (strcmp(argv[1], "--datetime") == 0 && argc >= 4) {
        int date_res = tsfi_fips4_validate_date(argv[2]);
        int time_res = tsfi_fips58_validate_time(argv[3]);
        printf("[FIPS 4-1 / 58-1 AUDIT] Date: '%s', Time: '%s'\n", argv[2], argv[3]);
        if (date_res == 0 && time_res == 0) {
            printf("RESULT: VALID DATE AND TIME FORMATS\n");
        } else {
            printf("RESULT: FORMAT VIOLATION (Date: %d, Time: %d)\n", date_res, time_res);
        }
    } else {
        printf("Unknown option or insufficient arguments.\n");
    }

    return 0;
}

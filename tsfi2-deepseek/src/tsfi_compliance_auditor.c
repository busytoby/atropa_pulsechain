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
    } else {
        printf("Unknown option or insufficient arguments.\n");
    }

    return 0;
}

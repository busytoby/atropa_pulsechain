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
    } else {
        printf("Unknown option or insufficient arguments.\n");
    }

    return 0;
}

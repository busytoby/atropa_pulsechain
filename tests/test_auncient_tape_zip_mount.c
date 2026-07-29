#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>
#include "tsfi_parc_tape_catalog.h"
#include "tsfi_parc_tape_label_yul.h"

// Basic EBCDIC mapping for verification
static char ebcdic_to_ascii(uint8_t ebcdic) {
    static const char ebcdic_map[256] = {
        [' '] = ' ',
        [0xC1] = 'A', [0xC2] = 'B', [0xC3] = 'C', [0xC4] = 'D', [0xC5] = 'E',
        [0xC6] = 'F', [0xC7] = 'G', [0xC8] = 'H', [0xC9] = 'I',
        [0xD1] = 'J', [0xD2] = 'K', [0xD3] = 'L', [0xD4] = 'M', [0xD5] = 'N',
        [0xD6] = 'O', [0xD7] = 'P', [0xD8] = 'Q', [0xD9] = 'R',
        [0xE2] = 'S', [0xE3] = 'T', [0xE4] = 'U', [0xE5] = 'V', [0xE6] = 'W',
        [0xE7] = 'X', [0xE8] = 'Y', [0xE9] = 'Z',
        [0xF0] = '0', [0xF1] = '1', [0xF2] = '2', [0xF3] = '3', [0xF4] = '4',
        [0xF5] = '5', [0xF6] = '6', [0xF7] = '7', [0xF8] = '8', [0xF9] = '9'
    };
    char c = ebcdic_map[ebcdic];
    return (c == '\0') ? '?' : c;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TAPE RED/BLACK ZIP MOUNT INTEGRATION TEST SUITE\n");
    printf("=============================================================\n");

    // 1. Create a dummy VOL1.HDR file with restrictive clearance level (e.g. level 50)
    printf("[RED/BLACK] Packaging mock labeled virtual tape members...\n");
    uint8_t mock_hdr[720];
    tsfi_tape_label_yul_format_full_header(
        mock_hdr, "VOL888", "SEC_TAPE.dat.bin", 50, /* Security Level = 50 */
        0.0f, 0.0f, 100.0f, 100.0f, NULL, NULL,
        0.0f, 1, 1, 1, 32, 30
    );

    FILE *f_hdr = fopen("tests/VOL1.HDR", "wb");
    assert(f_hdr != NULL);
    fwrite(mock_hdr, 1, 720, f_hdr);
    fclose(f_hdr);

    // Create target data file
    FILE *f_data = fopen("tests/TAPE_DATA.txt", "wb");
    assert(f_data != NULL);
    // Write an EBCDIC-encoded payload record
    uint8_t payload[80];
    memset(payload, 0x40, sizeof(payload)); // EBCDIC spaces
    payload[0] = 0xC1; // 'A'
    payload[1] = 0xC2; // 'B'
    payload[2] = 0xC3; // 'C'
    fwrite(payload, 1, 80, f_data);
    fclose(f_data);

    // Package them into a zip and rename to compliant .dat.bin layout
    system("zip -q -j tests/labeled_temp.zip tests/VOL1.HDR tests/TAPE_DATA.txt");
    system("mv tests/labeled_temp.zip tests/labeled_tape.dat.bin");

    // Clean up local temp files
    remove("tests/VOL1.HDR");
    remove("tests/TAPE_DATA.txt");

    // 2. RED RAIL: Test Access Denied with low clearance (e.g. clearance level 1)
    printf("[RED RAIL] Verifying access rejection under low clearance (level 1)...\n");
    FILE *f_denied = tsfi_tape_open_guarded("tests/labeled_tape.dat.bin", "rb", 1);
    assert(f_denied == NULL);
    printf("   ✓ RED RAIL successfully blocked unauthorized access.\n");

    // 3. RED/BLACK RAIL: Test Access Granted with sufficient clearance (e.g. clearance level 3)
    printf("[RED/BLACK RAIL] Verifying access authorization under high clearance (level 3)...\n");
    FILE *f_granted = tsfi_tape_open_guarded("tests/labeled_tape.dat.bin", "rb", 3);
    assert(f_granted != NULL);
    printf("   ✓ RED RAIL authorized access successfully.\n");

    // Read the payload from the stream (BLACK RAIL dynamic conduction)
    uint8_t read_buf[80];
    size_t nread = fread(read_buf, 1, 80, f_granted);
    assert(nread == 80);
    assert(ebcdic_to_ascii(read_buf[0]) == 'A');
    assert(ebcdic_to_ascii(read_buf[1]) == 'B');
    assert(ebcdic_to_ascii(read_buf[2]) == 'C');
    printf("   ✓ BLACK RAIL verified correct dynamic payload read: %c%c%c\n",
           ebcdic_to_ascii(read_buf[0]), ebcdic_to_ascii(read_buf[1]), ebcdic_to_ascii(read_buf[2]));

    pclose(f_granted);
    remove("tests/labeled_tape.dat.bin");

    printf("=============================================================\n");
    printf("ALL RED/BLACK ZIP TAPE INTEGRATION CHECKS PASSED CLEANLY\n");
    printf("=============================================================\n");
    return 0;
}

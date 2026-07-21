#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_tape_label_yul.h"

// Yul DDL Offset Resolvers (matching Yul TapeLabelDDL contract layout)
static int yul_get_label_offset(int label_type, int field_id) {
    switch (label_type) {
        case TAPE_LABEL_VOL1:
            switch (field_id) {
                case 0: return 0;   // label_id ("VOL1")
                case 1: return 4;   // volume_id
                case 2: return 37;  // owner_id
                default: return 0;
            }
        case TAPE_LABEL_HDR1:
            switch (field_id) {
                case 0: return 80;  // label_id ("HDR1")
                case 1: return 84;  // file_id (.dat.bin)
                case 2: return 127; // security_code
                default: return 80;
            }
        case TAPE_LABEL_HDR2:
            switch (field_id) {
                case 0: return 160; // label_id ("HDR2")
                case 1: return 164; // record_format ("F")
                case 2: return 165; // block_size ("00512")
                default: return 160;
            }
        default: return 0;
    }
}

int tsfi_tape_label_yul_format_header(uint8_t *header_buf, const char *volume_id, const char *file_id, uint8_t security_level) {
    if (!header_buf || !volume_id || !file_id) return -1;

    // Fill 240-byte header sequence buffer with space padding
    memset(header_buf, ' ', 240);

    // 1. VOL1 Label (Offset 0..79)
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 0), "VOL1", 4);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 1), volume_id, strlen(volume_id) > 6 ? 6 : strlen(volume_id));
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 2), "AUNCIENT_ZMM01", 14);

    // 2. HDR1 Label (Offset 80..159)
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 0), "HDR1", 4);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 1), file_id, strlen(file_id) > 17 ? 17 : strlen(file_id));
    header_buf[yul_get_label_offset(TAPE_LABEL_HDR1, 2)] = '0' + (security_level > 3 ? 3 : security_level);

    // 3. HDR2 Label (Offset 160..239)
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 0), "HDR2", 4);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 1), "F", 1); // Fixed-length record format
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 2), "00512", 5); // 512-byte quadtree block size

    return 0;
}

int tsfi_tape_label_yul_format_trailer(uint8_t *trailer_buf, int label_type, uint32_t block_count) {
    if (!trailer_buf) return -1;

    memset(trailer_buf, ' ', 80);

    if (label_type == TAPE_LABEL_EOF1) {
        memcpy(trailer_buf, "EOF1", 4);
    } else if (label_type == TAPE_LABEL_EOV1) {
        memcpy(trailer_buf, "EOV1", 4);
    } else {
        return -2;
    }

    // Write 6-digit block count at offset 54
    snprintf((char *)(trailer_buf + 54), 7, "%06u", block_count);
    return 0;
}

int tsfi_tape_label_yul_validate_sequence(const uint8_t *header_buf) {
    if (!header_buf) return -1;

    // Check VOL1 magic bytes
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 0), "VOL1", 4) != 0) {
        return -2;
    }

    // Check HDR1 magic bytes
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 0), "HDR1", 4) != 0) {
        return -3;
    }

    // Check HDR2 magic bytes
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 0), "HDR2", 4) != 0) {
        return -4;
    }

    // Verify .dat.bin extension constraint in HDR1 file_id field (Rule 13)
    char file_id_str[18] = {0};
    memcpy(file_id_str, header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 1), 17);
    
    if (!strstr(file_id_str, ".dat.bin") && !strstr(file_id_str, ".DAT.BIN")) {
        return -5; // Must end in .dat.bin extension
    }

    return 0; // Sequence validated successfully
}

int tsfi_tape_label_yul_check_governance(const uint8_t *header_buf, uint8_t required_clearance) {
    int val_res = tsfi_tape_label_yul_validate_sequence(header_buf);
    if (val_res != 0) return val_res;

    // Read security classification level code from HDR1
    uint8_t sec_code = header_buf[yul_get_label_offset(TAPE_LABEL_HDR1, 2)] - '0';
    if (sec_code > 3) sec_code = 0;

    // Reject access if required clearance is lower than asset classification tag
    if (required_clearance < sec_code) {
        return -6; // Access Denied: Clearance Insufficient
    }

    // Check owner ID provenance
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 2), "AUNCIENT_ZMM01", 14) != 0) {
        return -7; // Access Denied: Provenance Hash Mismatch
    }

    return 0; // Governance Gatekeeper Approved
}

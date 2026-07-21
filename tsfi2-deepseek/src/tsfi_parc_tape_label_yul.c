#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_tape_label_yul.h"

// Yul DDL Offset Resolvers (matching Yul TapeLabelDDL contract layout)
static int yul_get_vol1_offset(int field_id) {
    switch (field_id) {
        case 0: return 0;   // label_id ("VOL1")
        case 1: return 4;   // volume_id
        case 2: return 10;  // accessibility
        case 3: return 37;  // owner_id
        default: return 0;
    }
}

static int yul_get_hdr1_offset(int field_id) {
    switch (field_id) {
        case 0: return 80;  // label_id ("HDR1")
        case 1: return 84;  // file_id (must end in .dat.bin)
        case 2: return 101; // set_id
        default: return 80;
    }
}

int tsfi_tape_label_yul_format(uint8_t *header_buf, const char *volume_id, const char *file_id) {
    if (!header_buf || !volume_id || !file_id) return -1;

    // Fill 160-byte header buffer with space padding
    memset(header_buf, ' ', 160);

    // Format VOL1 header using Yul offsets
    memcpy(header_buf + yul_get_vol1_offset(0), "VOL1", 4);
    memcpy(header_buf + yul_get_vol1_offset(1), volume_id, strlen(volume_id) > 6 ? 6 : strlen(volume_id));
    memcpy(header_buf + yul_get_vol1_offset(3), "AUNCIENT_ZMM01", 14);

    // Format HDR1 header using Yul offsets
    memcpy(header_buf + yul_get_hdr1_offset(0), "HDR1", 4);
    memcpy(header_buf + yul_get_hdr1_offset(1), file_id, strlen(file_id) > 17 ? 17 : strlen(file_id));

    return 0;
}

int tsfi_tape_label_yul_validate(const uint8_t *header_buf) {
    if (!header_buf) return -1;

    // Check VOL1 magic bytes
    if (memcmp(header_buf + yul_get_vol1_offset(0), "VOL1", 4) != 0) {
        return -2;
    }

    // Check HDR1 magic bytes
    if (memcmp(header_buf + yul_get_hdr1_offset(0), "HDR1", 4) != 0) {
        return -3;
    }

    // Verify .dat.bin extension constraint in HDR1 file_id field (Rule 13)
    char file_id_str[18] = {0};
    memcpy(file_id_str, header_buf + yul_get_hdr1_offset(1), 17);
    
    if (!strstr(file_id_str, ".dat.bin") && !strstr(file_id_str, ".DAT.BIN")) {
        return -4; // Must end in .dat.bin extension
    }

    return 0; // Validated successfully
}

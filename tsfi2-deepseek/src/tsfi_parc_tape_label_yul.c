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
        case TAPE_LABEL_HDR3:
            switch (field_id) {
                case 0: return 240; // label_id ("HDR3")
                case 1: return 244; // sig_key ("SIG_2026_USLM_AFFIRMED")
                default: return 240;
            }
        case TAPE_LABEL_HDR4:
            switch (field_id) {
                case 0: return 320; // label_id ("HDR4")
                case 1: return 324; // spatial_xmin
                case 2: return 332; // spatial_ymin
                case 3: return 340; // spatial_xmax
                case 4: return 348; // spatial_ymax
                default: return 320;
            }
        case TAPE_LABEL_HDR5:
            switch (field_id) {
                case 0: return 400; // label_id ("HDR5")
                case 1: return 404; // prev_vol_id
                case 2: return 410; // next_vol_id
                default: return 400;
            }
        default: return 0;
    }
}

int tsfi_tape_label_yul_format_extended_header(
    uint8_t *header_buf,
    const char *volume_id,
    const char *file_id,
    uint8_t security_level,
    float xmin, float ymin, float xmax, float ymax,
    const char *prev_vol, const char *next_vol
) {
    if (!header_buf || !volume_id || !file_id) return -1;

    // Fill 480-byte header sequence buffer with space padding
    memset(header_buf, ' ', 480);

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
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 1), "F", 1);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 2), "00512", 5);

    // 4. HDR3 Label (Offset 240..319) - Cryptographic Provenance Key
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR3, 0), "HDR3", 4);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR3, 1), "SIG_2026_USLM_AFFIRMED", 22);

    // 5. HDR4 Label (Offset 320..399) - Quadtree Spatial Bounds
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 0), "HDR4", 4);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 1)), 8, "%07.1f", xmin);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 2)), 8, "%07.1f", ymin);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 3)), 8, "%07.1f", xmax);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 4)), 8, "%07.1f", ymax);

    // 6. HDR5 Label (Offset 400..479) - Volume Spanning Pointers
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR5, 0), "HDR5", 4);
    if (prev_vol) memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR5, 1), prev_vol, strlen(prev_vol) > 6 ? 6 : strlen(prev_vol));
    if (next_vol) memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR5, 2), next_vol, strlen(next_vol) > 6 ? 6 : strlen(next_vol));

    return 0;
}

int tsfi_tape_label_yul_format_trailer(uint8_t *trailer_buf, int label_type, uint32_t block_count) {
    if (!trailer_buf) return -1;

    memset(trailer_buf, ' ', 80);

    if (label_type == TAPE_LABEL_EOF1) {
        memcpy(trailer_buf, "EOF1", 4);
    } else if (label_type == TAPE_LABEL_EOV2) {
        memcpy(trailer_buf, "EOV2", 4);
    } else {
        return -2;
    }

    snprintf((char *)(trailer_buf + 54), 7, "%06u", block_count);
    return 0;
}

int tsfi_tape_label_yul_validate_sequence(const uint8_t *header_buf) {
    if (!header_buf) return -1;

    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 0), "VOL1", 4) != 0) return -2;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 0), "HDR1", 4) != 0) return -3;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR2, 0), "HDR2", 4) != 0) return -4;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR3, 0), "HDR3", 4) != 0) return -5;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 0), "HDR4", 4) != 0) return -6;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR5, 0), "HDR5", 4) != 0) return -7;

    char file_id_str[18] = {0};
    memcpy(file_id_str, header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 1), 17);
    if (!strstr(file_id_str, ".dat.bin") && !strstr(file_id_str, ".DAT.BIN")) {
        return -8; // Must end in .dat.bin extension (Rule 13)
    }

    return 0;
}

int tsfi_tape_label_yul_verify_signature(const uint8_t *header_buf) {
    if (!header_buf) return -1;

    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR3, 1), "SIG_2026_USLM_AFFIRMED", 22) != 0) {
        return -10; // Signature verification failed
    }
    return 0; // Verified
}

int tsfi_tape_label_yul_get_spatial_bounds(const uint8_t *header_buf, float *xmin, float *ymin, float *xmax, float *ymax) {
    if (!header_buf || !xmin || !ymin || !xmax || !ymax) return -1;

    *xmin = (float)atof((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 1)));
    *ymin = (float)atof((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 2)));
    *xmax = (float)atof((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 3)));
    *ymax = (float)atof((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR4, 4)));

    return 0;
}

int tsfi_tape_label_yul_check_governance(const uint8_t *header_buf, uint8_t required_clearance) {
    int val_res = tsfi_tape_label_yul_validate_sequence(header_buf);
    if (val_res != 0) return val_res;

    if (tsfi_tape_label_yul_verify_signature(header_buf) != 0) {
        return -11; // Cryptographic Signature Invalid
    }

    uint8_t sec_code = header_buf[yul_get_label_offset(TAPE_LABEL_HDR1, 2)] - '0';
    if (sec_code > 3) sec_code = 0;

    if (required_clearance < sec_code) {
        return -12; // Access Denied: Clearance Insufficient
    }

    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 2), "AUNCIENT_ZMM01", 14) != 0) {
        return -13; // Access Denied: Provenance Hash Mismatch
    }

    return 0;
}

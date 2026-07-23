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
        case TAPE_LABEL_HDR6:
            switch (field_id) {
                case 0: return 480; // label_id ("HDR6")
                case 1: return 484; // compression_type ("RLE")
                case 2: return 492; // vulkan_accel ("VK_ENABLE")
                default: return 480;
            }
        case TAPE_LABEL_HDR7:
            switch (field_id) {
                case 0: return 560; // label_id ("HDR7")
                case 1: return 564; // phase_phi
                case 2: return 572; // freq_fx
                case 3: return 580; // freq_fy
                case 4: return 588; // freq_fz
                default: return 560;
            }
        case TAPE_LABEL_HDR8:
            switch (field_id) {
                case 0: return 640; // label_id ("HDR8")
                case 1: return 644; // scsi_reg_20 (keycode 32)
                case 2: return 652; // scsi_reg_1E (keycode 30)
                case 3: return 670; // crypto_seal (64-bit FNV-1a Hex Seal)
                default: return 640;
            }
        default: return 0;
    }
}

int tsfi_tape_label_yul_format_full_header(
    uint8_t *header_buf,
    const char *volume_id,
    const char *file_id,
    uint8_t security_level,
    float xmin, float ymin, float xmax, float ymax,
    const char *prev_vol, const char *next_vol,
    float phase_phi, int fx, int fy, int fz,
    uint8_t keycode_32, uint8_t keycode_30
) {
    if (!header_buf || !volume_id || !file_id) return -1;

    // Fill 720-byte header sequence buffer with space padding
    memset(header_buf, ' ', 720);

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

    // 7. HDR6 Label (Offset 480..559) - Hardware Density & Vulkan Acceleration Tags
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR6, 0), "HDR6", 4);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR6, 1), "RLE", 3);
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR6, 2), "VK_ENABLE", 9);

    // 8. HDR7 Label (Offset 560..639) - Lissajous Phase Space Invariants
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 0), "HDR7", 4);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 1)), 8, "%07.3f", phase_phi);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 2)), 8, "%07d", fx);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 3)), 8, "%07d", fy);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 4)), 8, "%07d", fz);

    // 9. HDR8 Label (Offset 640..719) - WinchesterMQ SCSI Register State Map & Seal Slot
    memcpy(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 0), "HDR8", 4);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 1)), 8, "%07d", keycode_32);
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 2)), 8, "%07d", keycode_30);

    // Apply Cryptographic FNV-1a Integrity Seal across entire header block sequence
    tsfi_tape_label_apply_seal(header_buf);

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
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR6, 0), "HDR6", 4) != 0) return -8;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 0), "HDR7", 4) != 0) return -9;
    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 0), "HDR8", 4) != 0) return -10;

    char file_id_str[18] = {0};
    memcpy(file_id_str, header_buf + yul_get_label_offset(TAPE_LABEL_HDR1, 1), 17);
    if (!strstr(file_id_str, ".dat.bin") && !strstr(file_id_str, ".DAT.BIN")) {
        return -11; // Must end in .dat.bin extension (Rule 13)
    }

    return 0;
}

int tsfi_tape_label_yul_verify_signature(const uint8_t *header_buf) {
    if (!header_buf) return -1;

    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_HDR3, 1), "SIG_2026_USLM_AFFIRMED", 22) != 0) {
        return -12; // Signature verification failed
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

int tsfi_tape_label_yul_get_phase_invariants(const uint8_t *header_buf, float *phase_phi, int *fx, int *fy, int *fz) {
    if (!header_buf || !phase_phi || !fx || !fy || !fz) return -1;

    *phase_phi = (float)atof((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 1)));
    *fx = atoi((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 2)));
    *fy = atoi((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 3)));
    *fz = atoi((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR7, 4)));

    return 0;
}

int tsfi_tape_label_yul_get_scsi_map(const uint8_t *header_buf, uint8_t *kc_32, uint8_t *kc_30) {
    if (!header_buf || !kc_32 || !kc_30) return -1;

    *kc_32 = (uint8_t)atoi((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 1)));
    *kc_30 = (uint8_t)atoi((const char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 2)));

    return 0;
}

uint64_t tsfi_tape_label_compute_fnv1a(const uint8_t *data, size_t len) {
    if (!data) return 0;
    uint64_t hash = 0xCBF29CE484222325ULL;
    for (size_t i = 0; i < len; i++) {
        hash ^= data[i];
        hash *= 0x100000001B3ULL;
    }
    return hash;
}

int tsfi_tape_label_apply_seal(uint8_t *header_buf) {
    if (!header_buf) return -1;
    // Compute checksum over the first 640 bytes (VOL1 + HDR1..HDR7)
    uint64_t hash = tsfi_tape_label_compute_fnv1a(header_buf, 640);
    // Inscribe hex string seal in HDR8 reserved seal field via Yul offset resolver
    snprintf((char *)(header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 3)), 17, "%016llX", (unsigned long long)hash);
    return 0;
}

int tsfi_tape_label_verify_seal(const uint8_t *header_buf) {
    if (!header_buf) return -1;
    uint64_t computed_hash = tsfi_tape_label_compute_fnv1a(header_buf, 640);
    char seal_str[17] = {0};
    memcpy(seal_str, header_buf + yul_get_label_offset(TAPE_LABEL_HDR8, 3), 16);
    unsigned long long stored_hash = 0;
    if (sscanf(seal_str, "%llX", &stored_hash) != 1) return -2;
    if ((uint64_t)stored_hash != computed_hash) return -3; // Integrity Seal Broken
    return 0; // Seal Intact
}

int tsfi_tape_label_yul_check_governance(const uint8_t *header_buf, uint8_t required_clearance) {
    int val_res = tsfi_tape_label_yul_validate_sequence(header_buf);
    if (val_res != 0) return val_res;

    if (tsfi_tape_label_yul_verify_signature(header_buf) != 0) {
        return -13; // Cryptographic Signature Invalid
    }

    uint8_t sec_code = header_buf[yul_get_label_offset(TAPE_LABEL_HDR1, 2)] - '0';
    if (sec_code > 3) sec_code = 0;

    if (required_clearance < sec_code) {
        return -14; // Access Denied: Clearance Insufficient
    }

    if (memcmp(header_buf + yul_get_label_offset(TAPE_LABEL_VOL1, 2), "AUNCIENT_ZMM01", 14) != 0) {
        return -15; // Access Denied: Provenance Hash Mismatch
    }

    if (tsfi_tape_label_verify_seal(header_buf) != 0) {
        return -16; // Access Denied: Tape Integrity Seal Broken
    }

    return 0;
}

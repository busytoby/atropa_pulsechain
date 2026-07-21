#ifndef TSFI_PARC_TAPE_LABEL_YUL_H
#define TSFI_PARC_TAPE_LABEL_YUL_H

#include <stdint.h>
#include <stddef.h>

#define TAPE_SECURITY_UNCLASSIFIED 0
#define TAPE_SECURITY_CONFIDENTIAL 1
#define TAPE_SECURITY_SECRET       2
#define TAPE_SECURITY_TOPSECRET    3

#define TAPE_LABEL_VOL1 1
#define TAPE_LABEL_HDR1 2
#define TAPE_LABEL_HDR2 3
#define TAPE_LABEL_HDR3 4
#define TAPE_LABEL_HDR4 5
#define TAPE_LABEL_HDR5 6
#define TAPE_LABEL_HDR6 7
#define TAPE_LABEL_HDR7 8
#define TAPE_LABEL_HDR8 9
#define TAPE_LABEL_EOF1 10
#define TAPE_LABEL_EOV2 11

/* Format a full 720-byte .dat.bin header sequence (VOL1 + HDR1..HDR8) */
int tsfi_tape_label_yul_format_full_header(
    uint8_t *header_buf,
    const char *volume_id,
    const char *file_id,
    uint8_t security_level,
    float xmin, float ymin, float xmax, float ymax,
    const char *prev_vol, const char *next_vol,
    float phase_phi, int fx, int fy, int fz,
    uint8_t keycode_32, uint8_t keycode_30
);

/* Format an 80-byte trailer label (EOF1 / EOV2) with block counts for data integrity check */
int tsfi_tape_label_yul_format_trailer(uint8_t *trailer_buf, int label_type, uint32_t block_count);

/* Validate a full 720-byte .dat.bin full header buffer sequence against Yul DDL rules */
int tsfi_tape_label_yul_validate_sequence(const uint8_t *header_buf);

/* Verify cryptographic signature header (HDR3) */
int tsfi_tape_label_yul_verify_signature(const uint8_t *header_buf);

/* Extract quadtree spatial bounds from HDR4 header block */
int tsfi_tape_label_yul_get_spatial_bounds(const uint8_t *header_buf, float *xmin, float *ymin, float *xmax, float *ymax);

/* Extract Lissajous phase invariants from HDR7 header block */
int tsfi_tape_label_yul_get_phase_invariants(const uint8_t *header_buf, float *phase_phi, int *fx, int *fy, int *fz);

/* Extract WinchesterMQ SCSI state map from HDR8 header block */
int tsfi_tape_label_yul_get_scsi_map(const uint8_t *header_buf, uint8_t *kc_32, uint8_t *kc_30);

/* Security Governance Gatekeeper check: Enforces classification clearance and owner provenance */
int tsfi_tape_label_yul_check_governance(const uint8_t *header_buf, uint8_t required_clearance);

#endif // TSFI_PARC_TAPE_LABEL_YUL_H

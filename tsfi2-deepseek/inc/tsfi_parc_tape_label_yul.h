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
#define TAPE_LABEL_EOF1 4
#define TAPE_LABEL_EOF2 5
#define TAPE_LABEL_EOV1 6

/* Initialize and format a complete 240-byte .dat.bin header sequence (VOL1 + HDR1 + HDR2) using Yul DDL rules */
int tsfi_tape_label_yul_format_header(uint8_t *header_buf, const char *volume_id, const char *file_id, uint8_t security_level);

/* Format an 80-byte trailer label (EOF1 / EOV1) with block counts for data integrity check */
int tsfi_tape_label_yul_format_trailer(uint8_t *trailer_buf, int label_type, uint32_t block_count);

/* Validate a full .dat.bin header buffer sequence against Yul DDL rules */
int tsfi_tape_label_yul_validate_sequence(const uint8_t *header_buf);

/* Security Governance Gatekeeper check: Enforces classification clearance and owner provenance */
int tsfi_tape_label_yul_check_governance(const uint8_t *header_buf, uint8_t required_clearance);

#endif // TSFI_PARC_TAPE_LABEL_YUL_H

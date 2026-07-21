#ifndef TSFI_PARC_TAPE_LABEL_YUL_H
#define TSFI_PARC_TAPE_LABEL_YUL_H

#include <stdint.h>
#include <stddef.h>

#define TAPE_SECURITY_UNCLASSIFIED 0
#define TAPE_SECURITY_CONFIDENTIAL 1
#define TAPE_SECURITY_SECRET       2
#define TAPE_SECURITY_TOPSECRET    3

/* Initialize and format a .dat.bin header buffer using compiled Yul DDL rules */
int tsfi_tape_label_yul_format(uint8_t *header_buf, const char *volume_id, const char *file_id, uint8_t security_level);

/* Validate a .dat.bin header buffer against Yul DDL rules (ensuring .dat.bin extension) */
int tsfi_tape_label_yul_validate(const uint8_t *header_buf);

/* Security Governance Gatekeeper check: Enforces classification clearance and owner provenance */
int tsfi_tape_label_yul_check_governance(const uint8_t *header_buf, uint8_t required_clearance);

#endif // TSFI_PARC_TAPE_LABEL_YUL_H

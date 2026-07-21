#ifndef TSFI_PARC_TAPE_LABEL_YUL_H
#define TSFI_PARC_TAPE_LABEL_YUL_H

#include <stdint.h>
#include <stddef.h>

/* Initialize and format a .dat.bin header buffer using compiled Yul DDL rules */
int tsfi_tape_label_yul_format(uint8_t *header_buf, const char *volume_id, const char *file_id);

/* Validate a .dat.bin header buffer against Yul DDL rules (ensuring .dat.bin extension) */
int tsfi_tape_label_yul_validate(const uint8_t *header_buf);

#endif // TSFI_PARC_TAPE_LABEL_YUL_H

#ifndef TSFI_SPERONI_CONWAY_COBOL_H
#define TSFI_SPERONI_CONWAY_COBOL_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t level_number;      // COBOL Level (01, 05, 10, etc.)
    char field_name[32];        // Field Name (e.g. VOL-HEADER, FILE-BODY)
    uint32_t byte_length;       // Byte Length
    uint32_t arithmetized_off;  // Speroni-Conway Compiled Byte Offset
} tsfi_speroni_conway_field_t;

typedef struct {
    uint32_t record_id;                              // Record ID
    tsfi_speroni_conway_field_t fields[16];          // Arithmetized Fields Buffer
    size_t field_count;                              // Field Count
    uint32_t total_record_bytes;                     // Total Record Length
    uint32_t evm_gas_units;                          // 210 Gas / Auncient Ether Units
    double fet_power_watts;                          // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];                          // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_conway_cobol_t;

/* Initialize Speroni-Conway COBOL Record Layout */
int tsfi_speroni_conway_cobol_init(
    uint32_t record_id,
    tsfi_speroni_conway_cobol_t *rec
);

/* Add field and compile Speroni-Conway arithmetized offset */
int tsfi_speroni_conway_cobol_add_field(
    tsfi_speroni_conway_cobol_t *rec,
    uint32_t level_number,
    const char *field_name,
    uint32_t byte_length
);

#endif // TSFI_SPERONI_CONWAY_COBOL_H

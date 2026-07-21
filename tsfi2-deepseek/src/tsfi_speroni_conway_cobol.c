#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_conway_cobol.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_conway_cobol_init(
    uint32_t record_id,
    tsfi_speroni_conway_cobol_t *rec
) {
    if (!rec) return -1;
    memset(rec, 0, sizeof(tsfi_speroni_conway_cobol_t));

    rec->record_id = record_id;
    rec->evm_gas_units = 210; // 210 Gas / Auncient Ether Units per record slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    rec->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(rec->tape_dat_bin, sizeof(rec->tape_dat_bin), "SPERONI_CONWAY_COBOL_%08X.DAT.BIN", record_id);
    return 0;
}

int tsfi_speroni_conway_cobol_add_field(
    tsfi_speroni_conway_cobol_t *rec,
    uint32_t level_number,
    const char *field_name,
    uint32_t byte_length
) {
    if (!rec || !field_name || rec->field_count >= 16) return -1;

    size_t idx = rec->field_count;
    rec->fields[idx].level_number = level_number;
    snprintf(rec->fields[idx].field_name, sizeof(rec->fields[idx].field_name), "%s", field_name);
    rec->fields[idx].byte_length = byte_length;
    rec->fields[idx].arithmetized_off = rec->total_record_bytes; // Speroni-Conway 1963 offset polynomial

    rec->total_record_bytes += byte_length;
    rec->field_count++;

    printf("[SPERONI-CONWAY COBOL] Rec: %u | Field: '%s' (Level %02u) | Len: %u | Arithmetized Offset: 0x%04X | Total Len: %u | Gas: %u | File: %s\n",
           rec->record_id, field_name, level_number, byte_length,
           rec->fields[idx].arithmetized_off, rec->total_record_bytes,
           rec->evm_gas_units, rec->tape_dat_bin);

    return 0; // Field added successfully
}

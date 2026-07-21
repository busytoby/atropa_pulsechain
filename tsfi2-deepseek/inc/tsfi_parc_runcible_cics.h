#ifndef TSFI_PARC_RUNCIBLE_CICS_H
#define TSFI_PARC_RUNCIBLE_CICS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define CICS_TRANS_ID_LENGTH        4
#define CICS_MAP_NAME_LENGTH        8
#define CICS_MAX_QUEUED_TRANSACTIONS 32
#define CICS_MAX_SCREEN_ROWS        24
#define CICS_MAX_SCREEN_COLS        80
#define CICS_TS_QUEUE_MAX_ITEMS     16

/* CICS Basic Mapping Support (BMS) Field Attribute Byte */
typedef struct {
    uint8_t row;
    uint8_t col;
    uint8_t length;
    uint8_t attribute_flags; // 0x01: Protected, 0x02: Modified, 0x04: High Intensity
    char label[32];
    char value[64];
} tsfi_cics_bms_field_t;

/* CICS Basic Mapping Support Screen Map Definition */
typedef struct {
    char map_name[CICS_MAP_NAME_LENGTH + 1];
    uint16_t field_count;
    tsfi_cics_bms_field_t fields[32];
    char screen_buffer[CICS_MAX_SCREEN_ROWS][CICS_MAX_SCREEN_COLS + 1];
} tsfi_cics_bms_map_t;

/* CICS Temporary Storage Queue Item */
typedef struct {
    uint32_t item_id;
    uint8_t payload[256];
    size_t payload_bytes;
} tsfi_cics_ts_item_t;

/* CICS Temporary Storage Queue (TSQ) */
typedef struct {
    char queue_name[16];
    uint32_t item_count;
    tsfi_cics_ts_item_t items[CICS_TS_QUEUE_MAX_ITEMS];
} tsfi_cics_ts_queue_t;

/* CICS Transaction Control Table Entry (PCT / Program Control Table) */
typedef struct {
    char transaction_identifier[8];
    char program_name[16];
    char dynamic_contract_address[64];
    uint32_t execution_count;
    bool is_active;
} tsfi_cics_pct_entry_t;

/* CICS Runcible TTY Control Engine State */
typedef struct {
    bool is_initialized;
    uint32_t active_task_number;
    char current_trans_id[8];
    char terminal_identifier[8];
    tsfi_cics_bms_map_t active_map;
    tsfi_cics_ts_queue_t ts_queues[8];
    uint32_t ts_queue_count;
    tsfi_cics_pct_entry_t pct[CICS_MAX_QUEUED_TRANSACTIONS];
    uint32_t pct_count;
} tsfi_cics_engine_t;

/* Initialize Runcible TTY CICS Engine */
int tsfi_cics_engine_initialize(tsfi_cics_engine_t *engine);

/* Register Transaction in Program Control Table (PCT) */
int tsfi_cics_register_transaction(tsfi_cics_engine_t *engine, const char *trans_id, const char *program_name, const char *dynamic_contract_address);

/* Execute EXEC CICS START TRANSID Transaction */
int tsfi_cics_exec_start_transid(tsfi_cics_engine_t *engine, const char *trans_id, const char *input_payload);

/* Execute EXEC CICS SEND MAP / RECEIVE MAP (BMS Screen Processing) */
int tsfi_cics_exec_send_map(tsfi_cics_engine_t *engine, const char *map_name, const char *title);

/* Execute EXEC CICS WRITEQ TS / READQ TS (Temporary Storage Queues) */
int tsfi_cics_exec_writeq_ts(tsfi_cics_engine_t *engine, const char *queue_name, const void *data, size_t len, uint32_t *item_id_out);

/* Execute EXEC CICS RETURN (Pseudo-conversational termination / next TRANSID setup) */
int tsfi_cics_exec_return(tsfi_cics_engine_t *engine, const char *next_trans_id);

/* Render Current CICS Terminal Screen to ANSI / Runcible TTY Output */
int tsfi_cics_render_terminal_screen(const tsfi_cics_engine_t *engine, char *output_buffer, size_t buffer_max);

#endif // TSFI_PARC_RUNCIBLE_CICS_H

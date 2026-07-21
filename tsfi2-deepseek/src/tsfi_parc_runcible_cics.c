#include "tsfi_parc_runcible_cics.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int tsfi_cics_engine_initialize(tsfi_cics_engine_t *engine) {
    if (!engine) return -1;
    memset(engine, 0, sizeof(tsfi_cics_engine_t));

    engine->is_initialized = true;
    engine->active_task_number = 1001;
    snprintf(engine->terminal_identifier, sizeof(engine->terminal_identifier), "T001");
    snprintf(engine->current_trans_id, sizeof(engine->current_trans_id), "CESN");

    // Register default CICS transactions (CESN, CEMT, CECI, DISP)
    tsfi_cics_register_transaction(engine, "CESN", "DFHCSN", "dynamic_0x7343d8afa9d6e3376873ea24ccba7c7230aab14b");
    tsfi_cics_register_transaction(engine, "CEMT", "DFHEMT", "dynamic_0x1111111111111111111111111111111111111111");
    tsfi_cics_register_transaction(engine, "CECI", "DFHECI", "dynamic_0x2222222222222222222222222222222222222222");
    tsfi_cics_register_transaction(engine, "DISP", "RUNCICS", "dynamic_0x3333333333333333333333333333333333333333");

    return 0;
}

int tsfi_cics_register_transaction(tsfi_cics_engine_t *engine, const char *trans_id, const char *program_name, const char *dynamic_contract_address) {
    if (!engine || !trans_id || !program_name) return -1;
    if (engine->pct_count >= CICS_MAX_QUEUED_TRANSACTIONS) return -2;

    tsfi_cics_pct_entry_t *entry = &engine->pct[engine->pct_count++];
    snprintf(entry->transaction_identifier, sizeof(entry->transaction_identifier), "%s", trans_id);
    snprintf(entry->program_name, sizeof(entry->program_name), "%s", program_name);
    if (dynamic_contract_address) {
        snprintf(entry->dynamic_contract_address, sizeof(entry->dynamic_contract_address), "%s", dynamic_contract_address);
    } else {
        snprintf(entry->dynamic_contract_address, sizeof(entry->dynamic_contract_address), "dynamic_0x0000000000000000000000000000000000000000");
    }
    entry->execution_count = 0;
    entry->is_active = true;

    return 0;
}

int tsfi_cics_exec_start_transid(tsfi_cics_engine_t *engine, const char *trans_id, const char *input_payload) {
    if (!engine || !trans_id) return -1;

    for (uint32_t i = 0; i < engine->pct_count; i++) {
        if (strncmp(engine->pct[i].transaction_identifier, trans_id, CICS_TRANS_ID_LENGTH) == 0) {
            engine->pct[i].execution_count++;
            engine->active_task_number++;
            snprintf(engine->current_trans_id, sizeof(engine->current_trans_id), "%s", trans_id);

            // Execute BMS map display for transaction
            tsfi_cics_exec_send_map(engine, "MAP001", input_payload ? input_payload : "CICS TRANSACTION EXECUTION");
            return 0;
        }
    }
    return -404; // Transaction not found in PCT
}

int tsfi_cics_exec_send_map(tsfi_cics_engine_t *engine, const char *map_name, const char *title) {
    if (!engine || !map_name) return -1;

    tsfi_cics_bms_map_t *map = &engine->active_map;
    memset(map, 0, sizeof(tsfi_cics_bms_map_t));
    snprintf(map->map_name, sizeof(map->map_name), "%s", map_name);

    for (int r = 0; r < CICS_MAX_SCREEN_ROWS; r++) {
        memset(map->screen_buffer[r], ' ', CICS_MAX_SCREEN_COLS);
        map->screen_buffer[r][CICS_MAX_SCREEN_COLS] = '\0';
    }

    // Header line
    char header[CICS_MAX_SCREEN_COLS + 1];
    snprintf(header, sizeof(header), "CICS/TSFI RUNCIBLE TTY   TRANS: %-4s   TERM: %-4s   TASK: %06u",
             engine->current_trans_id, engine->terminal_identifier, engine->active_task_number);
    memcpy(map->screen_buffer[0], header, strlen(header));

    // Title line
    if (title) {
        snprintf(header, sizeof(header), "=== %s ===", title);
        memcpy(map->screen_buffer[2], header, strlen(header));
    }

    // Standard CICS Command Line at Row 23
    snprintf(header, sizeof(header), "COMMAND ===> %-4s                                      CLEAR=EXIT F3=END", engine->current_trans_id);
    memcpy(map->screen_buffer[23], header, strlen(header));

    return 0;
}

int tsfi_cics_exec_writeq_ts(tsfi_cics_engine_t *engine, const char *queue_name, const void *data, size_t len, uint32_t *item_id_out) {
    if (!engine || !queue_name || !data || len == 0) return -1;

    tsfi_cics_ts_queue_t *queue = NULL;
    for (uint32_t i = 0; i < engine->ts_queue_count; i++) {
        if (strncmp(engine->ts_queues[i].queue_name, queue_name, 8) == 0) {
            queue = &engine->ts_queues[i];
            break;
        }
    }

    if (!queue) {
        if (engine->ts_queue_count >= 8) return -2;
        queue = &engine->ts_queues[engine->ts_queue_count++];
        snprintf(queue->queue_name, sizeof(queue->queue_name), "%s", queue_name);
        queue->item_count = 0;
    }

    if (queue->item_count >= CICS_TS_QUEUE_MAX_ITEMS) return -3;

    tsfi_cics_ts_item_t *item = &queue->items[queue->item_count++];
    item->item_id = queue->item_count;
    size_t copy_bytes = (len > 256) ? 256 : len;
    memcpy(item->payload, data, copy_bytes);
    item->payload_bytes = copy_bytes;

    if (item_id_out) *item_id_out = item->item_id;
    return 0;
}

int tsfi_cics_exec_return(tsfi_cics_engine_t *engine, const char *next_trans_id) {
    if (!engine) return -1;
    if (next_trans_id) {
        snprintf(engine->current_trans_id, sizeof(engine->current_trans_id), "%s", next_trans_id);
    } else {
        snprintf(engine->current_trans_id, sizeof(engine->current_trans_id), "CESN");
    }
    return 0;
}

int tsfi_cics_render_terminal_screen(const tsfi_cics_engine_t *engine, char *output_buffer, size_t buffer_max) {
    if (!engine || !output_buffer || buffer_max < 2048) return -1;

    size_t written = 0;
    written += snprintf(output_buffer + written, buffer_max - written, "\033[2J\033[H"); // Clear screen & reset cursor
    for (int r = 0; r < CICS_MAX_SCREEN_ROWS; r++) {
        written += snprintf(output_buffer + written, buffer_max - written, "%.80s\n", engine->active_map.screen_buffer[r]);
    }
    return 0;
}

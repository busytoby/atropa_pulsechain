#include "tsfi_parc_runcible_cics.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

int main(void) {
    printf("=======================================================\n");
    printf(" RUNNING RUNCIBLE TTY CICS TRANSACTION ENGINE TEST SUITE\n");
    printf("=======================================================\n");

    tsfi_cics_engine_t engine;
    assert(tsfi_cics_engine_initialize(&engine) == 0);
    assert(engine.is_initialized == true);
    assert(strcmp(engine.terminal_identifier, "T001") == 0);
    printf("[PASS] CICS Engine Initialization verified\n");

    // Test PCT Transaction Execution
    assert(tsfi_cics_exec_start_transid(&engine, "CESN", "WELCOME TO TSFI RUNCIBLE TTY CICS") == 0);
    assert(engine.active_task_number == 1002);
    printf("[PASS] EXEC CICS START TRANSID 'CESN' verified (Task #1002)\n");

    // Test Temporary Storage Queue Write (WRITEQ TS)
    uint32_t item_id = 0;
    const char *payload = "PAYLOAD_DATA_RECORD_001";
    assert(tsfi_cics_exec_writeq_ts(&engine, "TSQ001", payload, strlen(payload), &item_id) == 0);
    assert(item_id == 1);
    printf("[PASS] EXEC CICS WRITEQ TS queue 'TSQ001' item #1 verified\n");

    // Test Screen Buffer Rendering
    char screen[4096];
    assert(tsfi_cics_render_terminal_screen(&engine, screen, sizeof(screen)) == 0);
    assert(strstr(screen, "CICS/TSFI RUNCIBLE TTY") != NULL);
    assert(strstr(screen, "CESN") != NULL);
    printf("[PASS] CICS BMS 3270 Terminal Screen Rendering verified\n");

    // Test EXEC CICS RETURN
    assert(tsfi_cics_exec_return(&engine, "CEMT") == 0);
    assert(strcmp(engine.current_trans_id, "CEMT") == 0);
    printf("[PASS] EXEC CICS RETURN next TRANSID 'CEMT' verified\n");

    printf("=======================================================\n");
    printf(" ALL RUNCIBLE TTY CICS ENGINE TESTS PASSED (100%%)      \n");
    printf("=======================================================\n");

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "tsfi_cli.h"
#include "auncient_sdk.h"

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT INTEGRATED CLI TEST\n");
    printf("=============================================================\n");

    // Initialize WaveSystem context
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "FATAL: System creation failed\n");
        return 1;
    }

    // 1. Speak a pre-loaded word immediately (SPK_WRITE_LEDGER, ID 2)
    printf("[TEST] Speaking pre-loaded SPK_WRITE_LEDGER directly...\n");
    char cmd_pre_loaded[128] = "SPEAK 350.0 700.0 0.2 2";
    int status = tsfi_cli_process_line(ws, cmd_pre_loaded);
    assert(status == 0);

    // 2. Register and speak a new custom dynamic word (SPK_CUSTOM_CMD, ID 4)
    printf("[TEST] Dispatching WORD compile directive for SPK_CUSTOM_CMD...\n");
    char cmd_word[128] = "WORD SPK_CUSTOM_CMD 4 500.0 1000.0 0.3 00 02";
    status = tsfi_cli_process_line(ws, cmd_word);
    assert(status == 0);

    printf("[TEST] Dispatching SPEAK command wave for SPK_CUSTOM_CMD...\n");
    char cmd_speak[128] = "SPEAK 500.0 1000.0 0.3 4";
    status = tsfi_cli_process_line(ws, cmd_speak);
    assert(status == 0);


    printf("   ✓ Integrated CLI commands executed and verified successfully.\n");
    printf("=============================================================\n");
    printf("INTEGRATED CLI TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}

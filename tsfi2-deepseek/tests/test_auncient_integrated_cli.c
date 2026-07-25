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

    // 1. Programmatically call CLI parser to compile and register the Word
    printf("[TEST] Dispatching WORD compile directive directly to the tsfi_cli...\n");
    char cmd_word[128] = "WORD SPK_LOCK_SCSI 1 440.0 880.0 0.4 10 02";
    int status = tsfi_cli_process_line(ws, cmd_word);
    assert(status == 0);

    // 2. Programmatically call CLI parser to SPEAK/execute the Word
    printf("[TEST] Dispatching SPEAK command wave directly to the tsfi_cli...\n");
    char cmd_speak[128] = "SPEAK 440.0 880.0 0.4 1";
    status = tsfi_cli_process_line(ws, cmd_speak);
    assert(status == 0);

    printf("   ✓ Integrated CLI commands executed and verified successfully.\n");
    printf("=============================================================\n");
    printf("INTEGRATED CLI TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}

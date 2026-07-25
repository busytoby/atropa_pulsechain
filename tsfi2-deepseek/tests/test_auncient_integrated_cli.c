#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

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

    // 3. Register a short-lived command and verify temporal expiration (Wheeler Jump Replay Gating)
    printf("[TEST] Dispatching short-lived SPK_EXPIRED_CMD...\n");
    char cmd_expired_word[128] = "WORD SPK_EXPIRED_CMD 5 400.0 800.0 0.1 00 02";
    status = tsfi_cli_process_line(ws, cmd_expired_word);
    assert(status == 0);

    struct timespec slp = { .tv_sec = 0, .tv_nsec = 150000000 };
    nanosleep(&slp, NULL); // 150ms > 100ms decay limit


    printf("[TEST] Speaking expired SPK_EXPIRED_CMD (should fail)...\n");
    char cmd_expired_speak[128] = "SPEAK 400.0 800.0 0.1 5";
    status = tsfi_cli_process_line(ws, cmd_expired_speak);
    assert(status == 1); // Confirms the CLI rejected the expired command wave
    // 4. Test altering words in memory using COBOL strategy
    printf("[TEST] Dispatching COBOL_ALTER command to modify SPK_CUSTOM_CMD WMQ byte in memory...\n");
    char cmd_alter[128] = "COBOL_ALTER 3 WORD-WMQ 32"; // Index 3 is SPK_CUSTOM_CMD (Preloaded 0, 1, 2, then Custom at 3)
    status = tsfi_cli_process_line(ws, cmd_alter);
    assert(status == 0);

    printf("[TEST] Speaking SPK_CUSTOM_CMD again to confirm memory alteration (should now show WMQ: 0x20)...\n");
    status = tsfi_cli_process_line(ws, cmd_speak);
    assert(status == 0);
    // 5. Test pre-dispatch strategy validation
    printf("[TEST] Dispatching WORD compile directive for strategically invalid SPK_INVALID_CMD...\n");
    char cmd_invalid_word[128] = "WORD SPK_INVALID_CMD 6 500.0 500.0 0.3 00 02"; // f1 == f2 is invalid
    status = tsfi_cli_process_line(ws, cmd_invalid_word);
    assert(status == 0);

    printf("[TEST] Speaking invalid SPK_INVALID_CMD (should be rejected by strategy rules)...\n");
    char cmd_invalid_speak[128] = "SPEAK 500.0 500.0 0.3 6";
    status = tsfi_cli_process_line(ws, cmd_invalid_speak);
    assert(status == 1); // Confirms pre-dispatch strategy check blocked execution



    printf("   ✓ Integrated CLI commands executed and verified successfully.\n");
    printf("=============================================================\n");
    printf("INTEGRATED CLI TESTS COMPLETED\n");
    printf("=============================================================\n");

    return 0;
}

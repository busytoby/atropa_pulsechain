#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include "tsfi_resonance.h"
#include "lau_memory.h"
#include "tsfi_wiring.h"
#include "tsfi_logic.h"
#include "tsfi_types.h"
#include "tsfi_trace.h"
#include "tsfi_raw.h"

int main() {
    printf("=== TSFi Wire Thread Orchestration Unit Test ===\n");

    // Initialize System
    WaveSystem *ws = tsfi_create_system();
    assert(ws != NULL);

    // Initial Provenance Check (Loads Plugins and sets g_ws)
    if (ws->provenance) ws->provenance();

    // 1. Start Session via Directive
    printf("[TEST 1] Start Session via Directive... \n");
    if (ws->current_directive) lau_free(ws->current_directive);
    ws->current_directive = lau_strdup("START_SESSION 1 /bin/sh");
    
    // Process directive
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;
    assert(ws->active_sessions[1] != NULL);
    printf("PASS\n");

    // Give child a moment to start
    tsfi_raw_usleep(100000);

    // 2. Inject Data via Directive
    printf("[TEST 2] Inject Data via Directive... \n");
    if (ws->current_directive) lau_free(ws->current_directive);
    ws->current_directive = lau_strdup("ECHO_SESSION 1 echo 'VERIFY_ECHO_DATA'");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;
    
    // Pump epoch to read output
    int loops = 0;
    while (strstr(ws->shared_pty_buffer, "VERIFY_ECHO_DATA") == NULL && loops++ < 100) {
        HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
        tsfi_raw_usleep(10000);
    }
    
    assert(strstr(ws->shared_pty_buffer, "VERIFY_ECHO_DATA") != NULL);
    printf("PASS (Buffer: %s)\n", ws->shared_pty_buffer);

    // 3. Stop Session via Directive
    printf("[TEST 3] Stop Session via Directive... \n");
    if (ws->current_directive) lau_free(ws->current_directive);
    ws->current_directive = lau_strdup("STOP_SESSION 1");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;

    // Pump to process termination and reap
    loops = 0;
    while (ws->active_sessions[1] != NULL && loops++ < 1000) {
        HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
        tsfi_raw_usleep(10000);
    }

    assert(ws->active_sessions[1] == NULL);
    printf("PASS\n");

    // 4. Multiple Concurrent Sessions
    printf("[TEST 4] Multiple Concurrent Sessions... \n");
    ws->current_directive = lau_strdup("START_SESSION 2 /bin/sh");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);

    ws->current_directive = lau_strdup("START_SESSION 3 /bin/sh");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;

    assert(ws->active_sessions[2] != NULL);
    assert(ws->active_sessions[3] != NULL);
    printf("PASS (Started sessions 2 and 3)\n");

    tsfi_raw_usleep(100000);

    // Echo into session 2
    ws->current_directive = lau_strdup("ECHO_SESSION 2 echo 'DATA_S2'");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;

    loops = 0;
    while (strstr(ws->shared_pty_buffer, "DATA_S2") == NULL && loops++ < 100) {
        HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
        tsfi_raw_usleep(10000);
    }
    assert(strstr(ws->shared_pty_buffer, "DATA_S2") != NULL);

    // Echo into session 3
    memset(ws->shared_pty_buffer, 0, sizeof(ws->shared_pty_buffer));
    ws->current_directive = lau_strdup("ECHO_SESSION 3 echo 'DATA_S3'");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;

    loops = 0;
    while (strstr(ws->shared_pty_buffer, "DATA_S3") == NULL && loops++ < 100) {
        HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
        tsfi_raw_usleep(10000);
    }
    assert(strstr(ws->shared_pty_buffer, "DATA_S3") != NULL);
    printf("PASS (Data verified from both sessions)\n");

    // 5. Invalid Session ID (Out of bounds / non-existent)
    printf("[TEST 5] Invalid Session ID Handling... \n");
    ws->current_directive = lau_strdup("START_SESSION 5 /bin/sh"); // Out of bounds
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);

    ws->current_directive = lau_strdup("ECHO_SESSION 0 echo 'NOBODY_HOME'"); // Not started
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;
    printf("PASS (No crashes on invalid inputs)\n");

    // Cleanup 2 & 3
    ws->current_directive = lau_strdup("STOP_SESSION 2");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = lau_strdup("STOP_SESSION 3");
    HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
    lau_free(ws->current_directive);
    ws->current_directive = NULL;

    loops = 0;
    while ((ws->active_sessions[2] != NULL || ws->active_sessions[3] != NULL) && loops++ < 1000) {
        HELMHOLTZ_RESONANCE_LIST(STEP, ws, 1.0);
        tsfi_raw_usleep(10000);
    }
    assert(ws->active_sessions[2] == NULL);
    assert(ws->active_sessions[3] == NULL);

    printf("=== All Orchestration Tests Passed ===\n");
    lau_final_cleanup(ws, -1);
    _exit(0);


    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
}

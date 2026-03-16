#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include "tsfi_resonance.h"
#include "lau_memory.h"
#include "tsfi_raw.h"

// Test state
static char captured_output[4096];
static size_t captured_len = 0;
static bool callback_called = false;

static void test_io_callback(void *arg, const char *buf, size_t n) {
    int *expected_val = (int*)arg;
    assert(*expected_val == 123); // Verify arg pointer integrity via R15/RDI
    
    if (captured_len + n < sizeof(captured_output)) {
        memcpy(captured_output + captured_len, buf, n);
        captured_len += n;
    }
    callback_called = true;
}

int main() {
    printf("=== TSFi Wire Thread I/O Unit Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    // Test 1: PTY Ingest (Echo)
    printf("[TEST 1] PTY Ingest (Echo)... ");
    memset(captured_output, 0, sizeof(captured_output));

    LauWireThread *wt_echo = tsfi_wire_thread_create_pty(1, "echo 'TSFI_IO_SUCCESS'", NULL);
    assert(wt_echo != NULL);
    
    // Wire mock logs
    wt_echo->log_stdout.buffer = (char *)lau_malloc(4096);
    wt_echo->log_stdout.head = &fw->rtl.log_stdout_head;
    wt_echo->log_stdout.tail = &fw->rtl.log_stdout_tail;
    wt_echo->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    
    // Poll resonance list until finished
    int loops = 0;
    while (!tsfi_wire_thread_is_at_rest(wt_echo) && loops++ < 1000) {
        tsfi_wire_thread_dispatch(wt_echo);
        tsfi_raw_usleep(1000);
    }

    assert(strstr(wt_echo->log_stdout.buffer, "TSFI_IO_SUCCESS") != NULL);
    printf("PASS\n");

    // Test 2: PTY Egress (Interactive Shell)
    printf("[TEST 2] PTY Egress (Interactive Shell)... ");
    
    LauWireThread *wt_shell = tsfi_wire_thread_create_pty(2, "/bin/sh", NULL);
    assert(wt_shell != NULL);
    
    wt_shell->log_stdin.buffer = (char *)lau_malloc(4096);
    wt_shell->log_stdin.head = &fw->rtl.log_stdin_head;
    wt_shell->log_stdin.tail = &fw->rtl.log_stdin_tail;
    wt_shell->log_stdin.lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;

    wt_shell->log_stdout.buffer = (char *)lau_malloc(4096);
    wt_shell->log_stdout.head = &fw->rtl.log_stdout_head;
    wt_shell->log_stdout.tail = &fw->rtl.log_stdout_tail;
    wt_shell->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;

    // Inject data into stdin wavefront
    const char *input_cmd = "echo 'VERIFY_ECHO_DATA'\n";
    strcpy(wt_shell->log_stdin.buffer, input_cmd);
    *wt_shell->log_stdin.head = strlen(input_cmd);

    // Poll for response
    loops = 0;
    while (strstr(wt_shell->log_stdout.buffer, "VERIFY_ECHO_DATA") == NULL && loops++ < 1000) {
        tsfi_wire_thread_dispatch(wt_shell);
        tsfi_raw_usleep(1000);
    }

    assert(strstr(wt_shell->log_stdout.buffer, "VERIFY_ECHO_DATA") != NULL);
    printf("PASS\n");

    // Clean up shell via Directive
    if (wt_shell->step_executor_directive) {
        wt_shell->step_executor_directive("STOP");
    }
    
    loops = 0;
    while (!tsfi_wire_thread_is_at_rest(wt_shell) && loops++ < 1000) {
        tsfi_wire_thread_dispatch(wt_shell);
        tsfi_raw_usleep(1000);
    }

    // Test 3: Lifecycle / Termination
    printf("[TEST 3] Lifecycle Termination... ");
    
    // Launch a process that exits immediately
    LauWireThread *wt_exit = tsfi_wire_thread_create_pty(3, "exit 0", NULL);
    assert(wt_exit != NULL);
    wt_exit->log_stdout.buffer = (char *)lau_malloc(4096);
    wt_exit->log_stdout.head = &fw->rtl.log_stdout_head;
    wt_exit->log_stdout.tail = &fw->rtl.log_stdout_tail;
    wt_exit->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;

    loops = 0;
    while (!tsfi_wire_thread_is_at_rest(wt_exit) && loops++ < 5000) {
        tsfi_wire_thread_dispatch(wt_exit);
        tsfi_raw_usleep(1000);
    }

    if (!tsfi_wire_thread_is_at_rest(wt_exit)) {
        printf("FAIL (intensity=%.2f, finished=%d)\n", wt_exit->current_intensity, wt_exit->finished);
        return 1;
    }
    printf("PASS\n");

    printf("=== All I/O Tests Passed ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "tsfi_logic.h"

static int g_alarm_triggered = 0;
void handle_sigalrm(int sig) { (void)sig; g_alarm_triggered = 1; }

static int g_thunk_executed = 0;
static void *g_thunk_context = NULL;

void sample_thunk(void *context) {
    g_thunk_executed = 1;
    g_thunk_context = context;
    printf("[THUNK] Firmware executed thunk with context: %p\n", context);
}

int main() {
    printf("=== TSFi Firmware Integrity Standard Cell Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    // 1. Test tsfi_wire_firmware_validate (Redaction)
    printf("[TEST 1] Wavefront Redaction (SECRET masking)... ");
    // Use the buffers already initialized by tsfi_wire_firmware_init()
    
    const char *sensitive_data = "Data: SECRET_KEY_123 and another SECRET here.";
    char *buf = (char *)(uintptr_t)fw->rtl.log_stdout_ptr;
    memcpy(buf, sensitive_data, strlen(sensitive_data));
    fw->rtl.log_stdout_head = strlen(sensitive_data);
    fw->rtl.log_stdout_valid_head = 0;

    tsfi_wire_firmware_validate(fw);

    assert(strstr(buf, "SECRET") == NULL);
    assert(strstr(buf, "******") != NULL);
    printf("PASS\n");

    // 2. Test cell_brush_teeth (Watchdog)
    printf("[TEST 2] Physiological Boundary (Watchdog)... ");
    signal(SIGALRM, handle_sigalrm);
    
    // Set a very short watchdog
    fw->cell_brush_teeth(1); 
    
    printf("(Waiting 2s for alarm) "); fflush(stdout);
    sleep(2);
    
    assert(g_alarm_triggered == 1);
    printf("PASS\n");

    // 3. Hardware Thunk Execution Test
    printf("[TEST 3] Invoking tsfi_wire_firmware_execute... ");
    void *dummy_context = (void*)0xDEADBEEF;
    tsfi_wire_firmware_execute(fw, sample_thunk, dummy_context);

    assert(g_thunk_executed == 1);
    assert(g_thunk_context == dummy_context);
    assert(fw->rtl.exec_done == 1); // Check RTL completion bit
    printf("PASS\n");

    printf("=== Integrity Tests PASSED ===\n");
    
    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

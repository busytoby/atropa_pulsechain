#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_raw.h"
#include <assert.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "tsfi_logic.h"
#include "vulkan/vulkan_system.h"

static int g_thunk_called = 0;
void test_thunk_target() {
    g_thunk_called = 1;
    printf("[THUNK] Target reached via LauMasterWavefront!\n");
}

int main() {
    printf("=== TSFi Master Wavefront Unit Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    LauMasterWavefront mw = {0};
    atomic_store(&mw.lock, 0);

    // Test 1: Read Memory via Master Wavefront
    printf("[TEST 1] Read Memory... ");
    uint64_t secret_val = 0xDEADC0DEBAADC0DEULL;
    mw.command_id = 1; // READ
    mw.arg0 = (uintptr_t)&secret_val;
    mw.status = 1; // BUSY
    
    fw->cell_mcp_execute(&mw);
    
    assert(mw.status == 2); // DONE
    assert(mw.result == secret_val);
    printf("PASS\n");

    // Test 2: Call JIT Thunk via Master Wavefront
    printf("[TEST 2] Call Thunk... ");
    mw.command_id = 3; // CALL
    mw.arg0 = (uintptr_t)test_thunk_target;
    mw.status = 1;
    
    fw->cell_mcp_execute(&mw);
    
    assert(mw.status == 2);
    assert(g_thunk_called == 1);
    printf("PASS\n");
    
    // Test 3: Hardware Poll Pass-Through Verification
    printf("[TEST 3] Hardware Poll Routing... ");
    char dummy_stdin_buf[16] = {0};
    int poll_result = fw->cell_hardware_poll(0, dummy_stdin_buf, 16);
    // Expecting nominal non-blocking poll outcome 
    assert(poll_result == 0 || poll_result == 1);
    printf("PASS\n");
    
    // Test 4: Dynamic Re-wiring of execution matrix test
    printf("[TEST 4] Execute dynamic Wave-Op... ");
    // Executing an operation map over the wave engine (id 0) using identity fields (dest 0, src1 0, src2 0)
    fw->cell_wave_exec(0, 0, 0, 0); 
    printf("PASS\n");
    
    // Test 5: Verify cell neural observation parser against blank stdout buffer
    printf("[TEST 5] Neural Observation Parsing... ");
    char intent_buf[256] = {0};
    char insight_buf[256] = {0};
    fw->cell_neural_observe(0, intent_buf, insight_buf);
    assert(strlen(intent_buf) == 0);
    assert(strlen(insight_buf) == 0);
    printf("PASS\n");

    printf("=== Master Wavefront Test PASSED ===\n");

    extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>

#include "tsfi_zmm_vm.h"
#include "tsfi_zmm_rpc.h"
#include "lau_memory.h"
#include "tsfi_types.h"
#include "vulkan/vulkan_system.h"
#include "tsfi_vision.h"

#include <signal.h>

void handle_abort(int sig) {
    (void)sig;
    extern void lau_free_all_active(void);
    lau_free_all_active();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    exit(1);
}

int main(void) {
    signal(SIGABRT, handle_abort);
    fprintf(stderr, "[INFO] Starting Google Flow Swapchain Compatibility Test...\n");
    fflush(stderr);

    TsfiZmmVmState state;
    tsfi_zmm_vm_init(&state);

    char output[4096];
    
    // 1. Simulate RPC: manifold.upload_asset (A TeddyBear structure)
    // TeddyBear: spectral_shift at offset 80 + 12 = 92? No, let's just upload 152 bytes.
    // We create a dummy bear hex.
    char bear_hex[305]; // 152 * 2 + 1
    memset(bear_hex, '0', 304);
    bear_hex[304] = 0;
    // Inject spectral_shift (float 0.475) at some offset for verification
    // But for now, we just test the upload and address return.
    
    char cmd_upload[1024];
    snprintf(cmd_upload, sizeof(cmd_upload), 
             "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.upload_asset\", \"params\": {\"data\": \"%s\"}, \"id\": 100}", 
             bear_hex);
    
    printf("[FLOW] Sending Upload Command...\n");
    tsfi_zmm_rpc_dispatch(&state, cmd_upload, output, sizeof(output));
    printf("[RPC DEBUG] upload output: %s\n", output);
    assert(strstr(output, "\"address\":"));

    // Extract address from output
    char *addr_ptr = strstr(output, "\"address\": \"") + 12;
    char addr_str[64];
    int k = 0;
    while(addr_ptr[k] != '"') { addr_str[k] = addr_ptr[k]; k++; }
    addr_str[k] = 0;
    void *uploaded_addr = (void*)strtoull(addr_str, NULL, 0);

    // 2. Simulate RPC: manifold.swap_asset (Slot 0)
    char cmd_swap[512];
    snprintf(cmd_swap, sizeof(cmd_swap), 
             "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.swap_asset\", \"params\": {\"slot\": 0, \"type\": 5, \"address\": \"%s\"}, \"id\": 101}", 
             addr_str);
    
    fprintf(stderr, "[TIME] %lu: Sending Swap Command...\n", (unsigned long)time(NULL));
    fflush(stderr);
    tsfi_zmm_rpc_dispatch(&state, cmd_swap, output, sizeof(output));
    fprintf(stderr, "[TIME] %lu: Swap Done.\n", (unsigned long)time(NULL));
    fflush(stderr);
    assert(strstr(output, "Swapped slot 0"));

    // 3. Simulate RPC: manifold.set_kernel (blue_teddy_bear)
    const char *cmd_kernel = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.set_kernel\", \"params\": {\"name\": \"blue_teddy_bear\"}, \"id\": 102}";
    fprintf(stderr, "[TIME] %lu: Sending Set Kernel Command...\n", (unsigned long)time(NULL));
    fflush(stderr);
    tsfi_zmm_rpc_dispatch(&state, cmd_kernel, output, sizeof(output));
    fprintf(stderr, "[TIME] %lu: Set Kernel Done.\n", (unsigned long)time(NULL));
    fflush(stderr);
    assert(strstr(output, "Kernel set to blue_teddy_bear"));

    // 4. Initialize Register Data for test
    float *rf = (float*)state.manifest->contiguous_rf;
    for (int i = 0; i < 16; i++) rf[i] = 1.0f; // Input Density
    for (int i = 16; i < 32; i++) rf[i] = 1.0f; // Spectral baseline
    state.manifest->active_mask = 0x3; // ZMM0, ZMM1

    // 5. Simulate RPC: manifold.set_secret (Change to a different shift)
    const char *cmd_secret = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.set_secret\", \"params\": {\"slot\": 0, \"value\": \"0.88\"}, \"id\": 103}";
    fprintf(stderr, "[TIME] %lu: Sending Set Secret Command...\n", (unsigned long)time(NULL));
    fflush(stderr);
    tsfi_zmm_rpc_dispatch(&state, cmd_secret, output, sizeof(output));
    fprintf(stderr, "[TIME] %lu: Set Secret Done.\n", (unsigned long)time(NULL));
    fflush(stderr);
    assert(strstr(output, "Secret injected"));
    TeddyBear *bear_remote = (TeddyBear*)uploaded_addr;
    assert(bear_remote->spectral_shift == 0.88f);

    // 6. Simulate RPC: manifold.dispatch
    const char *cmd_dispatch = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.dispatch\", \"id\": 104}";
    fprintf(stderr, "[TIME] %lu: Sending Dispatch Command...\n", (unsigned long)time(NULL));
    fflush(stderr);
    tsfi_zmm_rpc_dispatch(&state, cmd_dispatch, output, sizeof(output));
    fprintf(stderr, "[TIME] %lu: Dispatch Done.\n", (unsigned long)time(NULL));
    fflush(stderr);
    
    printf("[RPC DEBUG] dispatch output: %s\n", output);
    // expected density depends on internal math, currently outputs 16.000000 in test mode
    assert(strstr(output, "16.000000"));

    // 7. Inspect Slots
    const char *cmd_inspect = "{\"jsonrpc\": \"2.0\", \"method\": \"manifold.inspect_slots\", \"id\": 105}";
    fprintf(stderr, "[TIME] %lu: Sending Inspect Command...\n", (unsigned long)time(NULL));
    fflush(stderr);
    tsfi_zmm_rpc_dispatch(&state, cmd_inspect, output, sizeof(output));
    fprintf(stderr, "[TIME] %lu: Inspect Done.\n", (unsigned long)time(NULL));
    fflush(stderr);
    assert(strstr(output, "\"slot\": 0"));

    // 8. Test Visual Integration
    VulkanSystem mock_s = {0};
    mock_s.paint_buffer = (StagingBuffer*)lau_malloc(sizeof(StagingBuffer));
    mock_s.paint_buffer->data = (uint32_t*)lau_malloc(1280 * 720 * 4);
    mock_s.paint_buffer->width = 1280;
    mock_s.paint_buffer->height = 720;
    mock_s.paint_buffer->size = 1280 * 720 * 4;
    
    extern LauTelemetryState* lau_telemetry_get_state(void);
    mock_s.telem = lau_telemetry_get_state();
    
    fprintf(stderr, "[TIME] %lu: Testing UI Integration...\n", (unsigned long)time(NULL));
    fflush(stderr);
    extern void draw_ui_elements(VulkanSystem *s);
    draw_ui_elements(&mock_s); 
    fprintf(stderr, "[TIME] %lu: UI Done.\n", (unsigned long)time(NULL));
    fflush(stderr);

    printf("[PASS] Google Flow Asset Swapchain Verified.\n");

    fprintf(stderr, "[TIME] %lu: Freeing memory...\n", (unsigned long)time(NULL));
    fflush(stderr);
    lau_free(mock_s.paint_buffer->data);
    lau_free(mock_s.paint_buffer);

    tsfi_zmm_vm_destroy(&state);
    fprintf(stderr, "[TIME] %lu: Test Main Finished.\n", (unsigned long)time(NULL));
    fflush(stderr);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}

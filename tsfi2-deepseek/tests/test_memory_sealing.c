#include <unistd.h>
#define _POSIX_C_SOURCE 200809L
#include "tsfi_types.h"
#include "tsfi_wiring.h"
#include "lau_memory.h"
#include "tsfi_cli.h"
#include "tsfi_io.h"
#include "tsfi_wire_firmware.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
    alarm(5);
    printf("--- SYSTEM-11: MEMORY SEAL TEST ---\n");
    WaveSystem *ws = tsfi_create_system();
    if (!ws) {
        fprintf(stderr, "FATAL: System creation failed\n");
        return 1;
    }
    
    // 1. Basic Allocation & Sealing
    void *basic_ptr = lau_malloc(1024);
    assert(basic_ptr != NULL);
    lau_seal_object(basic_ptr);
    printf("[TEST] Basic Object Sealed.\n");
    
    // 2. Wired Allocation & Sealing
    void *wired_ptr = lau_malloc_wired(4096);
    assert(wired_ptr != NULL);
    lau_seal_object(wired_ptr);
    printf("[TEST] Wired Object Sealed.\n");

    // 3. GPU Allocation & Sealing
    void *gpu_ptr = lau_malloc_gpu(1024 * 1024, LAU_GPU_REBAR);
    if (gpu_ptr) {
        lau_seal_object(gpu_ptr);
        printf("[TEST] GPU Object Sealed.\n");
    }

    // Unseal before teardown
    lau_unseal_object(basic_ptr);
    lau_unseal_object(wired_ptr);
    if (gpu_ptr) lau_unseal_object(gpu_ptr);

    lau_free(basic_ptr);
    lau_free(wired_ptr);
    if (gpu_ptr) lau_free(gpu_ptr);

    lau_final_cleanup(ws, -1);
    
    printf("SYSTEM_AT_REST_SUCCESS\n");
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}

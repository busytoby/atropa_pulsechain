#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lau_memory.h"
#include "tsfi_io.h"

int main() {
    tsfi_io_log(stdout, TSFI_LOG_INFO, "LEAK_TEST", "Starting Leak Detection Verification");

    size_t initial_bytes = lau_get_active_bytes();
    size_t initial_count = lau_get_active_count();
    tsfi_io_log(stdout, TSFI_LOG_INFO, "LEAK_TEST", "Initial State: %zu bytes across %zu allocs", initial_bytes, initial_count);

    // 1. Perform Intentional Allocation
    const size_t alloc_size = 1024;
    void *ptr = lau_malloc(alloc_size);
    if (!ptr) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "LEAK_TEST", "lau_malloc failed");
        return 1;
    }
    
    size_t leaked_bytes = lau_get_active_bytes();
    size_t leaked_count = lau_get_active_count();
    tsfi_io_log(stdout, TSFI_LOG_INFO, "LEAK_TEST", "Leaked State: %zu bytes across %zu allocs", leaked_bytes, leaked_count);

    if (leaked_bytes <= initial_bytes) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "LEAK_TEST", "Error: Active bytes did not increase after allocation");
        return 1;
    }

    if (leaked_count != initial_count + 1) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "LEAK_TEST", "Error: Active count mismatch (Expected %zu, Found %zu)", initial_count + 1, leaked_count);
        return 1;
    }

    // 2. Perform Explicit Deallocation
    lau_free(ptr);
    
    size_t final_bytes = lau_get_active_bytes();
    size_t final_count = lau_get_active_count();
    tsfi_io_log(stdout, TSFI_LOG_INFO, "LEAK_TEST", "Final State: %zu bytes across %zu allocs", final_bytes, final_count);

    if (final_bytes != initial_bytes) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "LEAK_TEST", "Error: Active bytes not fully cleared (Expected %zu, Found %zu)", initial_bytes, final_bytes);
        return 1;
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "LEAK_TEST", "Test Passed!");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

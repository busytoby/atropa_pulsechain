#include <stdio.h>
#include <stdlib.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_io.h"
#include "tsfi_wire_firmware.h"

int main() {
    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Testing Memory Genesis Standard Cell");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    uint32_t initial_count = lau_registry_get_count();
    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Initial manifold count: %u", initial_count);

    const int num_allocs = 100;
    void *ptrs[num_allocs];

    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Performing %d allocations via Genesis cell...", num_allocs);
    for (int i = 0; i < num_allocs; i++) {
        ptrs[i] = lau_malloc(1024);
        if (!ptrs[i]) {
            tsfi_io_log(stdout, TSFI_LOG_FAIL, "GENESIS", "Error: Allocation %d failed", i);
            return 1;
        }
    }

    uint32_t final_count = lau_registry_get_count();
    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Final manifold count: %u", final_count);

    if (final_count != initial_count + num_allocs) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "GENESIS", "Error: Manifold count mismatch (Expected %u, Found %u)", initial_count + num_allocs, final_count);
        return 1;
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Verifying lookups for materialized objects...");
    for (int i = 0; i < num_allocs; i++) {
        LauMetadata *m = fw->cell_reg_lookup(ptrs[i]);
        if (!m) {
            tsfi_io_log(stdout, TSFI_LOG_FAIL, "GENESIS", "Error: Could not find object %d in registry", i);
            return 1;
        }
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Cleaning up...");
    for (int i = 0; i < num_allocs; i++) {
        lau_free(ptrs[i]);
    }

    uint32_t clean_count = lau_registry_get_count();
    if (clean_count != initial_count) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "GENESIS", "Error: Manifold not fully cleared (Found %u, Expected %u)", clean_count, initial_count);
        return 1;
    }

    tsfi_io_log(stdout, TSFI_LOG_INFO, "GENESIS", "Test Passed!");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

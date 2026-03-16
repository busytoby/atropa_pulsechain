#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    tsfi_io_printf(stdout, "=== TSFi Native IO: Masked Hardware Wildcard Validation ===\n");

    const char* log_stream = 
        "EVENT 0x01: SYSTEM_NORMAL_BOOT\n"
        "EVENT 0x02: FATAL_MEMORY_FAULT\n"
        "EVENT 0x03: DISK_OK\n"
        "EVENT 0x04: FATAL_SYSTEM_FAULT\n";

    size_t len = strlen(log_stream);
    
    const char* pattern = "FATAL_XXXXXX_FAULT";
    uint8_t ignore_mask[18] = {
        0, 0, 0, 0, 0, 0, 
        1, 1, 1, 1, 1, 1, 
        0, 0, 0, 0, 0, 0  
    };

    size_t match_indices[10];
    size_t match_count = tsfi_io_grep_masked((const uint8_t*)log_stream, len, pattern, ignore_mask, 18, match_indices, 10);

    tsfi_io_printf(stdout, "Found %zu matches using masked wildcard array.\n", match_count);

    if (match_count != 2) {
        tsfi_io_printf(stderr, "[FAIL] Expected 2 wildcard matches, got %zu.\n", match_count);
        return 1;
    }

    char extract1[19] = {0};
    char extract2[19] = {0};
    memcpy(extract1, log_stream + match_indices[0], 18);
    memcpy(extract2, log_stream + match_indices[1], 18);

    tsfi_io_printf(stdout, "  -> Match 1: %s\n", extract1);
    tsfi_io_printf(stdout, "  -> Match 2: %s\n", extract2);

    if (strcmp(extract1, "FATAL_MEMORY_FAULT") == 0 && strcmp(extract2, "FATAL_SYSTEM_FAULT") == 0) {
        tsfi_io_printf(stdout, "[SUCCESS] Masked wildcard evaluation is structurally robust.\n");
            extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
    } else {
        tsfi_io_printf(stderr, "[FAIL] Extracted values did not match expected variance.\n");
        return 1;
    }
}
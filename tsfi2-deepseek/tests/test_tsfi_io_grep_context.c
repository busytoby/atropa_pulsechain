#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    tsfi_io_printf(stdout, "=== TSFi Native IO: SIMD Context Boundary Extraction ===\n");
    
    const char* simulated_log = 
        "Line 1: System Boot\n"
        "Line 2: Allocating ReBAR pool...\n"
        "Line 3: WGP Matrix stable.\n"
        "Line 4: Initiating structural validation.\n"
        "Line 5: CRITICAL: Unaligned execution pointer detected.\n"
        "Line 6: Attempting to recover context...\n"
        "Line 7: Recovery failed. Suspending thread.\n"
        "Line 8: System transitioning to safe mode.\n"
        "Line 9: Safe mode active.\n";

    size_t log_size = strlen(simulated_log);
    
    // We want to grep for "CRITICAL:" with 2 lines before (-B 2) and 2 lines after (-A 2)
    const char* pattern = "CRITICAL:";
    size_t pat_len = strlen(pattern);
    
    size_t starts[10];
    size_t ends[10];
    
    size_t match_count = tsfi_io_grep_context((const uint8_t*)simulated_log, log_size, pattern, pat_len, starts, ends, 10, 2, 2);

    tsfi_io_printf(stdout, "Found %zu matches.\n", match_count);
    
    if (match_count != 1) {
        tsfi_io_printf(stderr, "[FAIL] Expected 1 match, got %zu.\n", match_count);
        return 1;
    }
    
    size_t context_len = ends[0] - starts[0];
    char* context_str = (char*)lau_malloc_wired(context_len + 1);
    memcpy(context_str, simulated_log + starts[0], context_len);
    context_str[context_len] = '\0';
    
    tsfi_io_printf(stdout, "--- Extracted Context Block ---\n%s\n-------------------------------\n", context_str);
    
    // Validate the bounds
    const char* expected_start = "Line 3: WGP Matrix stable.";
    const char* expected_end = "Recovery failed. Suspending thread.";
    
    if (strstr(context_str, expected_start) && strstr(context_str, expected_end)) {
        tsfi_io_printf(stdout, "[SUCCESS] Context bounds structurally verified.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Extracted context did not match expected structural bounds.\n");
        lau_free(context_str);
        return 1;
    }

    lau_free(context_str);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
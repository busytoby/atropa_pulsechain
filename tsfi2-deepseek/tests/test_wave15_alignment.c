#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_wave15.h"

// Helper to validate the capacity discovery logic
void validate_boundary(const char* label, int count, int boundary, bool should_fit) {
    size_t total_size = count * sizeof(WAVE15);
    bool fits = total_size <= (size_t)boundary;
    
    printf("[BOUNDARY] %s: %d WAVE15s (%zu bytes) vs %d-byte limit -> ", 
           label, count, total_size, boundary);
    
    if (fits == should_fit) {
        if (fits) {
            printf("ALIGNS WITHOUT WARNING [PASS]\n");
        } else {
            printf("DETECTED OVERFLOW [PASS]\n");
        }
    } else {
        printf("UNEXPECTED RESULT [FAIL]\n");
        exit(1);
    }
}

int main() {
    printf("=== Project Mariner: WAVE15 Boundary Discovery & Auto-Alignment Validation ===\n");

    // Verify WAVE15 size is exactly 15 bytes
    assert(sizeof(WAVE15) == 15);

    // 1) 2 WAVE15s will align without warning in 32-byte limited memory but 3 will not
    validate_boundary("32-byte limit", 2, 32, true);
    validate_boundary("32-byte limit", 3, 32, false);

    // 2) 4 WAVE15s will align without warning in 64-byte memory but 5 will not
    validate_boundary("64-byte limit", 4, 64, true);
    validate_boundary("64-byte limit", 5, 64, false);

    // 3) 8 WAVE15s will align without warning in 128-byte memory but 9 will not
    validate_boundary("128-byte limit", 8, 128, true);
    validate_boundary("128-byte limit", 9, 128, false);

    // 4) 17 WAVE15s will align without warning in 256-byte memory but 18 will not
    validate_boundary("256-byte limit", 17, 256, true);
    validate_boundary("256-byte limit", 18, 256, false);

    // 5) 34 WAVE15s will align without warning in 512-byte memory but 35 will not
    validate_boundary("512-byte limit", 34, 512, true);
    validate_boundary("512-byte limit", 35, 512, false);

    printf("\n[CONCLUSION] WAVE15 capacity discovery complete.\n");
    printf("WAVE(ANY-SIZE) can be supported by allocating in multiples of 15 bytes\n");
    printf("and padding to the next power-of-two boundary detected above.\n");
    printf("=== WAVE15 BOUNDARY VERIFICATION SUCCESSFUL ===\n");

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

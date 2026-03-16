#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "tsfi_merkle.h"
#include "tsfi_helmholtz.h"
#include "lau_memory.h"
#include "lau_registry.h"

int main() {
    printf("=== TSFi Ballet Step 0: Ouverture (Opening) ===\n");
    printf("[SUB-TEST] Ouverture: Discovery of Stable Rest\n");

    tsfi_helmholtz_init();
    setenv("TSFI_LEAK_TRACKING_IN_PROGRESS", "1", 1);

    uint32_t active = lau_registry_get_count();
    if (active > 0) {
        printf("[FRACTURE] Ouverture failed: System has %u active objects after init!\n", active);
        lau_report_memory_metrics();
        return 1;
    }

    printf("[SUCCESS] System at Stable Rest.\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_io.h"
#include "tsfi_wire_firmware.h"

const char* tier_to_str(uint8_t tier) {
    switch (tier) {
        case 1: return "CPU_CACHE (L3)";
        case 2: return "SYSTEM_RAM";
        case 3: return "GPU_REBAR";
        default: return "UNKNOWN";
    }
}

volatile float g_probe_dummy = 0.0f;

int main() {
    tsfi_io_log(stdout, TSFI_LOG_INFO, "TOPOGRAPHY", "Benchmarking Memory Tier Identification...");

    tsfi_wire_firmware_init();
    
    // 1. Basic Object (System RAM)
    printf("[DEBUG] bench: allocating RAM...\n");
    void * volatile ram_ptr = lau_malloc(1024);
    printf("[DEBUG] bench: calling mprotect on %p...\n", ram_ptr);
    uint32_t ram_lat = lau_mprotect(ram_ptr, PROT_READ); // Trigger probe
    LauMetadata *ram_meta = lau_registry_find(ram_ptr);
    
    if (ram_meta) {
        printf("\n[TIER: SYSTEM_RAM]\n");
        printf("Pointer:          %p\n", ram_ptr);
        printf("Measured Latency: %u cycles (Returned: %u)\n", ram_meta->probe_latency, ram_lat);
        printf("Identified Tier:  %s\n", tier_to_str(ram_meta->physical_tier));
        printf("Probe Side Effect: %f\n", g_probe_dummy);
    } else {
        printf("\n[ERROR] Could not find RAM metadata for %p\n", ram_ptr);
    }

    // 2. Wired Object (Simulation of device-like memory for categorization logic)
    printf("[DEBUG] bench: allocating Wired...\n");
    void * volatile wired_ptr = lau_malloc_wired(1024);
    printf("[DEBUG] bench: calling mprotect on %p...\n", wired_ptr);
    uint32_t wired_lat = lau_mprotect(wired_ptr, PROT_READ);
    LauMetadata *wired_meta = lau_registry_find(wired_ptr);

    if (wired_meta) {
        printf("\n[TIER: WIRED]\n");
        printf("Pointer:          %p\n", wired_ptr);
        printf("Measured Latency: %u cycles (Returned: %u)\n", wired_meta->probe_latency, wired_lat);
        printf("Identified Tier:  %s\n", tier_to_str(wired_meta->physical_tier));
        printf("Probe Side Effect: %f\n", g_probe_dummy);
    } else {
        printf("\n[ERROR] Could not find Wired metadata for %p\n", wired_ptr);
    }

    if (!ram_meta || ram_meta->physical_tier == 0) {
        tsfi_io_log(stdout, TSFI_LOG_FAIL, "TOPOGRAPHY", "Error: Failed to characterize RAM tier.");
        lau_free(ram_ptr); lau_free(wired_ptr);
        return 1;
    }

    lau_free(ram_ptr);
    lau_free(wired_ptr);

    tsfi_io_log(stdout, TSFI_LOG_INFO, "TOPOGRAPHY", "Test Passed!");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

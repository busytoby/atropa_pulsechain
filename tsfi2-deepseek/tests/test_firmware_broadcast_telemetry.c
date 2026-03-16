#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../thunks/tsfi_broadcast_telemetry_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Broadcast Telemetry Thunk Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    LauOccupancyManifest* manifest = (LauOccupancyManifest*)lau_malloc_wired(sizeof(LauOccupancyManifest));
    if (!manifest) return 1;
    memset(manifest, 0, sizeof(LauOccupancyManifest));

    for (int i = 0; i < 64; i++) {
        manifest->wgp_active_waves[i] = 4; 
    }

    for (int i = 0; i < 8; i++) {
        manifest->ace_pressure[i] = 8; 
    }

    regs.sgpr[10] = (uint32_t)((uintptr_t)manifest & 0xFFFFFFFF);
    regs.sgpr[11] = (uint32_t)((uintptr_t)manifest >> 32);

    tsfi_io_printf(stdout, "[EXEC] Triggering Zero-Copy RTL Telemetry Extraction...\n");
    
    tsfi_broadcast_telemetry_thunk(&regs);

    uint32_t success_flag = regs.sgpr[0];
    if (success_flag != 1) {
        tsfi_io_printf(stderr, "[FAIL] Telemetry Thunk returned hardware fault state. Flag: %u\n", success_flag);
        lau_free(manifest);
        return 1;
    }

    uint32_t cpu_load_scaled = regs.sgpr[1];
    uint32_t gpu_load_scaled = regs.sgpr[2];

    tsfi_io_printf(stdout, "[SUCCESS] Physical matrix natively ingested. Scaled CPU: %u, Scaled GPU: %u\n", cpu_load_scaled, gpu_load_scaled);

    if (cpu_load_scaled == 2500 && gpu_load_scaled == 5000) {
        tsfi_io_printf(stdout, "[VERIFIED] Thunk perfectly calculated 25.0%% WGP load and 50.0%% ACE load using direct zero-copy AVX-512 integration.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Topological metric alignment failed. Expected CPU: 2500, GPU: 5000.\n");
        lau_free(manifest);
        return 1;
    }

    lau_free(manifest);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
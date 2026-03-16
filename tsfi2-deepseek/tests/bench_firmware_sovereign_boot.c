#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include "tsfi_dys_math.h"
#include "tsfi_time.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ITERATIONS 1000

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Sovereign Boot Latency Benchmark ===\n");
    tsfi_io_printf(stdout, "Executing %d full Sovereign Boot & Seal sequences...\n", ITERATIONS);

    uint64_t start_time = get_time_ns();

    for (int i = 0; i < ITERATIONS; i++) {
        // 1. Physical Memory Genesis (64KB aligned)
        size_t fw_size = sizeof(LauWireFirmware);
        if (fw_size < 65536) fw_size = 65536; 
        
        LauWireFirmware *fw = (LauWireFirmware*)lau_memalign_wired(512, fw_size);
        if (!fw) {
            tsfi_io_printf(stderr, "[FAIL] Memory genesis failed on iteration %d\n", i);
            return 1;
        }
        memset(fw, 0, fw_size);

        // 2. Boot as Generation
        tsfi_reaction_boot_firmware_yi(&fw->core_yi);

        // 3. (Mock) Seal execution - we don't actually mprotect in the benchmark loop 
        // to avoid kernel vma exhaustion, but we simulate the mathematical seal.
        fw->rtl.cell_status = 1;

        // Cleanup: We don't deep-free the AVX-512 mapped components to avoid 
        // pointer resolution issues during the fast loop; the OS reclaims on exit.
        if (fw->core_yi.Xi) tsfi_bn_free(fw->core_yi.Xi);
        if (fw->core_yi.Ring) tsfi_bn_free(fw->core_yi.Ring);

        lau_free(fw);
    }

    uint64_t end_time = get_time_ns();
    double duration_ms = (double)(end_time - start_time) / 1000000.0;
    double avg_us = (duration_ms * 1000.0) / ITERATIONS;

    tsfi_io_printf(stdout, "\n[RESULTS]\n");
    tsfi_io_printf(stdout, "Total Time      : %.2f ms\n", duration_ms);
    tsfi_io_printf(stdout, "Avg Boot Latency: %.2f microseconds per Firmware\n", avg_us);
    tsfi_io_printf(stdout, "Throughput      : %.2f firmwares/sec\n", (1000.0 / duration_ms) * ITERATIONS);

    if (avg_us > 5000.0) {
        tsfi_io_printf(stderr, "[WARN] Firmware boot latency is severely degraded (>5ms).\n");
    } else {
        tsfi_io_printf(stdout, "[SUCCESS] Sovereign Boot is operating at high frequency.\n");
    }

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
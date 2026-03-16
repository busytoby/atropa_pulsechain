#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include "tsfi_dys_math.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <immintrin.h>
#include <time.h>

#define SHARED_SIZE (1024 * 1024)

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware Telemetry: ZMM Mirror IPC Validation ===\n");
    
    // Allocate a shared anonymous memory region that simulates the memfd / /dev/shm 
    // boundary between the firmware and the cockpit.
    void* shared_mem = mmap(NULL, SHARED_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared_mem == MAP_FAILED) {
        tsfi_io_printf(stderr, "[FAIL] mmap failed.\n");
        return 1;
    }
    
    LauWireFirmware* fw = (LauWireFirmware*)shared_mem;
    memset(fw, 0, sizeof(LauWireFirmware));

    TSFiBigInt* Prime = tsfi_bn_alloc();
    tsfi_bn_set_u64(Prime, 953467954114363ULL);

    pid_t pid = fork();

    if (pid == 0) {
        // --- CHILD PROCESS: The Firmware (Replacing tsfi_bridge) ---
        struct YI* root_yi = tsfi_reaction_shoot(Prime);
        
        TSFiBigInt* Pi = root_yi->Ring ? root_yi->Ring : root_yi->Xi;
        struct Dai* dai = tsfi_reaction_compute_reciprocity(root_yi->Psi, Pi, NULL);

        // ZMM Vector Drop (Telemetry Injection)
        // We drop the raw Ichidai and Daiichi directly into the WRF (Wavefront Register File)
        
        // We use WRF indices 120 and 121 as our designated telemetry block
        LauWireFirmware_State* rtl = &fw->rtl;
        
        // Use a generic memory copy or explicit cast to avoid strict aliasing
        _mm512_storeu_si512(&rtl->wrf[120], _mm512_loadu_si512((__m512i*)dai->Ichidai->limbs));
        _mm512_storeu_si512(&rtl->wrf[121], _mm512_loadu_si512((__m512i*)dai->Daiichi->limbs));
        
        // Set a strobe signal in the scalar registers
        rtl->cell_status = 0xFEEDBEEF;

        freeDAI(dai);
        freeYI(root_yi);
        exit(0);

    } else {
        // --- PARENT PROCESS: The Cockpit Reader ---
        int status;
        waitpid(pid, &status, 0);

        LauWireFirmware_State* mapped_rtl = &fw->rtl;

        if (mapped_rtl->cell_status != 0xFEEDBEEF) {
            tsfi_io_printf(stderr, "[FAIL] Signal strobe not detected in state!\n");
            return 1;
        }

        // Read the native 512-bit registers back out
        __m512i read_ichidai = _mm512_loadu_si512(&mapped_rtl->wrf[120]);
        __m512i read_daiichi = _mm512_loadu_si512(&mapped_rtl->wrf[121]);

        uint64_t ichi_val;
        uint64_t dai_val;
        
        // Extract the first 64-bit limb cleanly
        _mm512_storeu_si512((__m512i*)shared_mem, read_ichidai); // Temp buffer for extraction
        ichi_val = ((uint64_t*)shared_mem)[0];
        
        _mm512_storeu_si512((__m512i*)shared_mem, read_daiichi);
        dai_val = ((uint64_t*)shared_mem)[0];

        tsfi_io_printf(stdout, "[COCKPIT] Successfully intercepted ZMM Telemetry:\n");
        tsfi_io_printf(stdout, "  -> Ichidai (WRF 120): %lu\n", ichi_val);
        tsfi_io_printf(stdout, "  -> Daiichi (WRF 121): %lu\n", dai_val);

        if (ichi_val == 0 || dai_val == 0) {
            tsfi_io_printf(stderr, "[FAIL] Extracted values are zero. ZMM Mirroring failed.\n");
            return 1;
        }

        tsfi_io_printf(stdout, "[SUCCESS] Firmware seamlessly replaced bridge via direct AVX-512 memory mirror.\n");
    }

    tsfi_bn_free(Prime);
    munmap(shared_mem, SHARED_SIZE);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
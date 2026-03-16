#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../thunks/tsfi_vision_retrieval_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Vision Retrieval Edge Bounds Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    const char* log_start_edge = 
        "FATAL: Immediate failure on boot.\n"
        "[0.002] ReBAR matrix stable.\n"
        "[0.003] WGP online.\n"
        "[0.004] Neural observation active.\n"
        "[0.005] Safemode engaged.\n";

    regs.sgpr[10] = (uint32_t)((uintptr_t)log_start_edge & 0xFFFFFFFF);
    regs.sgpr[13] = (uint32_t)((uintptr_t)log_start_edge >> 32);
    regs.sgpr[11] = (uint32_t)strlen(log_start_edge);
    
    const char* target_pattern = "FATAL:";
    regs.sgpr[12] = (uint32_t)((uintptr_t)target_pattern & 0xFFFFFFFF);
    regs.sgpr[14] = (uint32_t)((uintptr_t)target_pattern >> 32);

    tsfi_vision_retrieval_thunk(&regs);

    if (regs.sgpr[0] != 1) {
        tsfi_io_printf(stderr, "[FAIL] Failed to retrieve on start edge.\n");
        return 1;
    }

    uintptr_t ptr1 = ((uintptr_t)regs.sgpr[2] << 32) | (uintptr_t)regs.sgpr[1];
    char* res1 = (char*)ptr1;
    
    if (strncmp(res1, "FATAL:", 6) == 0) {
        tsfi_io_printf(stdout, "[SUCCESS] Start edge boundary securely clamped to offset 0.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Start boundary clamp failed.\n");
        return 1;
    }
    lau_free((void*)ptr1);

    const char* log_end_edge = 
        "[0.001] System boot initialized.\n"
        "[0.002] ReBAR matrix stable.\n"
        "[0.003] WGP online.\n"
        "FATAL: Termination fault.";

    memset(&regs, 0, sizeof(LauRegisterBank));
    regs.sgpr[10] = (uint32_t)((uintptr_t)log_end_edge & 0xFFFFFFFF);
    regs.sgpr[13] = (uint32_t)((uintptr_t)log_end_edge >> 32);
    regs.sgpr[11] = (uint32_t)strlen(log_end_edge);
    regs.sgpr[12] = (uint32_t)((uintptr_t)target_pattern & 0xFFFFFFFF);
    regs.sgpr[14] = (uint32_t)((uintptr_t)target_pattern >> 32);

    tsfi_vision_retrieval_thunk(&regs);

    if (regs.sgpr[0] != 1) {
        tsfi_io_printf(stderr, "[FAIL] Failed to retrieve on end edge.\n");
        return 1;
    }

    uintptr_t ptr2 = ((uintptr_t)regs.sgpr[2] << 32) | (uintptr_t)regs.sgpr[1];
    char* res2 = (char*)ptr2;
    
    if (strstr(res2, "FATAL: Termination fault.") && res2[strlen(res2)-1] == '.') {
        tsfi_io_printf(stdout, "[SUCCESS] End edge boundary securely clamped to manifold EOF.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] End boundary clamp failed or read garbage memory.\n");
        return 1;
    }
    lau_free((void*)ptr2);

    const char* log_clean = 
        "[0.001] System boot initialized.\n"
        "[0.002] All systems nominal.\n";

    memset(&regs, 0, sizeof(LauRegisterBank));
    regs.sgpr[10] = (uint32_t)((uintptr_t)log_clean & 0xFFFFFFFF);
    regs.sgpr[13] = (uint32_t)((uintptr_t)log_clean >> 32);
    regs.sgpr[11] = (uint32_t)strlen(log_clean);
    regs.sgpr[12] = (uint32_t)((uintptr_t)target_pattern & 0xFFFFFFFF);
    regs.sgpr[14] = (uint32_t)((uintptr_t)target_pattern >> 32);

    tsfi_vision_retrieval_thunk(&regs);

    if (regs.sgpr[0] == 0) {
        tsfi_io_printf(stdout, "[SUCCESS] Null set organically rejected. Thunk safely returned 0 state.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Thunk hallucinated a match or failed to return 0.\n");
        return 1;
    }

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
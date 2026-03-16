#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include "tsfi_dys_math.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../thunks/tsfi_vision_retrieval_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Vision Retrieval Thunk Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    const char* log_buffer = 
        "[0.001] System boot initialized.\n"
        "[0.002] ReBAR matrix stable.\n"
        "[0.003] WGP online.\n"
        "[0.004] Neural observation active.\n"
        "[0.005] FATAL: Structural integrity loss detected at address 0x0.\n"
        "[0.006] Safemode engaged.\n"
        "[0.007] Halting wavefronts.\n"
        "[0.008] Power state S3 requested.\n";
        
    const char* target_pattern = "FATAL:";

    regs.sgpr[10] = (uint32_t)((uintptr_t)log_buffer & 0xFFFFFFFF);
    regs.sgpr[13] = (uint32_t)((uintptr_t)log_buffer >> 32);
    regs.sgpr[11] = (uint32_t)strlen(log_buffer);
    regs.sgpr[12] = (uint32_t)((uintptr_t)target_pattern & 0xFFFFFFFF);
    regs.sgpr[14] = (uint32_t)((uintptr_t)target_pattern >> 32);

    tsfi_io_printf(stdout, "[EXEC] Triggering Vision Retrieval Thunk for pattern '%s'...\n", target_pattern);
    
    tsfi_vision_retrieval_thunk(&regs);

    uint32_t success_flag = regs.sgpr[0];
    if (success_flag != 1) {
        tsfi_io_printf(stderr, "[FAIL] Retrieval thunk returned failure or zero matches. Flag: %u\n", success_flag);
        return 1;
    }

    uintptr_t extracted_ptr = ((uintptr_t)regs.sgpr[2] << 32) | (uintptr_t)regs.sgpr[1];
    uint32_t extracted_len = regs.sgpr[3];
    
    char* result_string = (char*)extracted_ptr;

    tsfi_io_printf(stdout, "[SUCCESS] Retrieval completed. Extracted %u bytes:\n", extracted_len);
    tsfi_io_printf(stdout, "---------------------------------------------------\n");
    tsfi_io_printf(stdout, "%s\n", result_string);
    tsfi_io_printf(stdout, "---------------------------------------------------\n");

    if (strstr(result_string, "[0.002] ReBAR matrix stable.") && strstr(result_string, "[0.008] Power state S3 requested.")) {
        tsfi_io_printf(stdout, "[VERIFIED] Context bounds natively resolved 3 lines forward and backward.\n");
    } else {
        tsfi_io_printf(stderr, "[FAIL] Extracted context bounds did not match expected structure.\n");
        lau_free((void*)extracted_ptr);
        return 1;
    }

    lau_free((void*)extracted_ptr);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
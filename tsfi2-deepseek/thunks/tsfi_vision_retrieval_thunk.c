#include "tsfi_io.h"
#include "lau_memory.h"
#include "tsfi_vision.h"
#include "tsfi_dysnomia.h"
#include <string.h>

/**
 * TSFi Vision Retrieval Thunk
 * 
 * This standard cell acts as the sensory filter for the local intelligence.
 * It natively greps the physical memory manifold to extract the explicit 
 * context boundary, formatting it into a zero-noise data injection block 
 * for the cognitive layer.
 */
void tsfi_vision_retrieval_thunk(LauRegisterBank* regs) {
    if (!regs) return;

    // Incoming parameters passed via 32-bit scalar registers
    uintptr_t manifold_ptr = ((uintptr_t)regs->sgpr[13] << 32) | (uintptr_t)regs->sgpr[10];
    uintptr_t pattern_ptr = ((uintptr_t)regs->sgpr[14] << 32) | (uintptr_t)regs->sgpr[12];
    
    const uint8_t* target_manifold = (const uint8_t*)manifold_ptr;
    size_t manifold_size = (size_t)regs->sgpr[11];
    const char* target_pattern = (const char*)pattern_ptr;
    
    if (!target_manifold || manifold_size == 0 || !target_pattern) {
        regs->sgpr[0] = 0; // Error / Null State
        return;
    }

    size_t pattern_len = strlen(target_pattern);
    size_t starts[1];
    size_t ends[1];

    // Execute the Retrieval phase: Extract exactly 3 lines before and 3 lines after the anomaly
    size_t matches = tsfi_io_grep_context(target_manifold, manifold_size, target_pattern, pattern_len, starts, ends, 1, 3, 3);

    if (matches == 0) {
        regs->sgpr[0] = 0; // No anomalies retrieved
        return;
    }

    // Allocate the Augmented context block securely
    size_t context_len = ends[0] - starts[0];
    if (context_len > 4096) context_len = 4096; // Cap the cognitive injection size

    char* prompt_injection_block = (char*)lau_malloc_wired(context_len + 1);
    memcpy(prompt_injection_block, target_manifold + starts[0], context_len);
    prompt_injection_block[context_len] = '\0';

    // Output the structural pointer back to the Host/Vision layer
    regs->sgpr[0] = 1; // Success
    regs->sgpr[1] = (uint32_t)(uintptr_t)prompt_injection_block; // Lower 32-bits of pointer
    regs->sgpr[2] = (uint32_t)((uintptr_t)prompt_injection_block >> 32); // Upper 32-bits
    regs->sgpr[3] = (uint32_t)context_len;
}
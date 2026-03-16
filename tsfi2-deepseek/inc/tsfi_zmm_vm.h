#ifndef TSFI_ZMM_VM_H
#define TSFI_ZMM_VM_H

#include "tsfi_wave512.h"
#include "tsfi_opt_zmm.h"
#include "lau_telemetry.h"

#define TSFI_ZMM_REG_COUNT 16

typedef struct {
    wave512 registers[TSFI_ZMM_REG_COUNT];
    char output_buffer[4096];
    int output_pos;
    
    // Neurology Integration
    TsfiZmmManifest *manifest; // Managed by VM (allocated/freed)
    
    // Telemetry Integration
    LauTelemetryState *telem;  // Attached Shared Memory
} TsfiZmmVmState;

void tsfi_zmm_vm_init(TsfiZmmVmState *state);
void tsfi_zmm_vm_destroy(TsfiZmmVmState *state); 
void tsfi_zmm_vm_exec(TsfiZmmVmState *state, const char *code);

// Instruction Pipelining (Fused Execution)
void* tsfi_zmm_vm_compile_block(const char *code);
size_t tsfi_zmm_vm_compile_block_buffer(const char *code, void *buffer, size_t max_len);
void tsfi_zmm_vm_exec_block(TsfiZmmVmState *state, void *block);
void tsfi_zmm_vm_free_block(void *block);

// Binary Crossover (Zero-Parse Evolution)
// Splices blockA and blockB into child_buf.
size_t tsfi_zmm_vm_splice_block(const void *blockA, const void *blockB, void *child_buf, size_t max_len);

// Attach to telemetry if available
void tsfi_zmm_vm_attach_telemetry(TsfiZmmVmState *state, const char *id);

#endif // TSFI_ZMM_VM_H

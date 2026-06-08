#ifndef TSFI_ZMM_VM_H
#define TSFI_ZMM_VM_H

#include "tsfi_wave512.h"
#include "tsfi_opt_zmm.h"
#include "lau_telemetry.h"

// --- Opcodes for Pipelined Execution ---
enum {
    ZMM_OP_END = 0,
    ZMM_OP_WLOAD,   // [OP, REG, FLOAT_VAL]
    ZMM_OP_WADD,    // [OP, DEST, SRC1, SRC2]
    ZMM_OP_WMUL,    // [OP, DEST, SRC1, SRC2]
    ZMM_OP_WSTORE,  // [OP, SRC, IDX]
    ZMM_OP_WDUMP,   // [OP, REG]
    // --- Hardware Thunk ABI ---
    VM_OP_SEAL,     // [OP, PTR_HI, PTR_LO]
    VM_OP_UNSEAL,   // [OP, PTR_HI, PTR_LO]
    VM_OP_INVOKE,   // [OP, PTR_HI, PTR_LO, OFFSET]
    // --- Yul CPU integration ---
    VM_OP_YUL_INIT, // [OP, NAME_HI, NAME_LO, PATH_HI, PATH_LO, VIRT_ADDR_HI, VIRT_ADDR_LO]
    VM_OP_YUL_EXEC  // [OP, NAME_HI, NAME_LO, CALLDATA_HI, CALLDATA_LO, CALLDATA_SIZE, RETVAL_HI, RETVAL_LO, RETVAL_LEN_HI, RETVAL_LEN_LO]
};

#define TSFI_ZMM_REG_COUNT 16

typedef struct {
    wave512 registers[TSFI_ZMM_REG_COUNT];
    char output_buffer[4096];
    int output_pos;
    
    // Neurology Integration
    TsfiZmmManifest *manifest; // Managed by VM (allocated/freed)
    
    // Telemetry Integration
    LauTelemetryState *telem;  // Attached Shared Memory
    
    // Session Recording
    void *record_file; // Cast to FILE* internally to avoid header clutter
    
    // Checkpointing / FlipTrack Time-Travel
    void *checkpoints; // Array of ZmmCpuCheckpoint
    int checkpoint_count;
    int checkpoint_capacity;
    
    // Microtechnic Solutions REU DMA Telemetry Buffer
    uint8_t *reu_ram;
    uint32_t reu_size;
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
size_t tsfi_zmm_vm_splice_block(const void *blockA, const void *blockB, void *child_buf, size_t max_len);

// Attach to telemetry if available
void tsfi_zmm_vm_attach_telemetry(TsfiZmmVmState *state, const char *id);

#endif // TSFI_ZMM_VM_H

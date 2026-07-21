#ifndef TSFI_SPERONI_IR_ENGINE_H
#define TSFI_SPERONI_IR_ENGINE_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    SPERONI_IR_OP_NOP = 0,
    SPERONI_IR_OP_LOAD_OFFSET,
    SPERONI_IR_OP_STORE_OFFSET,
    SPERONI_IR_OP_SCSI_READ,
    SPERONI_IR_OP_YUL_CALL
} tsfi_speroni_ir_opcode_t;

typedef struct {
    tsfi_speroni_ir_opcode_t opcode;  // Arithmetized Opcode
    uint32_t memory_offset;           // Speroni 1961 Arithmetized Field Offset
    uint64_t immediate_val;           // Immediate value / register argument
} tsfi_speroni_ir_inst_t;

typedef struct {
    uint32_t ir_proc_id;              // Speroni IR Process ID
    tsfi_speroni_ir_inst_t code[64];  // Arithmetized IR Instruction Buffer
    size_t inst_count;                // Instruction Count
    uint32_t evm_gas_units;           // 240 Gas / Auncient Ether Units
    double fet_power_watts;           // 0.0109 W under 78.2% Power Cut
    char tape_dat_bin[128];           // Single-Header Array (.DAT.BIN - Rule 13)
} tsfi_speroni_ir_proc_t;

/* Initialize a Speroni Arithmetized IR Processor */
int tsfi_speroni_ir_proc_init(
    uint32_t ir_proc_id,
    tsfi_speroni_ir_proc_t *proc
);

/* Emit an arithmetized IR instruction into processor buffer */
int tsfi_speroni_ir_emit(
    tsfi_speroni_ir_proc_t *proc,
    tsfi_speroni_ir_opcode_t opcode,
    uint32_t block_idx,
    uint32_t field_idx,
    uint64_t immediate_val
);

#endif // TSFI_SPERONI_IR_ENGINE_H

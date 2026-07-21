#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_speroni_ir_engine.h"
#include "tsfi_speroni_params.h"
#include "tsfi_lowpower_fet.h"

int tsfi_speroni_ir_proc_init(
    uint32_t ir_proc_id,
    tsfi_speroni_ir_proc_t *proc
) {
    if (!proc) return -1;
    memset(proc, 0, sizeof(tsfi_speroni_ir_proc_t));

    proc->ir_proc_id = ir_proc_id;
    proc->evm_gas_units = 240; // 240 Gas / Auncient Ether Units per IR instruction slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    proc->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(proc->tape_dat_bin, sizeof(proc->tape_dat_bin), "SPERONI_IR_%08X.DAT.BIN", ir_proc_id);
    return 0;
}

int tsfi_speroni_ir_emit(
    tsfi_speroni_ir_proc_t *proc,
    tsfi_speroni_ir_opcode_t opcode,
    uint32_t block_idx,
    uint32_t field_idx,
    uint64_t immediate_val
) {
    if (!proc || proc->inst_count >= 64) return -1;

    size_t idx = proc->inst_count;
    proc->code[idx].opcode = opcode;
    proc->code[idx].memory_offset = tsfi_speroni_arithmetize_header_offset(block_idx, field_idx);
    proc->code[idx].immediate_val = immediate_val;
    proc->inst_count++;

    printf("[SPERONI IR EMIT] Proc: %u | Inst: %zu | Op: %d | Arithmetized Offset: 0x%04X (%u bytes) | Immed: 0x%llX | Gas: %u | File: %s\n",
           proc->ir_proc_id, proc->inst_count, (int)opcode,
           proc->code[idx].memory_offset, proc->code[idx].memory_offset,
           (unsigned long long)immediate_val, proc->evm_gas_units, proc->tape_dat_bin);

    return 0; // Instruction emit successful
}

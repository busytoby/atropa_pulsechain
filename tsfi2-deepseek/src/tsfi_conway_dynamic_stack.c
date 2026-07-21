#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_conway_dynamic_stack.h"
#include "tsfi_lowpower_fet.h"

int tsfi_conway_dynamic_stack_init(
    uint32_t process_id,
    tsfi_conway_dynamic_stack_t *stack
) {
    if (!stack) return -1;
    memset(stack, 0, sizeof(tsfi_conway_dynamic_stack_t));

    stack->process_id = process_id;
    stack->evm_gas_units = 220; // 220 Gas / Auncient Ether Units per stack frame slot

    // Rule 10: Verlet Soft-Body FET Discharge Physics Solver (3.3V Low-Power Floor)
    tsfi_lowpower_fet_metrics_t fet_metrics;
    tsfi_lowpower_fet_calculate(1e9f, 1e-12f, 5.0f, 3.3f, &fet_metrics);
    stack->fet_power_watts = (double)fet_metrics.optimized_power_watts; // 0.0109 W (78.2% Cut!)

    // Format Rule 13 dataset filename (.DAT.BIN)
    snprintf(stack->tape_dat_bin, sizeof(stack->tape_dat_bin), "CONWAY_DYNAMIC_STACK_%08X.DAT.BIN", process_id);
    return 0;
}

int tsfi_conway_dynamic_stack_push(
    tsfi_conway_dynamic_stack_t *stack,
    uint32_t return_pc,
    uint64_t local_word
) {
    if (!stack) return -1;

    if (stack->depth >= CONWAY_MAX_STACK_DEPTH) {
        printf("[CONWAY STACK OVERFLOW] Proc ID: %u | Max Depth %d Reached!\n", stack->process_id, CONWAY_MAX_STACK_DEPTH);
        return 1; // Stack Overflow Error
    }

    tsfi_conway_stack_frame_t *frame = &stack->frames[stack->depth];
    frame->return_pc = return_pc;
    frame->frame_local_word = local_word;
    stack->depth++;

    printf("[CONWAY STACK PUSH] Proc ID: %u | New Depth: %u | Return PC: 0x%04X | Local Word: 0x%016llX | Gas: %u | File: %s\n",
           stack->process_id, stack->depth, return_pc, (unsigned long long)local_word,
           stack->evm_gas_units, stack->tape_dat_bin);

    return 0; // Push successful
}

int tsfi_conway_dynamic_stack_pop(
    tsfi_conway_dynamic_stack_t *stack,
    uint32_t *out_return_pc,
    uint64_t *out_local_word
) {
    if (!stack || !out_return_pc || !out_local_word) return -1;

    if (stack->depth == 0) {
        printf("[CONWAY STACK UNDERFLOW] Proc ID: %u | Stack Empty!\n", stack->process_id);
        return 2; // Stack Underflow Error
    }

    stack->depth--;
    tsfi_conway_stack_frame_t *frame = &stack->frames[stack->depth];
    *out_return_pc = frame->return_pc;
    *out_local_word = frame->frame_local_word;

    printf("[CONWAY STACK POP] Proc ID: %u | Remaining Depth: %u | Popped Return PC: 0x%04X | Popped Local: 0x%016llX | Gas: %u | File: %s\n",
           stack->process_id, stack->depth, *out_return_pc, (unsigned long long)*out_local_word,
           stack->evm_gas_units, stack->tape_dat_bin);

    return 0; // Pop successful
}

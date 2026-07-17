#ifndef TSFI_STRATEGY_LANG_H
#define TSFI_STRATEGY_LANG_H

#include "tsfi_priority_queue.h"

// Strategy Language VM Context
typedef struct {
    int depth_priority_scale;
    int abductive_priority_scale;
    int executed_evals;
    int registers[4]; // General-purpose registers R0-R3
    int call_stack[8]; // Call stack for subroutine return PCs
    int stack_pointer; // Pointer to top of stack
    int eval_stack[32]; // Evaluation stack for Burroughs B5000 stack-oriented instructions
    int eval_stack_ptr; // Top pointer for evaluation stack
} TSFiStrategyVM;

// Initialize the Strategy VM
void tsfi_strategy_vm_init(TSFiStrategyVM *vm);

// Parse and execute a strategy language script
int tsfi_strategy_vm_execute(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const char *script, void *logic_vm);

int tsfi_strategy_vm_execute_bytecode(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const uint8_t *bytecode, int len, void *logic_vm);

int tsfi_strategy_compile_script(const char *script, uint8_t *bytecode_out, int max_len, int *len_out);

typedef struct {
    int registers[4];
    int queue_size;
} TSFiStrategyTelemetry;

void tsfi_strategy_vm_broadcast(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, TSFiStrategyTelemetry *telemetry);
void tsfi_strategy_vm_bind_dbtg(TSFiStrategyVM *vm, const void *cur, const void *realm_reg);

#endif // TSFI_STRATEGY_LANG_H

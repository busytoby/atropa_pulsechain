#ifndef TSFI_STRATEGY_LANG_H
#define TSFI_STRATEGY_LANG_H

#include "tsfi_priority_queue.h"

// Strategy Language VM Context
typedef struct {
    int depth_priority_scale;
    int abductive_priority_scale;
    int executed_evals;
} TSFiStrategyVM;

// Initialize the Strategy VM
void tsfi_strategy_vm_init(TSFiStrategyVM *vm);

// Parse and execute a strategy language script
int tsfi_strategy_vm_execute(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const char *script);

int tsfi_strategy_vm_execute_bytecode(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const uint8_t *bytecode, int len);

int tsfi_strategy_compile_script(const char *script, uint8_t *bytecode_out, int max_len, int *len_out);

#endif // TSFI_STRATEGY_LANG_H

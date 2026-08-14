#ifndef TSFI_STRATEGY_LANG_H
#define TSFI_STRATEGY_LANG_H

#include "tsfi_priority_queue.h"

// Strategy Language VM Context
typedef struct {
    int depth_priority_scale;
    int abductive_priority_scale;
    int executed_evals;
    int registers[16]; // General-purpose registers R0-R15
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

// Universal Transactional Receipt Output for all Strategy executions (Zorse, OpenClaw, DeepSeek)
typedef struct {
    uint32_t magic_header;       // 0x52435054 ('R''C''P''T')
    char strategy_name[64];      // Name/identifier of the executed COBOL strategy
    int registers[16];           // Snapshot of VM registers R0-R15
    int depth_scale;             // Final VM depth priority scale
    int abductive_scale;         // Final VM abductive priority scale
    int executed_evals;          // Total evaluations executed
    int final_queue_size;        // Final items remaining in Priority Queue
    uint32_t top_keycode;        // Keycode of highest priority item
    int top_priority;            // Priority of top item
    uint64_t receipt_timestamp;  // Execution cycle / timestamp
    uint8_t receipt_sha[32];     // Receipt state hash
} TSFiStrategyReceipt;

void tsfi_strategy_vm_broadcast(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, TSFiStrategyTelemetry *telemetry);
void tsfi_strategy_vm_bind_dbtg(TSFiStrategyVM *vm, const void *cur, const void *realm_reg);
int tsfi_strategy_vm_generate_receipt(const TSFiStrategyVM *vm, const TSFiPriorityQueue *pq, const char *strategy_name, TSFiStrategyReceipt *receipt_out);
int tsfi_strategy_load_and_run(const char *strategy_filename, int r0, int r1, int r2, int r3, TSFiStrategyVM *vm_out, TSFiStrategyReceipt *receipt_out);

#endif // TSFI_STRATEGY_LANG_H

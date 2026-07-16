#ifndef TSFI_DUAL_STACK_H
#define TSFI_DUAL_STACK_H

#include "tsfi_akb.h"

// Opcode Definitions
#define OP_PUSH_CONS    0x41 // Push value to Constraint Stack
#define OP_PUSH_SEMA    0x42 // Push value to Semantic Stack
#define OP_VERIFY_CONS  0x43 // Pop & assert constraint (backtracks on failure)
#define OP_RESOLVE_SEMA 0x44 // Pop & write semantic temporary deduction to AKB
#define OP_HALT_DUAL    0x45

// Semantic-Constraint Dual Stack VM
typedef struct {
    int constraint_stack[32];
    int cons_len;
    
    char semantic_stack[32][128]; // Holds deduced path variables
    int sema_len;

    int backtrack_triggered;
    int halted;
} TSFiDualStackVM;

// Execute dual stack VM bytecode over AKB coordinate cache rails
int tsfi_dual_stack_execute(TSFiDualStackVM *vm, TSFiAKB *akb, const int *bytecode, size_t len);

#endif // TSFI_DUAL_STACK_H

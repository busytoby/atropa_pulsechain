#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_dual_stack.h"

int tsfi_dual_stack_execute(TSFiDualStackVM *vm, TSFiAKB *akb, const int *bytecode, size_t len) {
    if (!vm || !akb || !bytecode || len == 0) return -1;

    size_t pc = 0;
    vm->cons_len = 0;
    vm->sema_len = 0;
    vm->backtrack_triggered = 0;
    vm->halted = 0;

    while (pc < len && !vm->halted && !vm->backtrack_triggered) {
        int op = bytecode[pc++];
        switch (op) {
            case OP_PUSH_CONS:
                if (pc >= len) return -2;
                vm->constraint_stack[vm->cons_len++] = bytecode[pc++];
                break;
                
            case OP_PUSH_SEMA:
                if (pc >= len) return -2;
                {
                    // Map index pointer to actual string identifier (mock dictionary lookup)
                    int path_idx = bytecode[pc++];
                    if (path_idx == 1) {
                        strncpy(vm->semantic_stack[vm->sema_len++], "svdag/1/10/10", 127);
                    } else {
                        strncpy(vm->semantic_stack[vm->sema_len++], "svdag/unknown", 127);
                    }
                }
                break;

            case OP_VERIFY_CONS:
                if (vm->cons_len < 1) return -3;
                {
                    int val = vm->constraint_stack[--vm->cons_len];
                    if (val <= 0) {
                        // Constraint failed -> Trigger Backtracking
                        vm->backtrack_triggered = 1;
                    }
                }
                break;

            case OP_RESOLVE_SEMA:
                if (vm->sema_len < 1) return -3;
                {
                    // Deduce and write temporary state to cache rails
                    char *path = vm->semantic_stack[--vm->sema_len];
                    tsfi_akb_write_temp(akb, path, "RELATION_TRUE");
                }
                break;

            case OP_HALT_DUAL:
                vm->halted = 1;
                break;

            default:
                return -4; // Unknown opcode
        }
    }

    return vm->backtrack_triggered ? 1 : 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_anvil_vm.h"

void tsfi_anvil_vm_init(TSFiAnvilVM *vm) {
    if (!vm) return;
    vm->trail_len = 0;
    vm->subgoal_count = 0;
    vm->backtrack_triggered = 0;
    memset(vm->subgoal_table, 0, sizeof(vm->subgoal_table));
}

void tsfi_anvil_vm_bind(TSFiAnvilVM *vm, const char *key, const char *value) {
    if (!vm || !key || !value || vm->trail_len >= 64) return;

    // Push to WAM Trail
    strncpy(vm->trail_keys[vm->trail_len++], key, 127);

    // Save as calling subgoal in the lookup table
    tsfi_anvil_vm_memoize_subgoal(vm, key, value, SUBGOAL_CALLING);
}

void tsfi_anvil_vm_backtrack(TSFiAnvilVM *vm, int target_len) {
    if (!vm || target_len < 0 || target_len > vm->trail_len) return;

    // Unbind variables popped from WAM Trail
    while (vm->trail_len > target_len) {
        char *key = vm->trail_keys[--vm->trail_len];

        // Find and remove from subgoal table (Unbind)
        for (int i = 0; i < vm->subgoal_count; i++) {
            if (strcmp(vm->subgoal_table[i].key, key) == 0) {
                // Swap last entry to delete
                vm->subgoal_table[i] = vm->subgoal_table[--vm->subgoal_count];
                break;
            }
        }
    }

    vm->backtrack_triggered = 1;
}

void tsfi_anvil_vm_memoize_subgoal(TSFiAnvilVM *vm, const char *key, const char *val, TSFiSubgoalState state) {
    if (!vm || !key || !val || vm->subgoal_count >= 64) return;

    // Check if subgoal already exists to update it
    for (int i = 0; i < vm->subgoal_count; i++) {
        if (strcmp(vm->subgoal_table[i].key, key) == 0) {
            strncpy(vm->subgoal_table[i].value, val, 127);
            vm->subgoal_table[i].state = state;
            return;
        }
    }

    // Insert new subgoal
    strncpy(vm->subgoal_table[vm->subgoal_count].key, key, 127);
    strncpy(vm->subgoal_table[vm->subgoal_count].value, val, 127);
    vm->subgoal_table[vm->subgoal_count].state = state;
    vm->subgoal_count++;
}

const TSFiSubgoalEntry* tsfi_anvil_vm_lookup_subgoal(TSFiAnvilVM *vm, const char *key) {
    if (!vm || !key) return NULL;

    for (int i = 0; i < vm->subgoal_count; i++) {
        if (strcmp(vm->subgoal_table[i].key, key) == 0) {
            return &vm->subgoal_table[i];
        }
    }

    return NULL;
}

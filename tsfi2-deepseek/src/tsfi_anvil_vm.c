#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_anvil_vm.h"

// Fast hash calculation for subgoal coordinates
static unsigned int subgoal_hash(const char *str) {
    unsigned int hash = 0;
    while (*str) {
        hash = hash * 31 + *str++;
    }
    return hash % 64;
}

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

        // Find and remove from subgoal table (Unbind) using hash index
        unsigned int h = subgoal_hash(key);
        for (int i = 0; i < 64; i++) {
            int slot = (h + i) % 64;
            if (vm->subgoal_table[slot].key[0] != '\0' && strcmp(vm->subgoal_table[slot].key, key) == 0) {
                memset(&vm->subgoal_table[slot], 0, sizeof(TSFiSubgoalEntry));
                vm->subgoal_count--;
                break;
            }
        }
    }

    vm->backtrack_triggered = 1;
}

void tsfi_anvil_vm_memoize_subgoal(TSFiAnvilVM *vm, const char *key, const char *val, TSFiSubgoalState state) {
    if (!vm || !key || !val || vm->subgoal_count >= 64) return;

    unsigned int h = subgoal_hash(key);
    for (int i = 0; i < 64; i++) {
        int slot = (h + i) % 64;
        
        // Update existing subgoal
        if (vm->subgoal_table[slot].key[0] != '\0' && strcmp(vm->subgoal_table[slot].key, key) == 0) {
            strncpy(vm->subgoal_table[slot].value, val, 127);
            vm->subgoal_table[slot].state = state;
            return;
        }
        
        // Insert new subgoal in empty slot
        if (vm->subgoal_table[slot].key[0] == '\0') {
            strncpy(vm->subgoal_table[slot].key, key, 127);
            strncpy(vm->subgoal_table[slot].value, val, 127);
            vm->subgoal_table[slot].state = state;
            vm->subgoal_count++;
            return;
        }
    }
}

const TSFiSubgoalEntry* tsfi_anvil_vm_lookup_subgoal(TSFiAnvilVM *vm, const char *key) {
    if (!vm || !key) return NULL;

    unsigned int h = subgoal_hash(key);
    for (int i = 0; i < 64; i++) {
        int slot = (h + i) % 64;
        if (vm->subgoal_table[slot].key[0] != '\0' && strcmp(vm->subgoal_table[slot].key, key) == 0) {
            return &vm->subgoal_table[slot];
        }
        // If slot is empty, target key does not exist (due to linear probing boundary)
        if (vm->subgoal_table[slot].key[0] == '\0') {
            return NULL;
        }
    }

    return NULL;
}

int tsfi_anvil_vm_execute(TSFiAnvilVM *vm, const int *bytecode, int len, float prior_h, float prior_e, float threshold) {
    if (!vm || !bytecode || len <= 0) return -1;
    
    int pc = 0;
    while (pc < len) {
        int op = bytecode[pc++];
        if (op == 0x5A) { // OP_VERIFY_ABDUCTIVE
            // Calculate abductive relation: P(H|E) = (P(H) * 0.8) / P(E)
            float posterior = (prior_h * 0.8f) / (prior_e > 0.0f ? prior_e : 1.0f);
            if (posterior < threshold) {
                // Assert fail: trigger WAM backtrack unbinding!
                tsfi_anvil_vm_backtrack(vm, 0);
                return 0; // Verification failed
            }
        } else if (op == 0x5B) { // OP_BACKTRACK_RAIL
            tsfi_anvil_vm_backtrack(vm, 0);
            return 0;
        } else if (op == 0x5C) { // OP_RESOLVE_IMPLICATION
            if (pc + 2 < len) {
                int src_a = bytecode[pc++];
                int src_b = bytecode[pc++];
                int dst = bytecode[pc++];
                if (src_a < 64 && src_b < 64 && dst < 64) {
                    TSFiSubgoalEntry *entry_a = &vm->subgoal_table[src_a];
                    TSFiSubgoalEntry *entry_b = &vm->subgoal_table[src_b];
                    TSFiSubgoalEntry *entry_dst = &vm->subgoal_table[dst];
                    if (entry_a->key[0] != '\0' && entry_b->key[0] != '\0' && entry_dst->key[0] != '\0') {
                        if (entry_a->value[0] == 'T' && entry_b->value[0] == 'T') {
                            entry_dst->value[0] = 'T';
                            entry_dst->value[1] = 'R';
                            entry_dst->value[2] = 'U';
                            entry_dst->value[3] = 'E';
                            entry_dst->value[4] = '\0';
                        }
                    }
                }
            }
        }
    }
    return 1; // Passed abductive verification rails
}

#ifndef TSFI_ANVIL_VM_H
#define TSFI_ANVIL_VM_H

#include "tsfi_trie.h"

// Subgoal States
typedef enum {
    SUBGOAL_CALLING,
    SUBGOAL_COMPLETED
} TSFiSubgoalState;

// Subgoal Table Entry
typedef struct {
    char key[128];
    char value[128];
    TSFiSubgoalState state;
} TSFiSubgoalEntry;

// Anvil Tabled VM Context
typedef struct {
    // WAM Trail (Backtrack register pointers)
    char trail_keys[64][128];
    int trail_len;

    // Subgoal Memoization Table
    TSFiSubgoalEntry subgoal_table[64];
    int subgoal_count;

    int backtrack_triggered;
} TSFiAnvilVM;

// Initialize Anvil VM Context
void tsfi_anvil_vm_init(TSFiAnvilVM *vm);

// Bind a variable and push to the WAM Trail
void tsfi_anvil_vm_bind(TSFiAnvilVM *vm, const char *key, const char *value);

// Backtrack WAM Trail (unbinds all variables down to target length)
void tsfi_anvil_vm_backtrack(TSFiAnvilVM *vm, int target_len);

// Memoize subgoal state
void tsfi_anvil_vm_memoize_subgoal(TSFiAnvilVM *vm, const char *key, const char *val, TSFiSubgoalState state);

// Retrieve memoized subgoal
const TSFiSubgoalEntry* tsfi_anvil_vm_lookup_subgoal(TSFiAnvilVM *vm, const char *key);

// Run abductive logic verification bytecode
int tsfi_anvil_vm_execute(TSFiAnvilVM *vm, const int *bytecode, int len, float prior_h, float prior_e, float threshold);

#endif // TSFI_ANVIL_VM_H

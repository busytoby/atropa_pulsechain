#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_strategy_lang.h"

void tsfi_strategy_vm_init(TSFiStrategyVM *vm) {
    if (!vm) return;
    vm->depth_priority_scale = 1;
    vm->abductive_priority_scale = 1;
    vm->executed_evals = 0;
}

int tsfi_strategy_vm_execute(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const char *script) {
    if (!vm || !script) return -1;

    char script_copy[512];
    strncpy(script_copy, script, 511);
    script_copy[511] = '\0';

    char *token = strtok(script_copy, " ;");
    while (token) {
        if (strcmp(token, "SET") == 0) {
            char *param = strtok(NULL, " ;");
            char *val_str = strtok(NULL, " ;");
            if (param && val_str) {
                int val = atoi(val_str);
                if (strcmp(param, "depth") == 0) {
                    vm->depth_priority_scale = val;
                } else if (strcmp(param, "abductive") == 0) {
                    vm->abductive_priority_scale = val;
                }
            }
        } else if (strcmp(token, "EVAL") == 0) {
            vm->executed_evals++;
            // Re-prioritize active queue items according to scaling strategy
            if (pq) {
                for (int i = 0; i < pq->size; i++) {
                    pq->items[i].priority = (pq->items[i].priority * vm->depth_priority_scale) + vm->abductive_priority_scale;
                }
            }
        }
        token = strtok(NULL, " ;");
    }

    return 0;
}

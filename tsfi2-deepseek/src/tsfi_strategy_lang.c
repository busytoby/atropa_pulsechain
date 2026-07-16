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

// Restore heap invariants after priority mutations
static void heapify(TSFiPriorityQueue *pq) {
    for (int i = (pq->size / 2) - 1; i >= 0; i--) {
        int idx = i;
        while (2 * idx + 1 < pq->size) {
            int left = 2 * idx + 1;
            int right = left + 1;
            int smallest = left;
            if (right < pq->size && pq->items[right].priority < pq->items[left].priority) {
                smallest = right;
            }
            if (pq->items[idx].priority <= pq->items[smallest].priority) {
                break;
            }
            TSFiQueueItem temp = pq->items[idx];
            pq->items[idx] = pq->items[smallest];
            pq->items[smallest] = temp;
            idx = smallest;
        }
    }
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
        } else if (strcmp(token, "PRUNE") == 0) {
            char *thresh_str = strtok(NULL, " ;");
            if (thresh_str && pq) {
                int threshold = atoi(thresh_str);
                // Drop any item exceeding threshold priority
                int write_idx = 0;
                for (int i = 0; i < pq->size; i++) {
                    if (pq->items[i].priority <= threshold) {
                        pq->items[write_idx++] = pq->items[i];
                    }
                }
                pq->size = write_idx;
            }
        } else if (strcmp(token, "WEIGHT") == 0) {
            char *subgoal_str = strtok(NULL, " ;");
            char *weight_str = strtok(NULL, " ;");
            if (subgoal_str && weight_str && pq) {
                int keycode = atoi(subgoal_str);
                int weight = atoi(weight_str);
                for (int i = 0; i < pq->size; i++) {
                    if (pq->items[i].keycode == keycode) {
                        pq->items[i].priority = weight;
                    }
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

    if (pq) {
        heapify(pq);
    }
    return 0;
}

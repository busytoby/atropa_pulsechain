#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_strategy_lang.h"

void tsfi_strategy_vm_init(TSFiStrategyVM *vm) {
    if (!vm) return;
    vm->depth_priority_scale = 1;
    vm->abductive_priority_scale = 1;
    vm->executed_evals = 0;
    memset(vm->registers, 0, sizeof(vm->registers));
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

int tsfi_strategy_vm_execute_bytecode(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const uint8_t *bytecode, int len) {
    if (!vm || !bytecode || len <= 0) return -1;

    int pc = 0;
    while (pc < len) {
        uint8_t op = bytecode[pc++];
        if (op == 0x01) { // SET_DEPTH
            if (pc < len) {
                vm->depth_priority_scale = bytecode[pc++];
            }
        } else if (op == 0x02) { // SET_ABDUCTIVE
            if (pc < len) {
                vm->abductive_priority_scale = bytecode[pc++];
            }
        } else if (op == 0x03) { // OP_EVAL
            vm->executed_evals++;
            if (pq) {
                for (int i = 0; i < pq->size; i++) {
                    pq->items[i].priority = (pq->items[i].priority * vm->depth_priority_scale) + vm->abductive_priority_scale;
                }
            }
        } else if (op == 0x04) { // OP_PRUNE
            if (pc < len && pq) {
                uint8_t threshold = bytecode[pc++];
                int write_idx = 0;
                for (int i = 0; i < pq->size; i++) {
                    if (pq->items[i].priority <= threshold) {
                        pq->items[write_idx++] = pq->items[i];
                    }
                }
                pq->size = write_idx;
            }
        } else if (op == 0x05) { // OP_WEIGHT
            if (pc + 1 < len && pq) {
                uint8_t keycode = bytecode[pc++];
                uint8_t weight = bytecode[pc++];
                for (int i = 0; i < pq->size; i++) {
                    if (pq->items[i].keycode == keycode) {
                        pq->items[i].priority = weight;
                    }
                }
            }
        } else if (op == 0x10) { // ADD dst src
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 4 && src < 4) {
                    vm->registers[dst] += vm->registers[src];
                }
            }
        } else if (op == 0x11) { // SUB dst src
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 4 && src < 4) {
                    vm->registers[dst] -= vm->registers[src];
                }
            }
        } else if (op == 0x12) { // JEQ target_pc dst src
            if (pc + 2 < len) {
                uint8_t target_pc = bytecode[pc++];
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 4 && src < 4 && vm->registers[dst] == vm->registers[src]) {
                    if (target_pc < len) {
                        pc = target_pc;
                    }
                }
            }
        } else if (op == 0x13) { // JLT target_pc dst src
            if (pc + 2 < len) {
                uint8_t target_pc = bytecode[pc++];
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 4 && src < 4 && vm->registers[dst] < vm->registers[src]) {
                    if (target_pc < len) {
                        pc = target_pc;
                    }
                }
            }
        } else if (op == 0x14) { // SET_REG reg val
            if (pc + 1 < len) {
                uint8_t reg = bytecode[pc++];
                uint8_t val = bytecode[pc++];
                if (reg < 4) {
                    vm->registers[reg] = val;
                }
            }
        } else if (op == 0x15) { // GET_PRIO dst keycode
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t keycode = bytecode[pc++];
                if (dst < 4) {
                    int found_prio = -1;
                    if (pq) {
                        for (int i = 0; i < pq->size; i++) {
                            if (pq->items[i].keycode == keycode) {
                                found_prio = pq->items[i].priority;
                                break;
                            }
                        }
                    }
                    vm->registers[dst] = found_prio;
                }
            }
        } else if (op == 0x16) { // GET_SIZE dst
            if (pc < len) {
                uint8_t dst = bytecode[pc++];
                if (dst < 4) {
                    vm->registers[dst] = pq ? pq->size : 0;
                }
            }
        }
    }

    if (pq) {
        heapify(pq);
    }
    return 0;
}

int tsfi_strategy_compile_script(const char *script, uint8_t *bytecode_out, int max_len, int *len_out) {
    if (!script || !bytecode_out || !len_out || max_len <= 0) return -1;

    char script_copy[512];
    strncpy(script_copy, script, 511);
    script_copy[511] = '\0';

    int pc = 0;
    char *token = strtok(script_copy, " ;");
    while (token) {
        if (strcmp(token, "SET") == 0) {
            char *param = strtok(NULL, " ;");
            char *val_str = strtok(NULL, " ;");
            if (param && val_str && pc + 1 < max_len) {
                int val = atoi(val_str);
                if (strcmp(param, "depth") == 0) {
                    bytecode_out[pc++] = 0x01; // SET_DEPTH
                    bytecode_out[pc++] = (uint8_t)val;
                } else if (strcmp(param, "abductive") == 0) {
                    bytecode_out[pc++] = 0x02; // SET_ABDUCTIVE
                    bytecode_out[pc++] = (uint8_t)val;
                }
            }
        } else if (strcmp(token, "EVAL") == 0) {
            if (pc < max_len) {
                bytecode_out[pc++] = 0x03; // OP_EVAL
            }
        } else if (strcmp(token, "PRUNE") == 0) {
            char *thresh_str = strtok(NULL, " ;");
            if (thresh_str && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x04; // OP_PRUNE
                bytecode_out[pc++] = (uint8_t)atoi(thresh_str);
            }
        } else if (strcmp(token, "WEIGHT") == 0) {
            char *subgoal_str = strtok(NULL, " ;");
            char *weight_str = strtok(NULL, " ;");
            if (subgoal_str && weight_str && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x05; // OP_WEIGHT
                bytecode_out[pc++] = (uint8_t)atoi(subgoal_str);
                bytecode_out[pc++] = (uint8_t)atoi(weight_str);
            }
        } else if (strcmp(token, "SET_REG") == 0) {
            char *reg_str = strtok(NULL, " ;");
            char *val_str = strtok(NULL, " ;");
            if (reg_str && val_str && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x14; // SET_REG
                bytecode_out[pc++] = (uint8_t)(reg_str[1] - '0');
                bytecode_out[pc++] = (uint8_t)atoi(val_str);
            }
        } else if (strcmp(token, "ADD") == 0) {
            char *dst_str = strtok(NULL, " ;");
            char *src_str = strtok(NULL, " ;");
            if (dst_str && src_str && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x10; // ADD
                bytecode_out[pc++] = (uint8_t)(dst_str[1] - '0');
                bytecode_out[pc++] = (uint8_t)(src_str[1] - '0');
            }
        } else if (strcmp(token, "SUB") == 0) {
            char *dst_str = strtok(NULL, " ;");
            char *src_str = strtok(NULL, " ;");
            if (dst_str && src_str && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x11; // SUB
                bytecode_out[pc++] = (uint8_t)(dst_str[1] - '0');
                bytecode_out[pc++] = (uint8_t)(src_str[1] - '0');
            }
        } else if (strcmp(token, "JEQ") == 0) {
            char *target_str = strtok(NULL, " ;");
            char *dst_str = strtok(NULL, " ;");
            char *src_str = strtok(NULL, " ;");
            if (target_str && dst_str && src_str && pc + 3 < max_len) {
                bytecode_out[pc++] = 0x12; // JEQ
                bytecode_out[pc++] = (uint8_t)atoi(target_str);
                bytecode_out[pc++] = (uint8_t)(dst_str[1] - '0');
                bytecode_out[pc++] = (uint8_t)(src_str[1] - '0');
            }
        } else if (strcmp(token, "JLT") == 0) {
            char *target_str = strtok(NULL, " ;");
            char *dst_str = strtok(NULL, " ;");
            char *src_str = strtok(NULL, " ;");
            if (target_str && dst_str && src_str && pc + 3 < max_len) {
                bytecode_out[pc++] = 0x13; // JLT
                bytecode_out[pc++] = (uint8_t)atoi(target_str);
                bytecode_out[pc++] = (uint8_t)(dst_str[1] - '0');
                bytecode_out[pc++] = (uint8_t)(src_str[1] - '0');
            }
        } else if (strcmp(token, "GET_PRIO") == 0) {
            char *dst_str = strtok(NULL, " ;");
            char *keycode_str = strtok(NULL, " ;");
            if (dst_str && keycode_str && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x15; // GET_PRIO
                bytecode_out[pc++] = (uint8_t)(dst_str[1] - '0');
                bytecode_out[pc++] = (uint8_t)atoi(keycode_str);
            }
        } else if (strcmp(token, "GET_SIZE") == 0) {
            char *dst_str = strtok(NULL, " ;");
            if (dst_str && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x16; // GET_SIZE
                bytecode_out[pc++] = (uint8_t)(dst_str[1] - '0');
            }
        }
        token = strtok(NULL, " ;");
    }

    *len_out = pc;
    return 0;
}

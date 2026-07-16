#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_strategy_lang.h"
#include "tsfi_anvil_vm.h"

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

int tsfi_strategy_vm_execute(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const char *script, void *logic_vm) {
    if (!vm || !script) return -1;
    uint8_t bytecode[512];
    int len = 0;
    int res = tsfi_strategy_compile_script(script, bytecode, 512, &len);
    if (res != 0) return res;
    return tsfi_strategy_vm_execute_bytecode(vm, pq, bytecode, len, logic_vm);
}

int tsfi_strategy_vm_execute_bytecode(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, const uint8_t *bytecode, int len, void *logic_vm) {
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
        } else if (op == 0x17) { // OP_LOOP_UNTIL_EMPTY loop_pc
            if (pc < len) {
                uint8_t loop_pc = bytecode[pc++];
                if (pq && pq->size > 0 && loop_pc < len) {
                    pc = loop_pc;
                }
            }
        } else if (op == 0x18) { // OP_GET_LOGIC dst len chars...
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t str_len = bytecode[pc++];
                if (pc + str_len <= len && dst < 4) {
                    char query_key[128];
                    memcpy(query_key, &bytecode[pc], str_len);
                    query_key[str_len] = '\0';
                    pc += str_len;

                    int val = -1;
                    if (logic_vm) {
                        const TSFiSubgoalEntry *entry = tsfi_anvil_vm_lookup_subgoal((TSFiAnvilVM *)logic_vm, query_key);
                        if (entry) {
                            if (strcmp(entry->value, "TRUE") == 0) val = 1;
                            else if (strcmp(entry->value, "FALSE") == 0) val = 0;
                            else if (strcmp(entry->value, "PENDING") == 0) val = 2;
                        }
                    }
                    vm->registers[dst] = val;
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

    // Delimiters include whitespace, periods, parens, braces, commas, quotes, and semicolons
    const char *delims = " ;(),{}\"\n\r\t.";
    
    // Store tokens in an array for multi-token lookahead parsing
    char *tokens[128];
    int token_count = 0;
    
    char *tok = strtok(script_copy, delims);
    while (tok && token_count < 128) {
        tokens[token_count++] = tok;
        tok = strtok(NULL, delims);
    }

    int pc = 0;
    int idx = 0;
    while (idx < token_count) {
        char *t = tokens[idx];
        if (strcmp(t, "SET") == 0) {
            // Support legacy: SET depth 3;
            if (idx + 2 < token_count) {
                char *param = tokens[idx + 1];
                int val = atoi(tokens[idx + 2]);
                if (strcmp(param, "depth") == 0 && pc + 1 < max_len) {
                    bytecode_out[pc++] = 0x01;
                    bytecode_out[pc++] = (uint8_t)val;
                } else if (strcmp(param, "abductive") == 0 && pc + 1 < max_len) {
                    bytecode_out[pc++] = 0x02;
                    bytecode_out[pc++] = (uint8_t)val;
                }
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "depth") == 0) {
            // C-like: depth = 3;
            if (idx + 2 < token_count && strcmp(tokens[idx + 1], "=") == 0 && pc + 1 < max_len) {
                int val = atoi(tokens[idx + 2]);
                bytecode_out[pc++] = 0x01;
                bytecode_out[pc++] = (uint8_t)val;
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "abductive") == 0) {
            // C-like: abductive = 5;
            if (idx + 2 < token_count && strcmp(tokens[idx + 1], "=") == 0 && pc + 1 < max_len) {
                int val = atoi(tokens[idx + 2]);
                bytecode_out[pc++] = 0x02;
                bytecode_out[pc++] = (uint8_t)val;
                idx += 3;
            } else {
                idx++;
            }
        } else if (t[0] == 'R' && t[1] >= '0' && t[1] <= '3' && t[2] == '\0') {
            // Register assignment or operation:
            // R0 = 10; or R0 = R0 + R1;
            int dst = t[1] - '0';
            if (idx + 4 < token_count && strcmp(tokens[idx + 1], "=") == 0 &&
                strcmp(tokens[idx + 2], t) == 0 &&
                (strcmp(tokens[idx + 3], "+") == 0 || strcmp(tokens[idx + 3], "ADD") == 0) &&
                tokens[idx + 4][0] == 'R' && tokens[idx + 4][1] >= '0' && tokens[idx + 4][1] <= '3' && pc + 2 < max_len) {
                // ADD R0 R1
                bytecode_out[pc++] = 0x10;
                bytecode_out[pc++] = (uint8_t)dst;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 4][1] - '0');
                idx += 5;
            } else if (idx + 4 < token_count && strcmp(tokens[idx + 1], "=") == 0 &&
                strcmp(tokens[idx + 2], t) == 0 &&
                (strcmp(tokens[idx + 3], "-") == 0 || strcmp(tokens[idx + 3], "SUB") == 0) &&
                tokens[idx + 4][0] == 'R' && tokens[idx + 4][1] >= '0' && tokens[idx + 4][1] <= '3' && pc + 2 < max_len) {
                // SUB R0 R1
                bytecode_out[pc++] = 0x11;
                bytecode_out[pc++] = (uint8_t)dst;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 4][1] - '0');
                idx += 5;
            } else if (idx + 2 < token_count && strcmp(tokens[idx + 1], "=") == 0 && pc + 2 < max_len) {
                // R0 = 10; -> SET_REG R0 10;
                int val = atoi(tokens[idx + 2]);
                bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = (uint8_t)dst;
                bytecode_out[pc++] = (uint8_t)val;
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "eval") == 0 || strcmp(t, "EVAL") == 0) {
            if (pc < max_len) {
                bytecode_out[pc++] = 0x03;
            }
            idx++;
        } else if (strcmp(t, "prune") == 0 || strcmp(t, "PRUNE") == 0) {
            if (idx + 1 < token_count && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x04;
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 1]);
                idx += 2;
            } else {
                idx++;
            }
        } else if (strcmp(t, "weight") == 0 || strcmp(t, "WEIGHT") == 0) {
            if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x05;
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 1]);
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 2]);
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "SET_REG") == 0) {
            // Support legacy: SET_REG R0 5;
            if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 2]);
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "ADD") == 0) {
            // Support COBOL style: ADD R1 TO R0;
            if (idx + 3 < token_count && strcmp(tokens[idx + 2], "TO") == 0 && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x10;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 3][1] - '0');
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                idx += 4;
            }
            // Support legacy: ADD R0 R1;
            else if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x10;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 2][1] - '0');
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "SUBTRACT") == 0) {
            // Support COBOL style: SUBTRACT R1 FROM R0;
            if (idx + 3 < token_count && strcmp(tokens[idx + 2], "FROM") == 0 && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x11;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 3][1] - '0');
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "SUB") == 0) {
            // Support legacy: SUB R0 R1;
            if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x11;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 2][1] - '0');
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "if") == 0) {
            // if (R0 == R1) jump(16); or if (R0 < R1) jump(16);
            if (idx + 5 < token_count && pc + 3 < max_len) {
                char *reg_a = tokens[idx + 1];
                char *op = tokens[idx + 2];
                char *reg_b = tokens[idx + 3];
                char *jmp = tokens[idx + 4];
                char *target = tokens[idx + 5];
                if (reg_a[0] == 'R' && reg_b[0] == 'R' && strcmp(jmp, "jump") == 0) {
                    if (strcmp(op, "==") == 0 || strcmp(op, "JEQ") == 0) {
                        bytecode_out[pc++] = 0x12;
                        bytecode_out[pc++] = (uint8_t)atoi(target);
                        bytecode_out[pc++] = (uint8_t)(reg_a[1] - '0');
                        bytecode_out[pc++] = (uint8_t)(reg_b[1] - '0');
                    } else if (strcmp(op, "<") == 0 || strcmp(op, "JLT") == 0) {
                        bytecode_out[pc++] = 0x13;
                        bytecode_out[pc++] = (uint8_t)atoi(target);
                        bytecode_out[pc++] = (uint8_t)(reg_a[1] - '0');
                        bytecode_out[pc++] = (uint8_t)(reg_b[1] - '0');
                    }
                }
                idx += 6;
            } else {
                idx++;
            }
        } else if (strcmp(t, "JEQ") == 0) {
            // Support legacy: JEQ 16 R0 R1;
            if (idx + 3 < token_count && pc + 3 < max_len) {
                bytecode_out[pc++] = 0x12;
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 1]);
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 2][1] - '0');
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 3][1] - '0');
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "JLT") == 0) {
            // Support legacy: JLT 16 R0 R1;
            if (idx + 3 < token_count && pc + 3 < max_len) {
                bytecode_out[pc++] = 0x13;
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 1]);
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 2][1] - '0');
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 3][1] - '0');
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "get_prio") == 0 || strcmp(t, "GET_PRIO") == 0) {
            if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x15;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 2]);
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "get_size") == 0 || strcmp(t, "GET_SIZE") == 0) {
            if (idx + 1 < token_count && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x16;
                bytecode_out[pc++] = (uint8_t)(tokens[idx + 1][1] - '0');
                idx += 2;
            } else {
                idx++;
            }
        } else if (strcmp(t, "loop_until_empty") == 0 || strcmp(t, "LOOP_UNTIL_EMPTY") == 0) {
            if (idx + 1 < token_count && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x17;
                bytecode_out[pc++] = (uint8_t)atoi(tokens[idx + 1]);
                idx += 2;
            } else {
                idx++;
            }
        } else if (strcmp(t, "get_logic") == 0 || strcmp(t, "GET_LOGIC") == 0) {
            if (idx + 2 < token_count) {
                char *key = tokens[idx + 1];
                char *reg = tokens[idx + 2];
                int key_len = strlen(key);
                if (reg[0] == 'R' && reg[1] >= '0' && reg[1] <= '3' && pc + 2 + key_len <= max_len) {
                    bytecode_out[pc++] = 0x18; // OP_GET_LOGIC
                    bytecode_out[pc++] = (uint8_t)(reg[1] - '0');
                    bytecode_out[pc++] = (uint8_t)key_len;
                    memcpy(&bytecode_out[pc], key, key_len);
                    pc += key_len;
                }
                idx += 3;
            } else {
                idx++;
            }
        } else {
            idx++;
        }
    }

    *len_out = pc;
    return 0;
}

void tsfi_strategy_vm_broadcast(TSFiStrategyVM *vm, TSFiPriorityQueue *pq, TSFiStrategyTelemetry *telemetry) {
    if (!vm || !telemetry) return;
    for (int i = 0; i < 4; i++) {
        telemetry->registers[i] = vm->registers[i];
    }
    telemetry->queue_size = pq ? pq->size : 0;
}

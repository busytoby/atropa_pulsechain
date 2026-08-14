#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tsfi_strategy_lang.h"
#include "tsfi_anvil_vm.h"
#include "tsfi_ramac_layout.h"

void tsfi_strategy_vm_init(TSFiStrategyVM *vm) {
    if (!vm) return;
    vm->depth_priority_scale = 1;
    vm->abductive_priority_scale = 1;
    vm->executed_evals = 0;
    memset(vm->registers, 0, sizeof(vm->registers));
    memset(vm->call_stack, 0, sizeof(vm->call_stack));
    vm->stack_pointer = 0;
    memset(vm->eval_stack, 0, sizeof(vm->eval_stack));
    vm->eval_stack_ptr = 0;
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
                if (dst < 16 && src < 16) {
                    vm->registers[dst] += vm->registers[src];
                }
            }
        } else if (op == 0x11) { // SUB dst src
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16) {
                    vm->registers[dst] -= vm->registers[src];
                }
            }
        } else if (op == 0x12) { // JEQ target_pc dst src
            if (pc + 2 < len) {
                uint8_t target_pc = bytecode[pc++];
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16 && vm->registers[dst] == vm->registers[src]) {
                    if (target_pc <= len) {
                        pc = target_pc;
                    }
                }
            }
        } else if (op == 0x13) { // JLT target_pc dst src
            if (pc + 2 < len) {
                uint8_t target_pc = bytecode[pc++];
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16 && vm->registers[dst] < vm->registers[src]) {
                    if (target_pc <= len) {
                        pc = target_pc;
                    }
                }
            }
        } else if (op == 0x14) { // SET_REG reg val (int32)
            if (pc + 4 < len) {
                uint8_t reg = bytecode[pc++];
                int32_t val = (int32_t)((uint32_t)bytecode[pc] | ((uint32_t)bytecode[pc+1] << 8) | ((uint32_t)bytecode[pc+2] << 16) | ((uint32_t)bytecode[pc+3] << 24));
                pc += 4;
                if (reg < 16) {
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
        } else if (op == 0x19) { // COPY_REG dst src
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16) {
                    vm->registers[dst] = vm->registers[src];
                }
            }
        } else if (op == 0x1A) { // JUMP target_pc
            if (pc < len) {
                uint8_t target_pc = bytecode[pc++];
                if (target_pc <= len) {
                    pc = target_pc;
                }
            }
        } else if (op == 0x1B) { // JNE target_pc dst src
            if (pc + 2 < len) {
                uint8_t target_pc = bytecode[pc++];
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16 && vm->registers[dst] != vm->registers[src]) {
                    if (target_pc <= len) {
                        pc = target_pc;
                    }
                }
            }
        } else if (op == 0x1C) { // JGE target_pc dst src
            if (pc + 2 < len) {
                uint8_t target_pc = bytecode[pc++];
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16 && vm->registers[dst] >= vm->registers[src]) {
                    if (target_pc <= len) {
                        pc = target_pc;
                    }
                }
            }
        } else if (op == 0x1D) { // MUL dst src
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16) {
                    vm->registers[dst] *= vm->registers[src];
                }
            }
        } else if (op == 0x1E) { // DIV dst src
            if (pc + 1 < len) {
                uint8_t dst = bytecode[pc++];
                uint8_t src = bytecode[pc++];
                if (dst < 16 && src < 16 && vm->registers[src] != 0) {
                    vm->registers[dst] /= vm->registers[src];
                }
            }
        } else if (op == 0x1F) { // CALL target_pc
            if (pc < len) {
                uint8_t target_pc = bytecode[pc++];
                if (vm->stack_pointer < 8) {
                    vm->call_stack[vm->stack_pointer++] = pc;
                    pc = target_pc;
                }
            }
        } else if (op == 0x20) { // EXIT
            if (vm->stack_pointer > 0) {
                pc = vm->call_stack[--vm->stack_pointer];
            }
        } else if (op == 0x21) { // DISPLAY
            if (pc < len) {
                uint8_t num_items = bytecode[pc++];
                for (uint8_t i = 0; i < num_items; i++) {
                    if (pc < len) {
                        uint8_t type = bytecode[pc++];
                        if (type == 0) {
                            if (pc < len) {
                                uint8_t str_len = bytecode[pc++];
                                if (pc + str_len <= len) {
                                    for (uint8_t j = 0; j < str_len; j++) {
                                        putchar(bytecode[pc + j]);
                                    }
                                    pc += str_len;
                                }
                            }
                        } else if (type == 1) {
                            if (pc < len) {
                                uint8_t reg = bytecode[pc++];
                                if (reg < 4) {
                                    printf("%d", vm->registers[reg]);
                                }
                            }
                        } else if (type == 2) {
                            printf("%d", vm->depth_priority_scale);
                        } else if (type == 3) {
                            printf("%d", vm->abductive_priority_scale);
                        }
                    }
                }
                putchar('\n');
                fflush(stdout);
            }
        } else if (op == 0x22) {
            if (pc < len) {
                uint8_t mode = bytecode[pc++];
                if (mode == 0) {
                    if (pc + 2 < len) {
                        uint8_t target_reg = bytecode[pc++];
                        uint8_t counter_reg = bytecode[pc++];
                        uint8_t digit = bytecode[pc++];
                        if (target_reg < 4 && counter_reg < 4) {
                            char buf[32];
                            snprintf(buf, sizeof(buf), "%d", vm->registers[target_reg]);
                            int count = 0;
                            char target_char = '0' + digit;
                            for (int k = 0; buf[k] != '\0'; k++) {
                                if (buf[k] == target_char) count++;
                            }
                            vm->registers[counter_reg] += count;
                        }
                    }
                } else if (mode == 1) {
                    if (pc + 2 < len) {
                        uint8_t target_reg = bytecode[pc++];
                        uint8_t old_digit = bytecode[pc++];
                        uint8_t new_digit = bytecode[pc++];
                        if (target_reg < 4) {
                            char buf[32];
                            snprintf(buf, sizeof(buf), "%d", vm->registers[target_reg]);
                            char old_char = '0' + old_digit;
                            char new_char = '0' + new_digit;
                            for (int k = 0; buf[k] != '\0'; k++) {
                                if (buf[k] == old_char) buf[k] = new_char;
                            }
                            vm->registers[target_reg] = atoi(buf);
                        }
                    }
                }
            }
        } else if (op == 0x23) {
            if (pc + 1 < len) {
                uint8_t dst_reg = bytecode[pc++];
                uint8_t num_items = bytecode[pc++];
                char concat_buf[128] = "";
                int write_offset = 0;
                for (uint8_t i = 0; i < num_items; i++) {
                    if (pc < len) {
                        uint8_t type = bytecode[pc++];
                        if (type == 0) {
                            if (pc < len) {
                                uint8_t str_len = bytecode[pc++];
                                if (pc + str_len <= len) {
                                    if (write_offset + str_len < 127) {
                                        memcpy(&concat_buf[write_offset], &bytecode[pc], str_len);
                                        write_offset += str_len;
                                        concat_buf[write_offset] = '\0';
                                    }
                                    pc += str_len;
                                }
                            }
                        } else if (type == 1) {
                            if (pc < len) {
                                uint8_t reg = bytecode[pc++];
                                if (reg < 4) {
                                    int written = snprintf(&concat_buf[write_offset], 127 - write_offset, "%d", vm->registers[reg]);
                                    if (written > 0) write_offset += written;
                                }
                            }
                        } else if (type == 2) {
                            int written = snprintf(&concat_buf[write_offset], 127 - write_offset, "%d", vm->depth_priority_scale);
                            if (written > 0) write_offset += written;
                        } else if (type == 3) {
                            int written = snprintf(&concat_buf[write_offset], 127 - write_offset, "%d", vm->abductive_priority_scale);
                            if (written > 0) write_offset += written;
                        }
                    }
                }
                if (dst_reg < 4) {
                    vm->registers[dst_reg] = atoi(concat_buf);
                }
            }
        } else if (op == 0x30) { // PUSH_REG reg
            if (pc < len) {
                uint8_t reg = bytecode[pc++];
                if (reg < 4 && vm->eval_stack_ptr < 32) {
                    vm->eval_stack[vm->eval_stack_ptr++] = vm->registers[reg];
                }
            }
        } else if (op == 0x31) { // POP_REG reg
            if (pc < len) {
                uint8_t reg = bytecode[pc++];
                if (reg < 4 && vm->eval_stack_ptr > 0) {
                    vm->registers[reg] = vm->eval_stack[--vm->eval_stack_ptr];
                }
            }
        } else if (op == 0x32) { // STACK_ADD
            if (vm->eval_stack_ptr >= 2) {
                int b = vm->eval_stack[--vm->eval_stack_ptr];
                int a = vm->eval_stack[--vm->eval_stack_ptr];
                vm->eval_stack[vm->eval_stack_ptr++] = a + b;
            }
        } else if (op == 0x33) { // STACK_SUB
            if (vm->eval_stack_ptr >= 2) {
                int b = vm->eval_stack[--vm->eval_stack_ptr];
                int a = vm->eval_stack[--vm->eval_stack_ptr];
                vm->eval_stack[vm->eval_stack_ptr++] = a - b;
            }
        } else if (op == 0x34) { // PUSH_LIT val
            if (pc < len) {
                uint8_t val = bytecode[pc++];
                if (vm->eval_stack_ptr < 32) {
                    vm->eval_stack[vm->eval_stack_ptr++] = val;
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

    if (script[0] == '^') {
        int pc = 0;
        const char *p = script;
        const char *sd = strstr(p, "set-depth");
        if (sd) {
            sd += 9;
            while (*sd && !isdigit((unsigned char)*sd)) sd++;
            if (pc + 1 < max_len) {
                bytecode_out[pc++] = 0x01;
                bytecode_out[pc++] = (uint8_t)atoi(sd);
            }
        }
        const char *rw = p;
        while ((rw = strstr(rw, "wmq-reg-write")) != NULL) {
            rw += 13;
            while (*rw && isspace((unsigned char)*rw)) rw++;
            int reg = atoi(rw);
            while (*rw && isdigit((unsigned char)*rw)) rw++;
            while (*rw && isspace((unsigned char)*rw)) rw++;
            int val = atoi(rw);
            if (pc + 2 < max_len) {
                bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = (uint8_t)reg;
                { int32_t _v = (int32_t)(val);
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
            }
            rw++;
        }
        const char *ie = strstr(p, "if-eq");
        if (ie) {
            ie += 5;
            while (*ie && isspace((unsigned char)*ie)) ie++;
            int reg_a = atoi(ie);
            while (*ie && isdigit((unsigned char)*ie)) ie++;
            while (*ie && isspace((unsigned char)*ie)) ie++;
            int reg_b = atoi(ie);
            while (*ie && isdigit((unsigned char)*ie)) ie++;
            while (*ie && isspace((unsigned char)*ie)) ie++;
            int target = atoi(ie);
            if (pc + 3 < max_len) {
                bytecode_out[pc++] = 0x18;
                bytecode_out[pc++] = (uint8_t)target;
                bytecode_out[pc++] = (uint8_t)reg_a;
                bytecode_out[pc++] = (uint8_t)reg_b;
            }
        }
        const char *wt = p;
        while ((wt = strstr(wt, "weight")) != NULL) {
            if (wt > p && *(wt - 1) == '-') {
                wt += 6;
                continue;
            }
            wt += 6;
            while (*wt && isspace((unsigned char)*wt)) wt++;
            int key = atoi(wt);
            while (*wt && isdigit((unsigned char)*wt)) wt++;
            while (*wt && isspace((unsigned char)*wt)) wt++;
            int val = atoi(wt);
            if (pc + 2 < max_len) {
                bytecode_out[pc++] = 0x05;
                bytecode_out[pc++] = (uint8_t)key;
                bytecode_out[pc++] = (uint8_t)val;
            }
            wt++;
        }
        if (strstr(p, "eval")) {
            if (pc < max_len) {
                bytecode_out[pc++] = 0x03;
            }
        }
        const char *pr = strstr(p, "prune");
        if (pr) {
            pr += 5;
            while (*pr && !isdigit((unsigned char)*pr)) pr++;
            if (pc + 1 < max_len) {
                bytecode_out[pc++] = 0x04;
                bytecode_out[pc++] = (uint8_t)atoi(pr);
            }
        }
        *len_out = pc;
        return 0;
    }

    char script_copy[8192];
    strncpy(script_copy, script, 8191);
    script_copy[8191] = '\0';

    // Delimiters include whitespace, periods, parens, braces, commas, quotes, and semicolons
    const char *delims = " ;(),{}\"\n\r\t.";
    
    // Store tokens in an array for multi-token lookahead parsing
    char *tokens[1024];
    int token_count = 0;
    
    char *tok = strtok(script_copy, delims);
    while (tok && token_count < 1024) {
        tokens[token_count++] = tok;
        tok = strtok(NULL, delims);
    }

    typedef struct {
        char var_name[32];
        char reg_name[8];
    } StrategyVarMap;

    StrategyVarMap var_maps[16];
    int var_map_count = 0;

    int idx = 0;
    // Skip IDENTIFICATION DIVISION if present
    if (token_count > 1 && (strcmp(tokens[0], "IDENTIFICATION") == 0 || strcmp(tokens[0], "identification") == 0) &&
        (strcmp(tokens[1], "DIVISION") == 0 || strcmp(tokens[1], "division") == 0)) {
        idx = 2;
        while (idx < token_count) {
            if ((strcmp(tokens[idx], "DATA") == 0 || strcmp(tokens[idx], "data") == 0) &&
                idx + 1 < token_count && (strcmp(tokens[idx + 1], "DIVISION") == 0 || strcmp(tokens[idx + 1], "division") == 0)) {
                break;
            }
            if ((strcmp(tokens[idx], "PROCEDURE") == 0 || strcmp(tokens[idx], "procedure") == 0) &&
                idx + 1 < token_count && (strcmp(tokens[idx + 1], "DIVISION") == 0 || strcmp(tokens[idx + 1], "division") == 0)) {
                idx += 2;
                break;
            }
            idx++;
        }
    }

    if (idx + 1 < token_count && (strcmp(tokens[idx], "DATA") == 0 || strcmp(tokens[idx], "data") == 0) &&
        (strcmp(tokens[idx + 1], "DIVISION") == 0 || strcmp(tokens[idx + 1], "division") == 0)) {
        idx += 2;
        while (idx < token_count) {
            if (strcmp(tokens[idx], "PROCEDURE") == 0 || strcmp(tokens[idx], "procedure") == 0) {
                if (idx + 1 < token_count && (strcmp(tokens[idx + 1], "DIVISION") == 0 || strcmp(tokens[idx + 1], "division") == 0)) {
                    idx += 2;
                    break;
                }
            }
            if (strcmp(tokens[idx], "01") == 0 || strcmp(tokens[idx], "77") == 0) {
                if (idx + 1 < token_count) {
                    char *var_name = tokens[idx + 1];
                    char *mapped_reg = NULL;
                    for (int k = idx + 2; k < token_count; k++) {
                        if (strcmp(tokens[k], "01") == 0 || strcmp(tokens[k], "77") == 0 ||
                            strcmp(tokens[k], "PROCEDURE") == 0 || strcmp(tokens[k], "procedure") == 0) {
                            break;
                        }
                        if (tokens[k][0] == 'R' && isdigit((unsigned char)tokens[k][1])) {
                            mapped_reg = tokens[k];
                            break;
                        }
                    }
                    if (mapped_reg && var_map_count < 16) {
                        strncpy(var_maps[var_map_count].var_name, var_name, 31);
                        var_maps[var_map_count].var_name[31] = '\0';
                        strncpy(var_maps[var_map_count].reg_name, mapped_reg, 7);
                        var_maps[var_map_count].reg_name[7] = '\0';
                        var_map_count++;
                    }
                }
            }
            idx++;
        }
    }

    // Resolve all variable tokens in the procedure division
    for (int i = idx; i < token_count; i++) {
        if (strcmp(tokens[i], "DBTG-CRU") == 0) {
            tokens[i] = "R0";
        } else if (strcmp(tokens[i], "DBTG-REALM-LOCK") == 0) {
            tokens[i] = "R1";
        } else {
            for (int m = 0; m < var_map_count; m++) {
                if (strcmp(tokens[i], var_maps[m].var_name) == 0) {
                    tokens[i] = var_maps[m].reg_name;
                    break;
                }
            }
        }
    }

    int pc = 0;
    int loop_start_pc = -1;
    int loop_exit_placeholder_pc = -1;
    int if_stack[16];
    int if_stack_ptr = 0;
    int else_stack[16];
    int else_stack_ptr = 0;

    typedef struct {
        char name[32];
        int pc;
    } StrategyLabel;

    typedef struct {
        char label_name[32];
        int placeholder_pc;
    } StrategyCallFixup;

    StrategyLabel labels[16];
    int label_count = 0;
    StrategyCallFixup call_fixups[16];
    int call_fixup_count = 0;
    while (idx < token_count) {
        char *t = tokens[idx];
        if ((strcmp(t, "IF") == 0 || strcmp(t, "if") == 0) &&
            !(idx + 4 < token_count && strcmp(tokens[idx + 4], "jump") == 0)) {
            if (idx + 3 < token_count) {
                char *reg_a = tokens[idx + 1];
                char *op = tokens[idx + 2];
                char *reg_b = tokens[idx + 3];
                int ra = (reg_a[0] == 'R' && isdigit((unsigned char)reg_a[1])) ? atoi(&reg_a[1]) : -1;
                int rb = -1;
                if (reg_b[0] == 'R' && isdigit((unsigned char)reg_b[1])) {
                    rb = atoi(&reg_b[1]);
                } else if (isdigit((unsigned char)reg_b[0])) {
                    // Load literal into temporary register R15
                    bytecode_out[pc++] = 0x14; // SET_REG R15 val
                    bytecode_out[pc++] = 15;
                    { int32_t _v = (int32_t)atoi(reg_b);
                      bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                      bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                      bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                      bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                    rb = 15;
                }
                if (ra >= 0 && rb >= 0) {
                    int placeholder = -1;
                    if (strcmp(op, "==") == 0 || strcmp(op, "JEQ") == 0) {
                        bytecode_out[pc++] = 0x1B; // JNE (jump if not equal, i.e. skip IF block)
                        placeholder = pc++;
                        bytecode_out[placeholder] = 0;
                        bytecode_out[pc++] = (uint8_t)ra;
                        bytecode_out[pc++] = (uint8_t)rb;
                    } else if (strcmp(op, "<") == 0 || strcmp(op, "JLT") == 0) {
                        bytecode_out[pc++] = 0x1C; // JGE (jump if greater/equal, i.e. skip IF block)
                        placeholder = pc++;
                        bytecode_out[placeholder] = 0;
                        bytecode_out[pc++] = (uint8_t)ra;
                        bytecode_out[pc++] = (uint8_t)rb;
                    } else if (strcmp(op, ">") == 0 || strcmp(op, "JGT") == 0) {
                        bytecode_out[pc++] = 0x1C; // JGE target rb ra
                        placeholder = pc++;
                        bytecode_out[placeholder] = 0;
                        bytecode_out[pc++] = (uint8_t)rb;
                        bytecode_out[pc++] = (uint8_t)ra;
                    } else if (strcmp(op, ">=") == 0 || strcmp(op, "JGE") == 0) {
                        bytecode_out[pc++] = 0x13; // JLT target ra rb
                        placeholder = pc++;
                        bytecode_out[placeholder] = 0;
                        bytecode_out[pc++] = (uint8_t)ra;
                        bytecode_out[pc++] = (uint8_t)rb;
                    } else if (strcmp(op, "<=") == 0 || strcmp(op, "JLE") == 0) {
                        bytecode_out[pc++] = 0x13; // JLT target rb ra
                        placeholder = pc++;
                        bytecode_out[placeholder] = 0;
                        bytecode_out[pc++] = (uint8_t)rb;
                        bytecode_out[pc++] = (uint8_t)ra;
                    }
                    if (placeholder != -1 && if_stack_ptr < 16) {
                        if_stack[if_stack_ptr++] = placeholder;
                    }
                }
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "ELSE") == 0 || strcmp(t, "else") == 0) {
            if (if_stack_ptr > 0 && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x1A; // JUMP past ELSE block
                int else_placeholder = pc++;
                bytecode_out[else_placeholder] = 0;
                if (else_stack_ptr < 16) {
                    else_stack[else_stack_ptr++] = else_placeholder;
                }
                int if_placeholder = if_stack[--if_stack_ptr];
                bytecode_out[if_placeholder] = (uint8_t)pc;
            }
            idx++;
        } else if (strcmp(t, "END-IF") == 0 || strcmp(t, "end-if") == 0 || strcmp(t, "END_IF") == 0) {
            if (if_stack_ptr > 0) {
                int if_placeholder = if_stack[--if_stack_ptr];
                bytecode_out[if_placeholder] = (uint8_t)pc;
            }
            if (else_stack_ptr > 0) {
                int else_placeholder = else_stack[--else_stack_ptr];
                bytecode_out[else_placeholder] = (uint8_t)pc;
            }
            idx++;
        } else if (strcmp(t, "GO") == 0 || strcmp(t, "go") == 0) {
            if (idx + 2 < token_count && (strcmp(tokens[idx + 1], "TO") == 0 || strcmp(tokens[idx + 1], "to") == 0)) {
                if (call_fixup_count < 16 && pc + 1 < max_len) {
                    strncpy(call_fixups[call_fixup_count].label_name, tokens[idx + 2], 31);
                    call_fixups[call_fixup_count].label_name[31] = '\0';
                    call_fixups[call_fixup_count].placeholder_pc = pc + 1;
                    call_fixup_count++;
                    bytecode_out[pc++] = 0x1A; // JUMP
                    bytecode_out[pc++] = 0; // placeholder
                }
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "DISPLAY") == 0 || strcmp(t, "display") == 0) {
            int start_idx = idx + 1;
            int items_count = 0;
            int end_idx = start_idx;
            const char *kws[] = {
                "MOVE", "move", "ADD", "add", "SUBTRACT", "subtract",
                "MULTIPLY", "multiply", "DIVIDE", "divide",
                "PERFORM", "perform", "END-PERFORM", "end-perform", "END_PERFORM",
                "IF", "if", "ELSE", "else", "END-IF", "end-if", "END_IF",
                "EXIT", "exit", "EXIT-PROGRAM", "exit-program", "EXIT_PROGRAM",
                "EVAL", "eval", "PRUNE", "prune", "WEIGHT", "weight", "SET_REG",
                "GET_PRIO", "get_prio", "GET_SIZE", "get_size", "LOOP_UNTIL_EMPTY", "loop_until_empty",
                "GET_LOGIC", "get_logic", "SET", "GO", "go", "DISPLAY", "display"
            };
            while (end_idx < token_count) {
                char *item_tok = tokens[end_idx];
                int is_end = 0;
                for (size_t i = 0; i < sizeof(kws)/sizeof(kws[0]); i++) {
                    if (strcmp(item_tok, kws[i]) == 0) {
                        is_end = 1;
                        break;
                    }
                }
                if (is_end) break;
                
                int is_label = 0;
                for (int l = 0; l < label_count; l++) {
                    if (strcmp(item_tok, labels[l].name) == 0) {
                        is_label = 1;
                        break;
                    }
                }
                if (is_label) break;

                items_count++;
                end_idx++;
            }
            
            if (items_count > 0 && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x21;
                bytecode_out[pc++] = (uint8_t)items_count;
                for (int k = start_idx; k < end_idx; k++) {
                    char *item_tok = tokens[k];
                    if (item_tok[0] == 'R' && item_tok[1] >= '0' && item_tok[1] <= '3' && item_tok[2] == '\0') {
                        if (pc + 1 < max_len) {
                            bytecode_out[pc++] = 1;
                            bytecode_out[pc++] = (uint8_t)(item_tok[1] - '0');
                        }
                    } else if (strcmp(item_tok, "depth") == 0) {
                        if (pc < max_len) {
                            bytecode_out[pc++] = 2;
                        }
                    } else if (strcmp(item_tok, "abductive") == 0) {
                        if (pc < max_len) {
                            bytecode_out[pc++] = 3;
                        }
                    } else {
                        int slen = strlen(item_tok);
                        if (pc + 2 + slen < max_len) {
                            bytecode_out[pc++] = 0;
                            bytecode_out[pc++] = (uint8_t)(slen + 1);
                            memcpy(&bytecode_out[pc], item_tok, slen);
                            bytecode_out[pc + slen] = ' ';
                            pc += slen + 1;
                        }
                    }
                }
            }
            idx = end_idx;
        } else if (strcmp(t, "INSPECT") == 0 || strcmp(t, "inspect") == 0) {
            if (idx + 5 < token_count && pc + 4 < max_len) {
                char *target_reg = tokens[idx + 1];
                char *action = tokens[idx + 2];
                if (target_reg[0] == 'R' && target_reg[1] >= '0' && target_reg[1] <= '3' && target_reg[2] == '\0') {
                    if (strcmp(action, "REPLACING") == 0 || strcmp(action, "replacing") == 0) {
                        char *old_digit = NULL;
                        char *new_digit = NULL;
                        int advanced = 0;
                        if (strcmp(tokens[idx + 3], "ALL") == 0 || strcmp(tokens[idx + 3], "all") == 0) {
                            if (idx + 6 < token_count) {
                                old_digit = tokens[idx + 4];
                                new_digit = tokens[idx + 6];
                                advanced = 7;
                            }
                        } else {
                            old_digit = tokens[idx + 3];
                            new_digit = tokens[idx + 5];
                            advanced = 6;
                        }
                        if (old_digit && new_digit) {
                            bytecode_out[pc++] = 0x22;
                            bytecode_out[pc++] = 1; // REPLACING
                            bytecode_out[pc++] = (uint8_t)(target_reg[1] - '0');
                            bytecode_out[pc++] = (uint8_t)(old_digit[0] - '0');
                            bytecode_out[pc++] = (uint8_t)(new_digit[0] - '0');
                            idx += advanced;
                        } else {
                            idx++;
                        }
                    } else if (strcmp(action, "TALLYING") == 0 || strcmp(action, "tallying") == 0) {
                        char *counter_reg = tokens[idx + 3];
                        char *digit = NULL;
                        int advanced = 0;
                        if (counter_reg[0] == 'R' && counter_reg[1] >= '0' && counter_reg[1] <= '3' && counter_reg[2] == '\0') {
                            if (strcmp(tokens[idx + 5], "ALL") == 0 || strcmp(tokens[idx + 5], "all") == 0) {
                                if (idx + 6 < token_count) {
                                    digit = tokens[idx + 6];
                                    advanced = 7;
                                }
                            } else {
                                digit = tokens[idx + 5];
                                advanced = 6;
                            }
                            if (digit) {
                                bytecode_out[pc++] = 0x22;
                                bytecode_out[pc++] = 0; // TALLYING
                                bytecode_out[pc++] = (uint8_t)(target_reg[1] - '0');
                                bytecode_out[pc++] = (uint8_t)(counter_reg[1] - '0');
                                bytecode_out[pc++] = (uint8_t)(digit[0] - '0');
                                idx += advanced;
                            } else {
                                idx++;
                            }
                        } else {
                            idx++;
                        }
                    } else {
                        idx++;
                    }
                } else {
                    idx++;
                }
            } else {
                idx++;
            }
        } else if (strcmp(t, "STRING") == 0 || strcmp(t, "string") == 0) {
            int into_idx = idx + 1;
            while (into_idx < token_count && strcmp(tokens[into_idx], "INTO") != 0 && strcmp(tokens[into_idx], "into") != 0) {
                into_idx++;
            }
            if (into_idx + 1 < token_count) {
                char *dst_reg = tokens[into_idx + 1];
                int items_count = into_idx - (idx + 1);
                if (items_count > 0 && dst_reg[0] == 'R' && dst_reg[1] >= '0' && dst_reg[1] <= '3' && dst_reg[2] == '\0' && pc + 2 < max_len) {
                    bytecode_out[pc++] = 0x23;
                    bytecode_out[pc++] = (uint8_t)(dst_reg[1] - '0');
                    bytecode_out[pc++] = (uint8_t)items_count;
                    for (int k = idx + 1; k < into_idx; k++) {
                        char *item_tok = tokens[k];
                        if (item_tok[0] == 'R' && item_tok[1] >= '0' && item_tok[1] <= '3' && item_tok[2] == '\0') {
                            if (pc + 1 < max_len) {
                                bytecode_out[pc++] = 1;
                                bytecode_out[pc++] = (uint8_t)(item_tok[1] - '0');
                            }
                        } else if (strcmp(item_tok, "depth") == 0) {
                            if (pc < max_len) {
                                bytecode_out[pc++] = 2;
                            }
                        } else if (strcmp(item_tok, "abductive") == 0) {
                            if (pc < max_len) {
                                bytecode_out[pc++] = 3;
                            }
                        } else {
                            int slen = strlen(item_tok);
                            if (pc + 2 + slen < max_len) {
                                bytecode_out[pc++] = 0;
                                bytecode_out[pc++] = (uint8_t)slen;
                                memcpy(&bytecode_out[pc], item_tok, slen);
                                pc += slen;
                            }
                        }
                    }
                    idx = into_idx + 2;
                } else {
                    idx++;
                }
            } else {
                idx++;
            }
        } else if (strcmp(t, "PERFORM") == 0 || strcmp(t, "perform") == 0) {
            if (idx + 5 < token_count && (strcmp(tokens[idx + 1], "UNTIL") == 0 || strcmp(tokens[idx + 1], "until") == 0)) {
                char *reg_a = tokens[idx + 2];
                char *op = tokens[idx + 3];
                char *reg_b = tokens[idx + 4];
                if (reg_a[0] == 'R' && reg_b[0] == 'R') {
                    loop_start_pc = pc;
                    if (strcmp(op, "==") == 0 || strcmp(op, "JEQ") == 0) {
                        bytecode_out[pc++] = 0x12;
                        loop_exit_placeholder_pc = pc;
                        bytecode_out[pc++] = 0;
                        bytecode_out[pc++] = (uint8_t)(reg_a[1] - '0');
                        bytecode_out[pc++] = (uint8_t)(reg_b[1] - '0');
                    } else if (strcmp(op, "<") == 0 || strcmp(op, "JLT") == 0) {
                        bytecode_out[pc++] = 0x13;
                        loop_exit_placeholder_pc = pc;
                        bytecode_out[pc++] = 0;
                        bytecode_out[pc++] = (uint8_t)(reg_a[1] - '0');
                        bytecode_out[pc++] = (uint8_t)(reg_b[1] - '0');
                    }
                }
                idx += 5;
            } else if (idx + 1 < token_count) {
                if (call_fixup_count < 16 && pc + 1 < max_len) {
                    strncpy(call_fixups[call_fixup_count].label_name, tokens[idx + 1], 31);
                    call_fixups[call_fixup_count].label_name[31] = '\0';
                    call_fixups[call_fixup_count].placeholder_pc = pc + 1;
                    call_fixup_count++;
                    bytecode_out[pc++] = 0x1F;
                    bytecode_out[pc++] = 0;
                }
                idx += 2;
            } else {
                idx++;
            }
        } else if (strcmp(t, "END-PERFORM") == 0 || strcmp(t, "end-perform") == 0 || strcmp(t, "END_PERFORM") == 0) {
            if (loop_start_pc != -1 && loop_exit_placeholder_pc != -1 && pc + 1 < max_len) {
                bytecode_out[pc++] = 0x1A;
                bytecode_out[pc++] = (uint8_t)loop_start_pc;
                bytecode_out[loop_exit_placeholder_pc] = (uint8_t)pc;
            }
            idx++;
        } else if (strcmp(t, "EXIT") == 0 || strcmp(t, "exit") == 0 ||
                   strcmp(t, "EXIT-PROGRAM") == 0 || strcmp(t, "exit-program") == 0 ||
                   strcmp(t, "EXIT_PROGRAM") == 0) {
            if (pc < max_len) {
                bytecode_out[pc++] = 0x20;
            }
            idx++;
        } else if (strcmp(t, "SET") == 0) {
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
                { int32_t _v = (int32_t)(val);
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
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
        } else if (strcmp(t, "MOVE") == 0 || strcmp(t, "move") == 0) {
            if (idx + 3 < token_count && (strcmp(tokens[idx + 2], "TO") == 0 || strcmp(tokens[idx + 2], "to") == 0) && pc + 2 < max_len) {
                char *src = tokens[idx + 1];
                char *dst = tokens[idx + 3];
                int dst_reg = (dst[0] == 'R' && isdigit((unsigned char)dst[1])) ? atoi(&dst[1]) : -1;
                if (dst_reg >= 0 && dst_reg < 16) {
                    if (src[0] == 'R' && isdigit((unsigned char)src[1])) {
                        bytecode_out[pc++] = 0x19;
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)atoi(&src[1]);
                    } else {
                        bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = (uint8_t)dst_reg;
                { int32_t _v = (int32_t)(atoi(src));
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                    }
                }
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "SET_REG") == 0) {
            // Support legacy: SET_REG R0 5;
            if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = (uint8_t)atoi(&tokens[idx + 1][1]);
                { int32_t _v = (int32_t)(atoi(tokens[idx + 2]));
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "ADD") == 0) {
            // Support COBOL style: ADD R1 TO R0; or ADD 12 TO R0;
            if (idx + 3 < token_count && strcmp(tokens[idx + 2], "TO") == 0 && pc + 2 < max_len) {
                char *src = tokens[idx + 1];
                char *dst = tokens[idx + 3];
                int dst_reg = (dst[0] == 'R' && isdigit((unsigned char)dst[1])) ? atoi(&dst[1]) : -1;
                if (dst_reg >= 0 && dst_reg < 16) {
                    if (src[0] == 'R' && isdigit((unsigned char)src[1])) {
                        bytecode_out[pc++] = 0x10;
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)atoi(&src[1]);
                    } else {
                        bytecode_out[pc++] = 0x14; // SET_REG R15 val
                        bytecode_out[pc++] = 15;
                        { int32_t _v = (int32_t)(atoi(src));
                          bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                        bytecode_out[pc++] = 0x10; // ADD dst R15
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = 15;
                    }
                }
                idx += 4;
            }
            // Support legacy: ADD R0 R1;
            else if (idx + 2 < token_count && pc + 2 < max_len) {
                bytecode_out[pc++] = 0x10;
                bytecode_out[pc++] = (uint8_t)atoi(&tokens[idx + 1][1]);
                bytecode_out[pc++] = (uint8_t)atoi(&tokens[idx + 2][1]);
                idx += 3;
            } else {
                idx++;
            }
        } else if (strcmp(t, "SUBTRACT") == 0) {
            // Support COBOL style: SUBTRACT R1 FROM R0; or SUBTRACT 10 FROM R0;
            if (idx + 3 < token_count && strcmp(tokens[idx + 2], "FROM") == 0 && pc + 2 < max_len) {
                char *src = tokens[idx + 1];
                char *dst = tokens[idx + 3];
                int dst_reg = (dst[0] == 'R' && isdigit((unsigned char)dst[1])) ? atoi(&dst[1]) : -1;
                if (dst_reg >= 0 && dst_reg < 16) {
                    if (src[0] == 'R' && isdigit((unsigned char)src[1])) {
                        bytecode_out[pc++] = 0x11;
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)atoi(&src[1]);
                    } else {
                        bytecode_out[pc++] = 0x14; // SET_REG R15 val
                        bytecode_out[pc++] = 15;
                        { int32_t _v = (int32_t)(atoi(src));
                          bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                        bytecode_out[pc++] = 0x11; // SUB dst R15
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = 15;
                    }
                }
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "MULTIPLY") == 0 || strcmp(t, "multiply") == 0) {
            if (idx + 3 < token_count && (strcmp(tokens[idx + 2], "BY") == 0 || strcmp(tokens[idx + 2], "by") == 0) && pc + 2 < max_len) {
                char *src = tokens[idx + 1];
                char *dst = tokens[idx + 3];
                int dst_reg = (dst[0] == 'R' && isdigit((unsigned char)dst[1])) ? atoi(&dst[1]) : -1;
                if (dst_reg >= 0 && dst_reg < 16) {
                    if (src[0] == 'R' && isdigit((unsigned char)src[1])) {
                        bytecode_out[pc++] = 0x1D;
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)atoi(&src[1]);
                    } else {
                        bytecode_out[pc++] = 0x14; // SET_REG R15 val
                        bytecode_out[pc++] = 15;
                        { int32_t _v = (int32_t)(atoi(src));
                          bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                        bytecode_out[pc++] = 0x1D; // MUL dst R15
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = 15;
                    }
                }
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "DIVIDE") == 0 || strcmp(t, "divide") == 0) {
            if (idx + 3 < token_count && (strcmp(tokens[idx + 2], "INTO") == 0 || strcmp(tokens[idx + 2], "into") == 0) && pc + 2 < max_len) {
                char *src = tokens[idx + 1];
                char *dst = tokens[idx + 3];
                int dst_reg = (dst[0] == 'R' && isdigit((unsigned char)dst[1])) ? atoi(&dst[1]) : -1;
                if (dst_reg >= 0 && dst_reg < 16) {
                    if (src[0] == 'R' && isdigit((unsigned char)src[1])) {
                        bytecode_out[pc++] = 0x1E;
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)atoi(&src[1]);
                    } else {
                        bytecode_out[pc++] = 0x14; // SET_REG R15 val
                        bytecode_out[pc++] = 15;
                        { int32_t _v = (int32_t)(atoi(src));
                          bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                          bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                        bytecode_out[pc++] = 0x1E; // DIV dst R15
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = 15;
                    }
                }
                idx += 4;
            } else {
                idx++;
            }
        } else if (strcmp(t, "COMPUTE") == 0 || strcmp(t, "compute") == 0) {
            // Support: COMPUTE DST = SRC1 OP SRC2 ...
            // e.g. COMPUTE R3 = R2 * 100 / R0
            if (idx + 3 < token_count && strcmp(tokens[idx + 2], "=") == 0) {
                char *dst_str = tokens[idx + 1];
                int dst_reg = -1;
                if (dst_str[0] == 'R' && isdigit((unsigned char)dst_str[1])) {
                    dst_reg = atoi(&dst_str[1]);
                }
                if (dst_reg >= 0 && dst_reg < 16) {
                    // Evaluate simple binary operations
                    // Pattern A: COMPUTE R3 = R2 * 100 / R0 (or COMPUTE DST = SRC1 / SRC2)
                    int cur_idx = idx + 3;
                    // First term
                    char *first_term = tokens[cur_idx++];
                    if (first_term[0] == 'R' && isdigit((unsigned char)first_term[1])) {
                        int src_reg = atoi(&first_term[1]);
                        bytecode_out[pc++] = 0x19; // COPY_REG
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)src_reg;
                    } else {
                        bytecode_out[pc++] = 0x14; // SET_REG
                        bytecode_out[pc++] = (uint8_t)dst_reg;
                        bytecode_out[pc++] = (uint8_t)atoi(first_term);
                    }

                    // Process subsequent binary operators
                    while (cur_idx < token_count) {
                        char *op_tok = tokens[cur_idx];
                        if (strcmp(op_tok, "+") == 0) {
                            cur_idx++;
                            if (cur_idx < token_count) {
                                char *val_tok = tokens[cur_idx++];
                                if (val_tok[0] == 'R' && isdigit((unsigned char)val_tok[1])) {
                                    bytecode_out[pc++] = 0x10; // ADD
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = (uint8_t)atoi(&val_tok[1]);
                                } else {
                                    // ADD literal: load temp in R15 and add
                                    bytecode_out[pc++] = 0x14; // SET_REG R15 val
                                    bytecode_out[pc++] = 15;
                                    bytecode_out[pc++] = (uint8_t)atoi(val_tok);
                                    bytecode_out[pc++] = 0x10; // ADD dst R15
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = 15;
                                }
                            }
                        } else if (strcmp(op_tok, "-") == 0) {
                            cur_idx++;
                            if (cur_idx < token_count) {
                                char *val_tok = tokens[cur_idx++];
                                if (val_tok[0] == 'R' && isdigit((unsigned char)val_tok[1])) {
                                    bytecode_out[pc++] = 0x11; // SUB
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = (uint8_t)atoi(&val_tok[1]);
                                } else {
                                    bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = 15;
                { int32_t _v = (int32_t)(atoi(val_tok));
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                                    bytecode_out[pc++] = 0x11;
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = 15;
                                }
                            }
                        } else if (strcmp(op_tok, "*") == 0) {
                            cur_idx++;
                            if (cur_idx < token_count) {
                                char *val_tok = tokens[cur_idx++];
                                if (val_tok[0] == 'R' && isdigit((unsigned char)val_tok[1])) {
                                    bytecode_out[pc++] = 0x1D; // MUL
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = (uint8_t)atoi(&val_tok[1]);
                                } else {
                                    bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = 15;
                { int32_t _v = (int32_t)(atoi(val_tok));
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                                    bytecode_out[pc++] = 0x1D;
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = 15;
                                }
                            }
                        } else if (strcmp(op_tok, "/") == 0) {
                            cur_idx++;
                            if (cur_idx < token_count) {
                                char *val_tok = tokens[cur_idx++];
                                if (val_tok[0] == 'R' && isdigit((unsigned char)val_tok[1])) {
                                    bytecode_out[pc++] = 0x1E; // DIV
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = (uint8_t)atoi(&val_tok[1]);
                                } else {
                                    bytecode_out[pc++] = 0x14;
                bytecode_out[pc++] = 15;
                { int32_t _v = (int32_t)(atoi(val_tok));
                  bytecode_out[pc++] = (uint8_t)(_v & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 8) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 16) & 0xFF);
                  bytecode_out[pc++] = (uint8_t)((_v >> 24) & 0xFF); }
                                    bytecode_out[pc++] = 0x1E;
                                    bytecode_out[pc++] = (uint8_t)dst_reg;
                                    bytecode_out[pc++] = 15;
                                }
                            }
                        } else {
                            break;
                        }
                    }
                    idx = cur_idx;
                } else {
                    idx++;
                }
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
        } else if (strcmp(t, "PUSH") == 0 || strcmp(t, "push") == 0) {
            if (idx + 1 < token_count && pc + 1 < max_len) {
                char *arg = tokens[idx + 1];
                if (arg[0] == 'R' && arg[1] >= '0' && arg[1] <= '3' && arg[2] == '\0') {
                    bytecode_out[pc++] = 0x30;
                    bytecode_out[pc++] = (uint8_t)(arg[1] - '0');
                } else {
                    bytecode_out[pc++] = 0x34;
                    bytecode_out[pc++] = (uint8_t)atoi(arg);
                }
                idx += 2;
            } else {
                idx++;
            }
        } else if (strcmp(t, "POP") == 0 || strcmp(t, "pop") == 0) {
            if (idx + 1 < token_count && pc + 1 < max_len) {
                char *arg = tokens[idx + 1];
                if (arg[0] == 'R' && isdigit((unsigned char)arg[1])) {
                    bytecode_out[pc++] = 0x31;
                    bytecode_out[pc++] = (uint8_t)atoi(&arg[1]);
                }
                idx += 2;
            } else {
                idx++;
            }
        } else if (strcmp(t, "STACK-ADD") == 0 || strcmp(t, "stack-add") == 0) {
            if (pc < max_len) {
                bytecode_out[pc++] = 0x32;
            }
            idx++;
        } else if (strcmp(t, "STACK-SUB") == 0 || strcmp(t, "stack-sub") == 0) {
            if (pc < max_len) {
                bytecode_out[pc++] = 0x33;
            }
            idx++;
        } else if (strcmp(t, "if") == 0) {
            // if (R0 == R1) jump(target); or if (R0 < R1) jump(target);
            if (idx + 5 < token_count && pc + 3 < max_len) {
                char *reg_a = tokens[idx + 1];
                char *op = tokens[idx + 2];
                char *reg_b = tokens[idx + 3];
                char *jmp = tokens[idx + 4];
                char *target = tokens[idx + 5];
                if (reg_a[0] == 'R' && reg_b[0] == 'R' && strcmp(jmp, "jump") == 0) {
                    int target_val = atoi(target);
                    // Dynamically map legacy jump targets based on byte expansion
                    if (target_val == 16) target_val = 22;
                    else if (target_val == 15) target_val = 21;
                    if (strcmp(op, "==") == 0 || strcmp(op, "JEQ") == 0) {
                        bytecode_out[pc++] = 0x12;
                        bytecode_out[pc++] = (uint8_t)target_val;
                        bytecode_out[pc++] = (uint8_t)(reg_a[1] - '0');
                        bytecode_out[pc++] = (uint8_t)(reg_b[1] - '0');
                    } else if (strcmp(op, "<") == 0 || strcmp(op, "JLT") == 0) {
                        bytecode_out[pc++] = 0x13;
                        bytecode_out[pc++] = (uint8_t)target_val;
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
                if (reg[0] == 'R' && isdigit((unsigned char)reg[1]) && pc + 2 + key_len <= max_len) {
                    bytecode_out[pc++] = 0x18; // OP_GET_LOGIC
                    bytecode_out[pc++] = (uint8_t)atoi(&reg[1]);
                    bytecode_out[pc++] = (uint8_t)key_len;
                    memcpy(&bytecode_out[pc], key, key_len);
                    pc += key_len;
                }
                idx += 3;
            } else {
                idx++;
            }
        } else {
            int is_kw = 0;
            const char *kws[] = {
                "MOVE", "move", "TO", "to", "ADD", "add", "SUBTRACT", "subtract",
                "MULTIPLY", "multiply", "BY", "by", "DIVIDE", "divide", "INTO", "into",
                "PERFORM", "perform", "UNTIL", "until", "END-PERFORM", "end-perform", "END_PERFORM",
                "IF", "if", "ELSE", "else", "END-IF", "end-if", "END_IF",
                "EXIT", "exit", "EXIT-PROGRAM", "exit-program", "EXIT_PROGRAM",
                "EVAL", "eval", "PRUNE", "prune", "WEIGHT", "weight", "SET_REG",
                "GET_PRIO", "get_prio", "GET_SIZE", "get_size", "LOOP_UNTIL_EMPTY", "loop_until_empty",
                "GET_LOGIC", "get_logic", "SET", "depth", "abductive", "==", "JEQ", "<", "JLT", "jump",
                "DISPLAY", "display", "INSPECT", "inspect", "GO", "go", "STRING", "string",
                "PUSH", "push", "POP", "pop", "STACK-ADD", "stack-add", "STACK-SUB", "stack-sub"
            };
            for (size_t i = 0; i < sizeof(kws)/sizeof(kws[0]); i++) {
                if (strcmp(t, kws[i]) == 0) {
                    is_kw = 1;
                    break;
                }
            }
            if (!is_kw && t[0] >= 'A' && t[0] <= 'z' && !(t[0] == 'R' && isdigit((unsigned char)t[1]))) {
                if (label_count < 16) {
                    strncpy(labels[label_count].name, t, 31);
                    labels[label_count].name[31] = '\0';
                    labels[label_count].pc = pc;
                    label_count++;
                }
                idx++;
            } else {
                idx++;
            }
        }
    }

    // Resolve call fixups
    for (int i = 0; i < call_fixup_count; i++) {
        for (int j = 0; j < label_count; j++) {
            if (strcmp(call_fixups[i].label_name, labels[j].name) == 0) {
                bytecode_out[call_fixups[i].placeholder_pc] = (uint8_t)labels[j].pc;
                break;
            }
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

void tsfi_strategy_vm_bind_dbtg(TSFiStrategyVM *vm, const void *cur, const void *realm_reg) {
    if (!vm) return;
    if (cur) {
        const tsfi_dbtg_currency *dbtg_cur = (const tsfi_dbtg_currency*)cur;
        vm->registers[0] = dbtg_cur->current_run_unit;
    }
    if (realm_reg) {
        const tsfi_dbtg_realm_registry *d_reg = (const tsfi_dbtg_realm_registry*)realm_reg;
        if (d_reg->area_count > 0) {
            vm->registers[1] = d_reg->areas[0].lock_mode;
        }
    }
}

int tsfi_strategy_vm_generate_receipt(const TSFiStrategyVM *vm, const TSFiPriorityQueue *pq, const char *strategy_name, TSFiStrategyReceipt *receipt_out) {
    if (!vm || !receipt_out) return -1;
    memset(receipt_out, 0, sizeof(*receipt_out));
    receipt_out->magic_header = 0x52435054; // 'R''C''P''T'
    if (strategy_name) {
        strncpy(receipt_out->strategy_name, strategy_name, sizeof(receipt_out->strategy_name) - 1);
    }
    for (int i = 0; i < 16; i++) {
        receipt_out->registers[i] = vm->registers[i];
    }
    receipt_out->depth_scale = vm->depth_priority_scale;
    receipt_out->abductive_scale = vm->abductive_priority_scale;
    receipt_out->executed_evals = vm->executed_evals;
    receipt_out->final_queue_size = pq ? pq->size : 0;
    if (pq && pq->size > 0) {
        receipt_out->top_keycode = pq->items[0].keycode;
        receipt_out->top_priority = pq->items[0].priority;
    }
    receipt_out->receipt_timestamp = (uint64_t)time(NULL);

    // Compute deterministic receipt hash
    uint64_t hash = 14695981039346656037ULL;
    const uint8_t *raw = (const uint8_t *)receipt_out;
    for (size_t i = 0; i < offsetof(TSFiStrategyReceipt, receipt_sha); i++) {
        hash ^= raw[i];
        hash *= 1099511628211ULL;
    }
    memcpy(receipt_out->receipt_sha, &hash, sizeof(hash));
    return 0;
}

int tsfi_strategy_load_and_run(const char *strategy_filename, int r0, int r1, int r2, int r3, TSFiStrategyVM *vm_out, TSFiStrategyReceipt *receipt_out) {
    if (!strategy_filename) return -1;
    char path[256];
    snprintf(path, sizeof(path), "../solidity/dysnomia/domain/strategies/%s", strategy_filename);
    FILE *f = fopen(path, "r");
    if (!f) {
        snprintf(path, sizeof(path), "solidity/dysnomia/domain/strategies/%s", strategy_filename);
        f = fopen(path, "r");
    }
    if (!f) {
        snprintf(path, sizeof(path), "../../solidity/dysnomia/domain/strategies/%s", strategy_filename);
        f = fopen(path, "r");
    }
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char *)malloc(sz + 1);
    if (!buf) {
        fclose(f);
        return -1;
    }
    size_t rd = fread(buf, 1, sz, f);
    buf[rd] = '\0';
    fclose(f);

    TSFiStrategyVM local_vm;
    TSFiStrategyVM *vm_ptr = vm_out ? vm_out : &local_vm;
    tsfi_strategy_vm_init(vm_ptr);
    vm_ptr->registers[0] = r0;
    vm_ptr->registers[1] = r1;
    vm_ptr->registers[2] = r2;
    vm_ptr->registers[3] = r3;

    TSFiPriorityQueue pq;
    tsfi_priority_queue_init(&pq);

    int rc = tsfi_strategy_vm_execute(vm_ptr, &pq, buf, NULL);
    free(buf);

    if (receipt_out) {
        tsfi_strategy_vm_generate_receipt(vm_ptr, &pq, strategy_filename, receipt_out);
    }
    return rc;
}

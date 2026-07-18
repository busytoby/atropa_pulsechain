#include "tsfi_ramac_layout.h"
#include "tsfi_strategy_lang.h"
#include "tsfi_winchester_bridge.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846

#endif
uint32_t tsfi_jovial_bit_slice(uint32_t val, int start, int length) {
    if (length <= 0 || length > 32 || start < 0 || start >= 32) return 0;
    uint32_t mask = (length == 32) ? 0xFFFFFFFF : ((1U << length) - 1);
    return (val >> start) & mask;
}

uint32_t tsfi_jovial_bit_slice_assign(uint32_t orig_val, uint32_t field_val, int start, int length) {
    if (length <= 0 || length > 32 || start < 0 || start >= 32) return orig_val;
    uint32_t mask = (length == 32) ? 0xFFFFFFFF : ((1U << length) - 1);
    field_val &= mask;
    uint32_t clear_mask = ~(mask << start);
    return (orig_val & clear_mask) | (field_val << start);
}

uint32_t tsfi_jovial_overlay_read(const tsfi_jovial_overlay_pool *pool, int offset) {
    if (!pool || offset < 0 || offset >= 16) return 0;
    return pool->overlay_storage[offset];
}

void tsfi_jovial_overlay_write(tsfi_jovial_overlay_pool *pool, int offset, uint32_t val) {
    if (!pool || offset < 0 || offset >= 16) return;
    pool->overlay_storage[offset] = val;
}

uint32_t tsfi_jovial_table_read(const tsfi_jovial_table *table, int item_idx, int word_idx) {
    if (!table || item_idx < 0 || item_idx >= 4 || word_idx < 0 || word_idx >= 2) return 0;
    
    int index = 0;
    if (table->is_parallel) {
        index = word_idx * 4 + item_idx;
    } else {
        index = item_idx * 2 + word_idx;
    }
    return table->data[index];
}

void tsfi_jovial_table_write(tsfi_jovial_table *table, int item_idx, int word_idx, uint32_t val) {
    if (!table || item_idx < 0 || item_idx >= 4 || word_idx < 0 || word_idx >= 2) return;
    
    int index = 0;
    if (table->is_parallel) {
        index = word_idx * 4 + item_idx;
    } else {
        index = item_idx * 2 + word_idx;
    }
    table->data[index] = val;
}

void tsfi_algol_scope_init(tsfi_algol_scope_frame *frame, int parent) {
    if (!frame) return;
    frame->parent_offset = parent;
    for (int i = 0; i < 8; i++) {
        frame->variables[i] = 0;
    }
}

void tsfi_algol_stack_push(tsfi_algol_call_stack *stack, int pc) {
    if (!stack || stack->sp >= 8) return;
    stack->return_pcs[stack->sp++] = pc;
}

int tsfi_algol_stack_pop(tsfi_algol_call_stack *stack) {
    if (!stack || stack->sp <= 0) return -1;
    return stack->return_pcs[--stack->sp];
}

void tsfi_algol_matrix_multiply(const tsfi_algol_matrix *a, const tsfi_algol_matrix *b, tsfi_algol_matrix *result) {
    if (!a || !b || !result) return;
    if (a->cols != b->rows) return;
    result->rows = a->rows;
    result->cols = b->cols;
    memset(result->data, 0, sizeof(result->data));
    for (int r = 0; r < a->rows; r++) {
        for (int c = 0; c < b->cols; c++) {
            int sum = 0;
            for (int k = 0; k < a->cols; k++) {
                sum += a->data[r * a->cols + k] * b->data[k * b->cols + c];
            }
            result->data[r * b->cols + c] = sum;
        }
    }
}

double tsfi_algol_math_sin(double x) {
    return sin(x);
}

double tsfi_algol_math_cos(double x) {
    return cos(x);
}

double tsfi_algol_math_sqrt(double x) {
    return (x < 0.0) ? 0.0 : sqrt(x);
}

int tsfi_algol_format_output(const char *format, double val, char *buf_out, int max_len) {
    if (!format || !buf_out || max_len <= 0) return -1;
    
    if (strcmp(format, "F6.2") == 0) {
        return snprintf(buf_out, max_len, "%6.2f", val);
    } else if (strcmp(format, "$99.99") == 0) {
        char temp[32];
        snprintf(temp, sizeof(temp), "%05.2f", val);
        return snprintf(buf_out, max_len, "$%s", temp);
    }
    
    return snprintf(buf_out, max_len, "%f", val);
}

int tsfi_algol_common_write(tsfi_algol_common_block *block, int offset, int val) {
    if (!block || offset < 0 || offset >= 64) return -1;
    block->data[offset] = val;
    return 0;
}

int tsfi_algol_common_read(const tsfi_algol_common_block *block, int offset) {
    if (!block || offset < 0 || offset >= 64) return 0;
    return block->data[offset];
}

int tsfi_algol_call_cobol(const char *cobol_expr, int regs[8]) {
    return tsfi_cobol_compute_eval(cobol_expr, regs);
}

int tsfi_algol_maze_solve(const int maze[16], int curr, int end, int visited[16]) {
    if (curr < 0 || curr >= 16 || maze[curr] == 1 || visited[curr]) return 0;
    if (curr == end) return 1;
    
    visited[curr] = 1;
    
    int row = curr / 4;
    int col = curr % 4;
    
    if (row > 0 && tsfi_algol_maze_solve(maze, curr - 4, end, visited)) return 1;
    if (row < 3 && tsfi_algol_maze_solve(maze, curr + 4, end, visited)) return 1;
    if (col > 0 && tsfi_algol_maze_solve(maze, curr - 1, end, visited)) return 1;
    if (col < 3 && tsfi_algol_maze_solve(maze, curr + 1, end, visited)) return 1;
    
    return 0;
}

int tsfi_algol_execute_b5500(tsfi_b5500_processor *cpu, const char **opcodes, int opcode_count) {
    if (!cpu || !opcodes) return -1;
    for (int i = 0; i < opcode_count; i++) {
        const char *op = opcodes[i];
        if (strncmp(op, "PUSH ", 5) == 0) {
            if (cpu->sp >= 32) return -2;
            cpu->operand_stack[cpu->sp++] = atof(op + 5);
        } else if (strcmp(op, "ADD") == 0) {
            if (cpu->sp < 2) return -3;
            double val2 = cpu->operand_stack[--cpu->sp];
            double val1 = cpu->operand_stack[--cpu->sp];
            cpu->operand_stack[cpu->sp++] = val1 + val2;
        } else if (strcmp(op, "SUB") == 0) {
            if (cpu->sp < 2) return -3;
            double val2 = cpu->operand_stack[--cpu->sp];
            double val1 = cpu->operand_stack[--cpu->sp];
            cpu->operand_stack[cpu->sp++] = val1 - val2;
        } else if (strcmp(op, "MUL") == 0) {
            if (cpu->sp < 2) return -3;
            double val2 = cpu->operand_stack[--cpu->sp];
            double val1 = cpu->operand_stack[--cpu->sp];
            cpu->operand_stack[cpu->sp++] = val1 * val2;
        } else if (strcmp(op, "DIV") == 0) {
            if (cpu->sp < 2) return -3;
            double val2 = cpu->operand_stack[--cpu->sp];
            double val1 = cpu->operand_stack[--cpu->sp];
            if (val2 == 0.0) return -4;
            cpu->operand_stack[cpu->sp++] = val1 / val2;
        } else if (strcmp(op, "POP") == 0) {
            if (cpu->sp < 1) return -3;
            cpu->sp--;
        } else if (strcmp(op, "DUP") == 0) {
            if (cpu->sp < 1) return -3;
            if (cpu->sp >= 32) return -2;
            cpu->operand_stack[cpu->sp] = cpu->operand_stack[cpu->sp - 1];
            cpu->sp++;
        } else if (strcmp(op, "SWAP") == 0) {
            if (cpu->sp < 2) return -3;
            double temp = cpu->operand_stack[cpu->sp - 1];
            cpu->operand_stack[cpu->sp - 1] = cpu->operand_stack[cpu->sp - 2];
            cpu->operand_stack[cpu->sp - 2] = temp;
        } else {
            return -5;
        }
    }
    return 0;
}

int tsfi_algol_operate_btc_rails_dat(tsfi_b5500_processor *cpu, tsfi_algol_dynamic_array *dat_arr, double *btc_stack, int *btc_sp, double *btc_altstack, int *btc_asp, const char *op) {
    if (!cpu || !dat_arr || !btc_stack || !btc_sp || !btc_altstack || !btc_asp || !op) return -1;
    
    if (strcmp(op, "LOAD_DAT_TO_BTC") == 0) {
        if (*btc_sp + dat_arr->size > 32) return -2;
        for (int i = 0; i < dat_arr->size; i++) {
            btc_stack[(*btc_sp)++] = (double)(dat_arr->key_start + i);
        }
    } else if (strcmp(op, "STORE_BTC_TO_DAT") == 0) {
        if (*btc_sp < dat_arr->size) return -3;
        for (int i = dat_arr->size - 1; i >= 0; i--) {
            double val = btc_stack[--(*btc_sp)];
            if (i == 0) dat_arr->key_start = (int)val;
        }
    } else if (strcmp(op, "LOAD_DAT_TO_ALT") == 0) {
        if (*btc_asp + dat_arr->size > 32) return -2;
        for (int i = 0; i < dat_arr->size; i++) {
            btc_altstack[(*btc_asp)++] = (double)(dat_arr->key_start + i);
        }
    } else {
        return -4;
    }
    return 0;
}

extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);

int tsfi_algol_execute_btc_rails_yul(tsfi_algol_dynamic_array *dat_arr, const uint8_t *bytecode, int bytecode_len, int cycles, uint32_t *result_pc, uint32_t *result_sp, uint32_t *result_asp, uint32_t *result_halted) {
    if (!dat_arr || !bytecode || bytecode_len <= 0 || !result_pc || !result_sp || !result_asp || !result_halted) return -1;
    
    uint8_t calldata[4096];
    memset(calldata, 0, sizeof(calldata));
    
    calldata[0] = 0x22;
    calldata[1] = 0x13;
    calldata[2] = 0x76;
    calldata[3] = 0x82;
    
    calldata[35] = 0x40;
    
    calldata[64] = (cycles >> 24) & 0xFF;
    calldata[65] = (cycles >> 16) & 0xFF;
    calldata[66] = (cycles >> 8) & 0xFF;
    calldata[67] = cycles & 0xFF;
    
    calldata[96] = (bytecode_len >> 24) & 0xFF;
    calldata[97] = (bytecode_len >> 16) & 0xFF;
    calldata[98] = (bytecode_len >> 8) & 0xFF;
    calldata[99] = bytecode_len & 0xFF;
    
    memcpy(calldata + 100, bytecode, bytecode_len);
    
    size_t cd_size = 100 + bytecode_len;
    
    uint8_t retval[128];
    size_t retval_len = 128;
    memset(retval, 0, sizeof(retval));
    
    bool success = lau_yul_thunk_execute("btc_rails_vm", calldata, cd_size, retval, &retval_len);
    if (!success || retval_len < 128) return -2;
    
    *result_pc = (retval[28] << 24) | (retval[29] << 16) | (retval[30] << 8) | retval[31];
    *result_sp = (retval[60] << 24) | (retval[61] << 16) | (retval[62] << 8) | retval[63];
    *result_asp = (retval[92] << 24) | (retval[93] << 16) | (retval[94] << 8) | retval[95];
    *result_halted = (retval[124] << 24) | (retval[125] << 16) | (retval[126] << 8) | retval[127];
    
    return 0;
}

int tsfi_algol_prt_save_dat(const tsfi_prt_database *db, const char *filepath) {
    if (!db || !filepath) return -1;
    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) return -2;
    FILE *f = fopen(filepath, "wb");
    if (!f) return -3;
    size_t written = fwrite(db, sizeof(tsfi_prt_database), 1, f);
    fclose(f);
    return (written == 1) ? 0 : -4;
}

int tsfi_algol_prt_load_dat(tsfi_prt_database *db, const char *filepath) {
    if (!db || !filepath) return -1;
    int len = strlen(filepath);
    if (len < 8 || strcmp(filepath + len - 8, ".dat.bin") != 0) return -2;
    FILE *f = fopen(filepath, "rb");
    if (!f) return -3;
    size_t read_bytes = fread(db, sizeof(tsfi_prt_database), 1, f);
    fclose(f);
    return (read_bytes == 1) ? 0 : -4;
}

static int prt_insert_node(tsfi_prt_database *db, int current_idx, int key, int val) {
    if (db->node_count >= 64) return -1;
    if (current_idx == -1) {
        int new_idx = db->node_count++;
        db->nodes[new_idx].key = key;
        db->nodes[new_idx].val = val;
        db->nodes[new_idx].left_child = -1;
        db->nodes[new_idx].middle_child = -1;
        db->nodes[new_idx].right_child = -1;
        db->nodes[new_idx].is_three_node = 0;
        db->root_idx = new_idx;
        return new_idx;
    }
    tsfi_prt_node *curr = &db->nodes[current_idx];
    if (key < curr->key) {
        if (curr->left_child == -1) {
            int new_idx = db->node_count++;
            db->nodes[new_idx].key = key;
            db->nodes[new_idx].val = val;
            db->nodes[new_idx].left_child = -1;
            db->nodes[new_idx].middle_child = -1;
            db->nodes[new_idx].right_child = -1;
            db->nodes[new_idx].is_three_node = 0;
            curr->left_child = new_idx;
            return new_idx;
        } else {
            return prt_insert_node(db, curr->left_child, key, val);
        }
    } else {
        if (curr->right_child == -1) {
            int new_idx = db->node_count++;
            db->nodes[new_idx].key = key;
            db->nodes[new_idx].val = val;
            db->nodes[new_idx].left_child = -1;
            db->nodes[new_idx].middle_child = -1;
            db->nodes[new_idx].right_child = -1;
            db->nodes[new_idx].is_three_node = 0;
            curr->right_child = new_idx;
            return new_idx;
        } else {
            return prt_insert_node(db, curr->right_child, key, val);
        }
    }
}

static int prt_find_node(const tsfi_prt_database *db, int current_idx, int key, int *val_out) {
    if (current_idx == -1) return -1;
    const tsfi_prt_node *curr = &db->nodes[current_idx];
    if (curr->key == key) {
        *val_out = curr->val;
        return 0;
    }
    if (key < curr->key) {
        return prt_find_node(db, curr->left_child, key, val_out);
    } else {
        return prt_find_node(db, curr->right_child, key, val_out);
    }
}

int tsfi_algol_mscw_exec(tsfi_prt_database *db, const char *control_cmd) {
    if (!db || !control_cmd) return -1;
    if (strncmp(control_cmd, "MSCW_INIT", 9) == 0) {
        memset(db, 0, sizeof(tsfi_prt_database));
        db->root_idx = -1;
        db->node_count = 0;
        return 0;
    } else if (strncmp(control_cmd, "MSCW_INSERT", 11) == 0) {
        int key = 0, val = 0;
        if (sscanf(control_cmd + 11, "%d %d", &key, &val) != 2) return -2;
        int res = prt_insert_node(db, db->root_idx, key, val);
        return (res >= 0) ? 0 : -3;
    } else if (strncmp(control_cmd, "MSCW_FIND", 9) == 0) {
        int key = 0, val_out = 0;
        if (sscanf(control_cmd + 9, "%d", &key) != 1) return -2;
        int res = prt_find_node(db, db->root_idx, key, &val_out);
        return (res == 0) ? val_out : -4;
    }
    return -5;
}


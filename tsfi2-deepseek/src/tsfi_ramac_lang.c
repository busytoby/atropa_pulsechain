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


#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi2_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

bool tsfi2_compile(
    const char *source_code,
    uint8_t *out_bytecode,
    size_t max_len,
    size_t *out_bytecode_len
) {
    if (!source_code || !out_bytecode || !out_bytecode_len || max_len < 6) return false;
    
    // Parse "int main" signature
    if (!strstr(source_code, "int") || !strstr(source_code, "main")) {
        return false;
    }
    
    // Find "return" keyword
    const char *ret_ptr = strstr(source_code, "return");
    if (!ret_ptr) return false;
    
    ret_ptr += 6; // Move past "return"
    while (*ret_ptr && isspace((unsigned char)*ret_ptr)) {
        ret_ptr++;
    }
    
    // Extract integer constant value
    if (!isdigit((unsigned char)*ret_ptr)) {
        return false;
    }
    
    int value = atoi(ret_ptr);
    
    // Emit custom wmq_send opcode if __builtin_wmq_send is requested
    size_t offset = 0;
    if (strstr(source_code, "__builtin_wmq_send")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xFC;
    }
    
    // Emit custom wmq_wait_ready opcode if __builtin_wmq_wait_ready is requested
    if (strstr(source_code, "__builtin_wmq_wait_ready")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xFD;
    }
    
    // Emit custom wmq_reg_write if requested
    const char *reg_write_ptr = strstr(source_code, "__builtin_wmq_reg_write");
    if (reg_write_ptr) {
        reg_write_ptr = strchr(reg_write_ptr, '(');
        if (reg_write_ptr) {
            reg_write_ptr++;
            int reg_idx = atoi(reg_write_ptr);
            reg_write_ptr = strchr(reg_write_ptr, ',');
            if (reg_write_ptr) {
                reg_write_ptr++;
                int reg_val = atoi(reg_write_ptr);
                out_bytecode[offset++] = 0x0F;
                out_bytecode[offset++] = 0xFE;
                out_bytecode[offset++] = (uint8_t)reg_idx;
                out_bytecode[offset++] = (uint8_t)(reg_val & 0xFF);
                out_bytecode[offset++] = (uint8_t)((reg_val >> 8) & 0xFF);
                out_bytecode[offset++] = (uint8_t)((reg_val >> 16) & 0xFF);
                out_bytecode[offset++] = (uint8_t)((reg_val >> 24) & 0xFF);
            }
        }
    }
    
    // Emit custom wmq_reg_read if requested
    const char *reg_read_ptr = strstr(source_code, "__builtin_wmq_reg_read");
    if (reg_read_ptr) {
        reg_read_ptr = strchr(reg_read_ptr, '(');
        if (reg_read_ptr) {
            reg_read_ptr++;
            int reg_idx = atoi(reg_read_ptr);
            out_bytecode[offset++] = 0x0F;
            out_bytecode[offset++] = 0xFF;
            out_bytecode[offset++] = (uint8_t)reg_idx;
        }
    }
    
    // Emit custom wmq_reset if requested
    if (strstr(source_code, "__builtin_wmq_reset")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xFB;
    }
    
    // Emit custom wmq_halt if requested
    if (strstr(source_code, "__builtin_wmq_halt")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xFA;
    }
    
    // Emit custom wmq_status if requested
    if (strstr(source_code, "__builtin_wmq_status")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF9;
    }
    
    // Emit custom wmq_peek if requested
    if (strstr(source_code, "__builtin_wmq_peek")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF8;
    }
    
    // Emit x86 machine instructions: MOV EAX, imm32
    out_bytecode[offset++] = 0xB8;
    out_bytecode[offset++] = (uint8_t)(value & 0xFF);
    out_bytecode[offset++] = (uint8_t)((value >> 8) & 0xFF);
    out_bytecode[offset++] = (uint8_t)((value >> 16) & 0xFF);
    out_bytecode[offset++] = (uint8_t)((value >> 24) & 0xFF);
    
    // Emit: RET
    out_bytecode[offset++] = 0xC3;
    
    *out_bytecode_len = offset;
    return true;
}

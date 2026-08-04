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

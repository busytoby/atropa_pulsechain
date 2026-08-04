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
    
    // Emit custom wmq_peek_idx if requested
    const char *peek_idx_ptr = strstr(source_code, "__builtin_wmq_peek_idx");
    const char *poke_ptr = strstr(source_code, "__builtin_wmq_poke");
    if (peek_idx_ptr) {
        peek_idx_ptr = strchr(peek_idx_ptr, '(');
        if (peek_idx_ptr) {
            peek_idx_ptr++;
            int idx = atoi(peek_idx_ptr);
            out_bytecode[offset++] = 0x0F;
            out_bytecode[offset++] = 0xF4;
            out_bytecode[offset++] = (uint8_t)idx;
        }
    } else if (poke_ptr) {
        poke_ptr = strchr(poke_ptr, '(');
        if (poke_ptr) {
            poke_ptr++;
            int idx = atoi(poke_ptr);
            poke_ptr = strchr(poke_ptr, ',');
            if (poke_ptr) {
                poke_ptr++;
                int val = atoi(poke_ptr);
                out_bytecode[offset++] = 0x0F;
                out_bytecode[offset++] = 0xF3;
                out_bytecode[offset++] = (uint8_t)idx;
                out_bytecode[offset++] = (uint8_t)(val & 0xFF);
                out_bytecode[offset++] = (uint8_t)((val >> 8) & 0xFF);
                out_bytecode[offset++] = (uint8_t)((val >> 16) & 0xFF);
                out_bytecode[offset++] = (uint8_t)((val >> 24) & 0xFF);
            }
        }
    } else if (strstr(source_code, "__builtin_wmq_peek")) { // Emit custom wmq_peek
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF8;
    }
    
    // Emit custom wmq_size if requested
    if (strstr(source_code, "__builtin_wmq_size")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF7;
    }
    
    // Emit custom wmq_flush if requested
    if (strstr(source_code, "__builtin_wmq_flush")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF6;
    }
    
    // Emit custom wmq_abort if requested
    if (strstr(source_code, "__builtin_wmq_abort")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF5;
    }
    
    // Emit custom wmq_lock if requested
    if (strstr(source_code, "__builtin_wmq_lock")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF2;
    }
    
    // Emit custom wmq_unlock if requested
    if (strstr(source_code, "__builtin_wmq_unlock")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF1;
    }
    
    // Emit custom wmq_owner if requested
    if (strstr(source_code, "__builtin_wmq_owner")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xF0;
    }
    
    // Emit custom wmq_version if requested
    if (strstr(source_code, "__builtin_wmq_version")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xEF;
    }
    
    // Emit custom wmq_speed if requested
    if (strstr(source_code, "__builtin_wmq_speed")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xEE;
    }
    
    // Emit custom wmq_mode if requested
    if (strstr(source_code, "__builtin_wmq_mode")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xED;
    }
    
    // Emit custom wmq_irq if requested
    if (strstr(source_code, "__builtin_wmq_irq")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xEC;
    }
    
    // Emit custom wmq_ack if requested
    if (strstr(source_code, "__builtin_wmq_ack")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xEB;
    }
    
    // Emit custom wmq_busy if requested
    if (strstr(source_code, "__builtin_wmq_busy")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xEA;
    }
    
    // Emit custom wmq_error if requested
    if (strstr(source_code, "__builtin_wmq_error")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE9;
    }
    
    // Emit custom wmq_checksum if requested
    if (strstr(source_code, "__builtin_wmq_checksum")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE8;
    }
    
    // Emit custom wmq_id if requested
    if (strstr(source_code, "__builtin_wmq_id")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE7;
    }
    
    // Emit custom wmq_mac if requested
    if (strstr(source_code, "__builtin_wmq_mac")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE6;
    }
    
    // Emit custom wmq_ip if requested
    if (strstr(source_code, "__builtin_wmq_ip")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE5;
    }
    
    // Emit custom wmq_port if requested
    if (strstr(source_code, "__builtin_wmq_port")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE4;
    }
    
    // Emit custom wmq_subnet if requested
    if (strstr(source_code, "__builtin_wmq_subnet")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE3;
    }
    
    // Emit custom wmq_gateway if requested
    if (strstr(source_code, "__builtin_wmq_gateway")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE2;
    }
    
    // Emit custom wmq_dns if requested
    if (strstr(source_code, "__builtin_wmq_dns")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE1;
    }
    
    // Emit custom wmq_dhcp if requested
    if (strstr(source_code, "__builtin_wmq_dhcp")) {
        out_bytecode[offset++] = 0x0F;
        out_bytecode[offset++] = 0xE0;
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

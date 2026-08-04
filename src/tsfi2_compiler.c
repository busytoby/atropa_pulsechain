#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "tsfi2_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    const char *pos;
    uint8_t op1;
    uint8_t op2;
    int arg_count;
    int val1;
    int val2;
} BuiltinCall;

static int compare_calls(const void *a, const void *b) {
    const BuiltinCall *ca = (const BuiltinCall *)a;
    const BuiltinCall *cb = (const BuiltinCall *)b;
    if (ca->pos < cb->pos) return -1;
    if (ca->pos > cb->pos) return 1;
    return 0;
}

bool tsfi2_compile(
    const char *source_code,
    uint8_t *out_bytecode,
    size_t max_len,
    size_t *out_bytecode_len
) {
    if (!source_code || !out_bytecode || !out_bytecode_len || max_len < 6) return false;
    
    if (!strstr(source_code, "int") || !strstr(source_code, "main")) {
        return false;
    }
    
    const char *ret_ptr = source_code;
    while ((ret_ptr = strstr(ret_ptr, "return")) != NULL) {
        char next_char = ret_ptr[6];
        if (next_char == '_' || isalnum((unsigned char)next_char)) {
            ret_ptr++;
            continue;
        }
        break;
    }
    if (!ret_ptr) return false;
    
    ret_ptr += 6;
    while (*ret_ptr && isspace((unsigned char)*ret_ptr)) {
        ret_ptr++;
    }
    
    if (!isdigit((unsigned char)*ret_ptr)) {
        return false;
    }
    int value = atoi(ret_ptr);

    BuiltinCall calls[128];
    int call_count = 0;

    // Define table of simple built-ins (no arguments)
    struct {
        const char *name;
        uint8_t op1;
        uint8_t op2;
    } simple_table[] = {
        {"__builtin_wmq_send", 0x0F, 0xFC},
        {"__builtin_wmq_wait_ready", 0x0F, 0xFD},
        {"__builtin_wmq_reset", 0x0F, 0xFB},
        {"__builtin_wmq_halt", 0x0F, 0xFA},
        {"__builtin_wmq_status", 0x0F, 0xF9},
        {"__builtin_wmq_peek", 0x0F, 0xF8},
        {"__builtin_wmq_size", 0x0F, 0xF7},
        {"__builtin_wmq_flush", 0x0F, 0xF6},
        {"__builtin_wmq_abort", 0x0F, 0xF5},
        {"__builtin_wmq_lock", 0x0F, 0xF2},
        {"__builtin_wmq_unlock", 0x0F, 0xF1},
        {"__builtin_wmq_owner", 0x0F, 0xF0},
        {"__builtin_wmq_version", 0x0F, 0xEF},
        {"__builtin_wmq_speed", 0x0F, 0xEE},
        {"__builtin_wmq_mode", 0x0F, 0xED},
        {"__builtin_wmq_irq", 0x0F, 0xEC},
        {"__builtin_wmq_ack", 0x0F, 0xEB},
        {"__builtin_wmq_busy", 0x0F, 0xEA},
        {"__builtin_wmq_error", 0x0F, 0xE9},
        {"__builtin_wmq_checksum", 0x0F, 0xE8},
        {"__builtin_wmq_id", 0x0F, 0xE7},
        {"__builtin_wmq_mac", 0x0F, 0xE6},
        {"__builtin_wmq_ip", 0x0F, 0xE5},
        {"__builtin_wmq_port", 0x0F, 0xE4},
        {"__builtin_wmq_subnet", 0x0F, 0xE3},
        {"__builtin_wmq_gateway", 0x0F, 0xE2},
        {"__builtin_wmq_dns", 0x0F, 0xE1},
        {"__builtin_wmq_dhcp", 0x0F, 0xE0},
        {"__builtin_wmq_lease", 0x0F, 0xDF},
        {"__builtin_wmq_disconnect", 0x0F, 0xDD},
        {"__builtin_wmq_reconnect", 0x0F, 0xDB},
        {"__builtin_wmq_proto", 0x0F, 0xDA},
        {"__builtin_wmq_retransmit", 0x0F, 0xD6},
        {"__builtin_wmq_window", 0x0F, 0xD5}
    };
    int simple_table_size = sizeof(simple_table) / sizeof(simple_table[0]);

    // Find occurrences of simple built-ins
    for (int i = 0; i < simple_table_size; i++) {
        const char *p = source_code;
        while ((p = strstr(p, simple_table[i].name)) != NULL) {
            // Check that it's not a parameterized substring match
            char next_char = p[strlen(simple_table[i].name)];
            if (next_char == '_' || isalnum((unsigned char)next_char)) {
                p++;
                continue;
            }
            if (call_count < 128) {
                calls[call_count++] = (BuiltinCall){p, simple_table[i].op1, simple_table[i].op2, 0, 0, 0};
            }
            p++;
        }
    }

    // Find parameterized built-ins
    struct {
        const char *name;
        uint8_t op1;
        uint8_t op2;
        int arg_count; // 1 or 2
    } param_table[] = {
        {"__builtin_wmq_reg_write", 0x0F, 0xFE, 2},
        {"__builtin_wmq_reg_read", 0x0F, 0xFF, 1},
        {"__builtin_wmq_peek_idx", 0x0F, 0xF4, 1},
        {"__builtin_wmq_poke", 0x0F, 0xF3, 2},
        {"__builtin_wmq_connect_idx", 0x0F, 0xDE, 1},
        {"__builtin_wmq_keepalive", 0x0F, 0xDC, 1},
        {"__builtin_wmq_auth_idx", 0x0F, 0xD9, 1},
        {"__builtin_wmq_timeout_idx", 0x0F, 0xD8, 1},
        {"__builtin_wmq_key_idx", 0x0F, 0xD7, 1},
        {"__builtin_wmq_peer_idx", 0x0F, 0xD4, 1},
        {"__builtin_wmq_keycodes", 0x0F, 0xD3, 2},
        {"__builtin_wmq_load_stream", 0x0F, 0xD2, 1}
    };
    int param_table_size = sizeof(param_table) / sizeof(param_table[0]);

    for (int i = 0; i < param_table_size; i++) {
        const char *p = source_code;
        while ((p = strstr(p, param_table[i].name)) != NULL) {
            if (call_count < 128) {
                int val1 = 0, val2 = 0;
                const char *args = strchr(p, '(');
                if (args) {
                    args++;
                    val1 = atoi(args);
                    if (param_table[i].arg_count == 2) {
                        const char *comma = strchr(args, ',');
                        if (comma) {
                            comma++;
                            val2 = atoi(comma);
                        }
                    }
                }
                calls[call_count++] = (BuiltinCall){p, param_table[i].op1, param_table[i].op2, param_table[i].arg_count, val1, val2};
            }
            p++;
        }
    }

    // Sort calls chronologically by order of appearance in source code
    qsort(calls, call_count, sizeof(BuiltinCall), compare_calls);

    // Phase 1: Compile-Time JCL Pre-Filtering (ANALYZER)
    uint64_t prohibited_opcodes[4] = {0ULL};
    if (strstr(source_code, "// audit_prohibit")) {
        uint8_t op_val = 0xD9;
        prohibited_opcodes[op_val / 64] |= (1ULL << (op_val % 64));
    }
    for (int i = 0; i < call_count; i++) {
        uint8_t op = calls[i].op2;
        uint64_t bit = (prohibited_opcodes[op / 64] >> (op % 64)) & 1ULL;
        if (bit != 0ULL) {
            printf("[ANALYZER] Compile abort: prohibited opcode 0x%02X detected.\n", op);
            return false;
        }
    }

    // Emit sorted instructions
    size_t offset = 0;
    for (int i = 0; i < call_count; i++) {
        if (offset + 10 >= max_len) return false;
        out_bytecode[offset++] = calls[i].op1;
        out_bytecode[offset++] = calls[i].op2;
        if (calls[i].arg_count >= 1) {
            out_bytecode[offset++] = (uint8_t)calls[i].val1;
        }
        if (calls[i].arg_count >= 2) {
            out_bytecode[offset++] = (uint8_t)(calls[i].val2 & 0xFF);
            out_bytecode[offset++] = (uint8_t)((calls[i].val2 >> 8) & 0xFF);
            out_bytecode[offset++] = (uint8_t)((calls[i].val2 >> 16) & 0xFF);
            out_bytecode[offset++] = (uint8_t)((calls[i].val2 >> 24) & 0xFF);
        }
    }

    // Emit x86 machine instructions: MOV EAX, imm32
    if (offset + 6 >= max_len) return false;
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

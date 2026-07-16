#include "lau_yul_thunk_internal.h"
#include "lau_memory.h"
#include "tsfi_qing_bst.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

_Thread_local YulEvmContext g_yul_evm_context;

static uint64_t g_dyn_addr_counter = 0x9000000000000000ULL;

static pthread_mutex_t g_thunk_execute_mutex = PTHREAD_MUTEX_INITIALIZER;

uint64_t g_thunk_cache_hits = 0;
uint64_t g_thunk_cache_lookups = 0;

typedef struct {
    uint64_t pc;
    uint8_t op;
} YulTraceStep;

#define TRACE_HISTORY_SIZE 32
YulTraceStep s_yul_trace_history[TRACE_HISTORY_SIZE];
uint32_t s_yul_trace_count = 0;
uint32_t s_yul_trace_head = 0;
pthread_mutex_t s_yul_trace_mutex = PTHREAD_MUTEX_INITIALIZER;


static u256_t load_calldata_32(YulEvmContext *ctx, uint64_t offset) {
    u256_t r = {{0}};
    for (int i = 0; i < 32; i++) {
        uint64_t idx = offset + i;
        uint8_t byte = (idx < ctx->calldatasize) ? ctx->calldata[idx] : 0;
        r.d[3 - (i / 8)] |= ((uint64_t)byte) << (8 * (7 - (i % 8)));
    }
    return r;
}

static u256_t load_memory_32(YulEvmContext *ctx, uint64_t offset) {
    u256_t r = {{0}};
    if (offset + 32 > 524288) return r;
    for (int i = 0; i < 32; i++) {
        uint8_t byte = ctx->memory[offset + i];
        r.d[3 - (i / 8)] |= ((uint64_t)byte) << (8 * (7 - (i % 8)));
    }
    return r;
}

static void store_memory_32(YulEvmContext *ctx, uint64_t offset, u256_t val) {
    if (offset + 32 > 524288) return;
    for (int i = 0; i < 32; i++) {
        uint8_t byte = (uint8_t)(val.d[3 - (i / 8)] >> (8 * (7 - (i % 8))));
        ctx->memory[offset + i] = byte;
    }
}

static bool is_jumpdest(const uint8_t *bytecode, size_t size, uint64_t pc) {
    return pc < size && bytecode[pc] == 0x5b;
}

static void sync_raw_frame_to_yul_memory(uint8_t *memory) {
    static int raw_fd = -1;
    static uint8_t *raw_map = NULL;
    static bool raw_checked = false;
    if (!raw_checked) {
        raw_fd = open("/dev/shm/atropa_raw_frame.bin", O_RDONLY);
        if (raw_fd != -1) {
            struct stat st;
            if (fstat(raw_fd, &st) == 0 && st.st_size >= 256000) {
                raw_map = mmap(NULL, 256000, PROT_READ, MAP_SHARED, raw_fd, 0);
            }
        }
        raw_checked = true;
    }
    if (raw_map && raw_map != MAP_FAILED) {
        memcpy(memory + 131072, raw_map, 256000);
    }
}

// EVM execution loop
bool run_yul_bytecode(YulEvmContext *ctx, const uint8_t *bytecode, size_t size, const char *name) {
    (void)name;
    uint64_t pc = 0;
    ctx->reverted = false;
    ctx->stack_ptr = 0;
    sync_raw_frame_to_yul_memory(ctx->memory);

    while (pc < size) {
        uint8_t op = bytecode[pc];
        if (name && strcmp(name, "cpu6502") != 0) {
            pthread_mutex_lock(&s_yul_trace_mutex);
            s_yul_trace_history[s_yul_trace_head].pc = pc;
            s_yul_trace_history[s_yul_trace_head].op = op;
            s_yul_trace_head = (s_yul_trace_head + 1) % TRACE_HISTORY_SIZE;
            if (s_yul_trace_count < TRACE_HISTORY_SIZE) {
                s_yul_trace_count++;
            }
            pthread_mutex_unlock(&s_yul_trace_mutex);
        }
        if (op >= 0x60 && op <= 0x7f) { // PUSH1..PUSH32
            int push_bytes = op - 0x5f;
            u256_t val = {{0}};
            for (int i = 0; i < push_bytes; i++) {
                uint64_t idx = pc + 1 + i;
                uint8_t byte = (idx < size) ? bytecode[idx] : 0;
                val.d[3 - (i / 8)] |= ((uint64_t)byte) << (8 * (7 - (i % 8)));
            }
            // Right justify the value
            int shift_bits = (32 - push_bytes) * 8;
            val = u256_shr(val, shift_bits);
            
            if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at PUSH\n"); return false; }
            ctx->stack[ctx->stack_ptr++] = val;
            pc += 1 + push_bytes;
            continue;
        }

        if (op >= 0x80 && op <= 0x8f) { // DUP1..DUP16
            int dup_idx = op - 0x7f;
            if (ctx->stack_ptr < dup_idx) { printf("[DEBUG_EVM] Stack underflow at DUP\n"); return false; }
            if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at DUP\n"); return false; }
            ctx->stack[ctx->stack_ptr] = ctx->stack[ctx->stack_ptr - dup_idx];
            ctx->stack_ptr++;
            pc++;
            continue;
        }

        if (op >= 0x90 && op <= 0x9f) { // SWAP1..SWAP16
            int swap_idx = op - 0x8f;
            if (ctx->stack_ptr < swap_idx + 1) { printf("[DEBUG_EVM] Stack underflow at SWAP\n"); return false; }
            u256_t tmp = ctx->stack[ctx->stack_ptr - 1];
            ctx->stack[ctx->stack_ptr - 1] = ctx->stack[ctx->stack_ptr - 1 - swap_idx];
            ctx->stack[ctx->stack_ptr - 1 - swap_idx] = tmp;
            pc++;
            continue;
        }

        switch (op) {
            case 0x00: // STOP
                return true;
            case 0x01: { // ADD
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at ADD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_add(a, b);
                break;
            }
            case 0x02: { // MUL
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MUL\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mul(a, b);
                break;
            }
            case 0x03: { // SUB
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SUB\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_sub(a, b);
                break;
            }
            case 0x04: { // DIV
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at DIV\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_div(a, b);
                break;
            }
            case 0x05: { // SDIV
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SDIV\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_sdiv(a, b);
                break;
            }
            case 0x06: { // MOD
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mod(a, b);
                break;
            }
            case 0x07: { // SMOD
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_smod(a, b);
                break;
            }
            case 0x08: { // ADDMOD
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at ADDMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[--ctx->stack_ptr];
                u256_t N = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_addmod(a, b, N);
                break;
            }
            case 0x09: { // MULMOD
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at MULMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[--ctx->stack_ptr];
                u256_t N = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mulmod(a, b, N);
                break;
            }
            case 0x0a: { // EXP
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at EXP\n"); return false; }
                u256_t base = ctx->stack[--ctx->stack_ptr];
                u256_t exponent = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_exp(base, exponent);
                break;
            }
            case 0x0b: { // SIGNEXTEND
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SIGNEXTEND\n"); return false; }
                u256_t k = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_signextend(val, k);
                break;
            }
            case 0x10: { // LT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at LT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_lt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x11: { // GT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at GT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_gt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x12: { // SLT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SLT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ (uint64_t)u256_slt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x13: { // SGT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SGT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ (uint64_t)u256_sgt(a, b), 0, 0, 0 }};
                break;
            }
            case 0x14: { // EQ
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at EQ\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_eq(a, b), 0, 0, 0 }};
                break;
            }
            case 0x15: { // ISZERO
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at ISZERO\n"); return false; }
                u256_t a = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = (u256_t){{ u256_iszero(a), 0, 0, 0 }};
                break;
            }
            case 0x16: { // AND
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at AND\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_and(a, b);
                break;
            }
            case 0x17: { // OR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at OR\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_or(a, b);
                break;
            }
            case 0x18: { // XOR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at XOR\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_xor(a, b);
                break;
            }
            case 0x19: { // NOT
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at NOT\n"); return false; }
                u256_t a = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_not(a);
                break;
            }
            case 0x1a: { // BYTE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at BYTE\n"); return false; }
                u256_t pos = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                u256_t res = {{0}};
                if (pos.d[0] < 32) {
                    int byte_idx = 31 - pos.d[0];
                    uint8_t byte = (uint8_t)(val.d[byte_idx / 8] >> (8 * (byte_idx % 8)));
                    res.d[0] = byte;
                }
                ctx->stack[ctx->stack_ptr - 1] = res;
                break;
            }
            case 0x1b: { // SHL
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHL\n"); return false; }
                u256_t shift = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_shl(val, shift.d[0]);
                break;
            }
            case 0x1c: { // SHR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHR\n"); return false; }
                u256_t shift = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_shr(val, shift.d[0]);
                break;
            }
            case 0x1d: { // SAR
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SAR\n"); return false; }
                u256_t shift = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[ctx->stack_ptr - 1];
                u256_t res = u256_shr(val, shift.d[0]);
                if (shift.d[0] < 256 && ((val.d[3] >> 63) & 1)) {
                    // Sign extend arithmetic shift right
                    for (int b = 255 - shift.d[0]; b < 256; b++) {
                        res.d[b / 64] |= (1ULL << (b % 64));
                    }
                } else if (shift.d[0] >= 256) {
                    res = ((val.d[3] >> 63) & 1) ? u256_not((u256_t){{0}}) : (u256_t){{0}};
                }
                ctx->stack[ctx->stack_ptr - 1] = res;
                break;
            }
            case 0x20: { // SHA3 / KECCAK256
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHA3\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[ctx->stack_ptr - 1];
                u256_t res = {{0}};
                // Mock simple hash generator logic based on input bounds
                uint64_t hash = 5381;
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    if (offset.d[0] + i < 524288) {
                        hash = ((hash << 5) + hash) + ctx->memory[offset.d[0] + i];
                    }
                }
                res.d[0] = hash;
                res.d[1] = hash ^ 0xDEADBEEF;
                res.d[2] = hash ^ 0xCAFEBABE;
                res.d[3] = hash ^ 0xBAADF00D;
                ctx->stack[ctx->stack_ptr - 1] = res;
                break;
            }
            case 0x30: { // ADDRESS
                u256_t r = {{0}};
                r.d[0] = ctx->self_address;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at ADDRESS\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x32: { // ORIGIN
                u256_t r = {{ 0x4cc, 0, 0, 0 }};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at ORIGIN\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x33: { // CALLER
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLER\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = ctx->caller_address;
                break;
            }
            case 0x34: { // CALLVALUE
                u256_t r = {{0}}; // always 0 for thunks
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLVALUE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x35: { // CALLDATALOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at CALLDATALOAD\n"); return false; }
                u256_t offset = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = load_calldata_32(ctx, offset.d[0]);
                break;
            }
            case 0x36: { // CALLDATASIZE
                u256_t r = {{0}};
                r.d[0] = ctx->calldatasize;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLDATASIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x37: { // CALLDATACOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CALLDATACOPY\n"); return false; }
                u256_t dest_offset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    uint64_t src_idx = offset.d[0] + i;
                    uint64_t dest_idx = dest_offset.d[0] + i;
                    if (dest_idx < 524288) {
                        ctx->memory[dest_idx] = (src_idx < ctx->calldatasize) ? ctx->calldata[src_idx] : 0;
                    }
                }
                break;
            }
            case 0x38: { // CODESIZE
                u256_t r = {{size, 0, 0, 0}};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CODESIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x39: { // CODECOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CODECOPY\n"); return false; }
                u256_t destOffset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t len = ctx->stack[--ctx->stack_ptr];
                
                uint64_t doff = destOffset.d[0];
                uint64_t src_off = offset.d[0];
                uint64_t l = len.d[0];
                
                for (uint64_t i = 0; i < l; i++) {
                    uint8_t val = 0;
                    if (src_off + i < size) {
                        val = bytecode[src_off + i];
                    }
                    if (doff + i < 524288) {
                        ctx->memory[doff + i] = val;
                    }
                }
                break;
            }
            case 0x3d: { // RETURNDATASIZE
                u256_t r = {{0}};
                r.d[0] = ctx->return_size;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at RETURNDATASIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x3e: { // RETURNDATACOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at RETURNDATACOPY\n"); return false; }
                u256_t dest_offset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    uint64_t src_idx = offset.d[0] + i;
                    uint64_t dest_idx = dest_offset.d[0] + i;
                    if (dest_idx < 524288) {
                        ctx->memory[dest_idx] = (src_idx < ctx->return_size) ? ctx->return_data[src_idx] : 0;
                    }
                }
                break;
            }
            case 0x50: { // POP
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at POP\n"); return false; }
                ctx->stack_ptr--;
                break;
            }
            case 0x51: { // MLOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at MLOAD\n"); return false; }
                u256_t offset = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = load_memory_32(ctx, offset.d[0]);
                break;
            }
            case 0x52: { // MSTORE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MSTORE\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                store_memory_32(ctx, offset.d[0], val);
                break;
            }
            case 0x53: { // MSTORE8
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at MSTORE8\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                if (offset.d[0] < 524288) {
                    ctx->memory[offset.d[0]] = (uint8_t)(val.d[0] & 0xFF);
                }
                break;
            }
            case 0x54: { // SLOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at SLOAD\n"); return false; }
                u256_t key = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = context_sload(ctx, key);
                break;
            }
            case 0x55: { // SSTORE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SSTORE\n"); return false; }
                u256_t key = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                context_sstore(ctx, key, val);
                extern void lau_yul_thunk_cache_invalidate(const char *name);
                lau_yul_thunk_cache_invalidate(name);
                break;
            }
            case 0x56: { // JUMP
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at JUMP\n"); return false; }
                u256_t dest = ctx->stack[--ctx->stack_ptr];
                if (!is_jumpdest(bytecode, size, dest.d[0])) {
                    printf("[DEBUG_EVM] Invalid JUMP destination: %lu\n", dest.d[0]);
                    return false;
                }
                pc = dest.d[0];
                continue;
            }
            case 0x57: { // JUMPI
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at JUMPI\n"); return false; }
                u256_t dest = ctx->stack[--ctx->stack_ptr];
                u256_t cond = ctx->stack[--ctx->stack_ptr];
                if (!u256_iszero(cond)) {
                    if (!is_jumpdest(bytecode, size, dest.d[0])) {
                        printf("[DEBUG_EVM] Invalid JUMPI destination: %lu\n", dest.d[0]);
                        return false;
                    }
                    pc = dest.d[0];
                    continue;
                }
                break;
            }
            case 0x5b: // JUMPDEST
                break;
            case 0x5f: { // PUSH0
                u256_t r = {{0}};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at PUSH0\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0xf3: { // RETURN
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at RETURN\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                ctx->return_size = length.d[0] < 524288 ? length.d[0] : 524288;
                if (offset.d[0] < 524288) {
                     size_t avail = 524288 - offset.d[0];
                     if (ctx->return_size > avail) ctx->return_size = avail;
                     memcpy(ctx->return_data, ctx->memory + offset.d[0], ctx->return_size);
                } else {
                     ctx->return_size = 0;
                }
                return true;
            }
            case 0xfd: { // REVERT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at REVERT\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                ctx->return_size = length.d[0] < 524288 ? length.d[0] : 524288;
                if (offset.d[0] < 524288) {
                     size_t avail = 524288 - offset.d[0];
                     if (ctx->return_size > avail) ctx->return_size = avail;
                     memcpy(ctx->return_data, ctx->memory + offset.d[0], ctx->return_size);
                } else {
                     ctx->return_size = 0;
                }
                printf("[DEBUG_EVM] REVERT triggered. Data (len %zu): ", ctx->return_size);
                for (size_t i = 0; i < (ctx->return_size < 64 ? ctx->return_size : 64); i++) {
                     printf("%02x", ctx->return_data[i]);
                }
                printf("\n");
                ctx->reverted = true;
                return false;
            }
            case 0xfe: { // INVALID
                printf("[DEBUG_EVM] Invalid opcode 0xfe executed\n");
                return false;
            }
            case 0xa0: // LOG0
            case 0xa1: // LOG1
            case 0xa2: // LOG2
            case 0xa3: // LOG3
            case 0xa4: { // LOG4
                int num_topics = op - 0xa0;
                if (ctx->stack_ptr < 2 + num_topics) { printf("[DEBUG_EVM] Stack underflow at LOG\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                if (ctx->log_count < 64) {
                    YulEventLog *log = &ctx->logs[ctx->log_count++];
                    log->address = ctx->self_address;
                    log->num_topics = num_topics;
                    for (int i = 0; i < num_topics; i++) {
                        log->topics[i] = ctx->stack[--ctx->stack_ptr];
                    }
                    log->data_size = length.d[0] < 2048 ? length.d[0] : 2048;
                    if (offset.d[0] < 524288) {
                        size_t avail = 524288 - offset.d[0];
                        if (log->data_size > avail) log->data_size = avail;
                        memcpy(log->data, ctx->memory + offset.d[0], log->data_size);
                    } else {
                        log->data_size = 0;
                    }
                } else {
                    // discard log, but pop stack topics
                    ctx->stack_ptr -= num_topics;
                }
                break;
            }
            case 0xf1: // CALL
            case 0xf2: // CALLCODE
            case 0xf4: // DELEGATECALL
            case 0xfa: { // STATICCALL
                bool is_delegate = (op == 0xf4);
                bool is_static = (op == 0xfa);
                int expected_stack = (is_delegate || is_static) ? 6 : 7;
                if (ctx->stack_ptr < expected_stack) { printf("[DEBUG_EVM] Stack underflow at CALL/DELEGATE/STATICCALL\n"); return false; }
                
                ctx->stack_ptr--; // discard gas limit
                u256_t target_addr_u256 = ctx->stack[--ctx->stack_ptr];
                uint64_t target_addr = target_addr_u256.d[0];
                
                if (!is_delegate && !is_static) {
                    ctx->stack_ptr--; // discard call value
                }
                
                u256_t argsOffset = ctx->stack[--ctx->stack_ptr];
                u256_t argsSize = ctx->stack[--ctx->stack_ptr];
                u256_t retOffset = ctx->stack[--ctx->stack_ptr];
                u256_t retSize = ctx->stack[--ctx->stack_ptr];
                
                u256_t success_out = {{0}};
                bool call_ok = execute_nested_call(ctx, target_addr, argsOffset.d[0], argsSize.d[0], retOffset.d[0], retSize.d[0], &success_out);
                (void)call_ok;
                ctx->stack[ctx->stack_ptr++] = success_out;
                break;
            }
            case 0x3b: { // EXTCODESIZE
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at EXTCODESIZE\n"); return false; }
                u256_t addr_val = ctx->stack[--ctx->stack_ptr];
                uint64_t search_addr = addr_val.d[0];
                size_t code_sz = 0;
                 tsfi_qing_bst_populate_safe();
                 CachedContract *c_match = tsfi_qing_bst_find_safe(search_addr);
                 if (c_match) {
                     code_sz = c_match->size;
                 }
                u256_t sz_val = {{ code_sz, 0, 0, 0 }};
                ctx->stack[ctx->stack_ptr++] = sz_val;
                break;
            }
            case 0x5a: { // GAS
                u256_t gas_val = {{ 1000000000ULL, 0, 0, 0 }};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at GAS\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = gas_val;
                break;
            }
            case 0xf0: { // CREATE
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CREATE\n"); return false; }
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                (void)value;

                uint8_t *initcode = NULL;
                size_t initcode_len = length.d[0];
                if (initcode_len > 0) {
                    initcode = malloc(initcode_len);
                    if (initcode) {
                        for (size_t i = 0; i < initcode_len; i++) {
                            uint64_t src = offset.d[0] + i;
                            initcode[i] = (src < 524288) ? ctx->memory[src] : 0;
                        }
                    }
                }

                uint64_t new_address = ++g_dyn_addr_counter;

                u256_t ret_addr = {{0}};
                if (initcode && initcode_len > 0) {
                    YulEvmContext *nested_ctx = calloc(1, sizeof(YulEvmContext));
                    if (nested_ctx) {
                        nested_ctx->self_address = new_address;
                        nested_ctx->caller_address.d[0] = ctx->self_address;
                        nested_ctx->storage_count = ctx->storage_count;
                        memcpy(nested_ctx->storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
                        memcpy(nested_ctx->storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
                        memcpy(nested_ctx->storage_addrs, ctx->storage_addrs, sizeof(ctx->storage_addrs));
                        nested_ctx->is_initcode = true;

                        bool success = run_yul_bytecode(nested_ctx, initcode, initcode_len, "dynamic");
                        if (success && !nested_ctx->reverted && nested_ctx->return_size > 0) {
                            if (g_cached_contracts_count < MAX_CACHED_CONTRACTS) {
                                CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
                                snprintf(c->name, sizeof(c->name), "dynamic_%lx", new_address);
                                c->bytecode = malloc(nested_ctx->return_size);
                                memcpy(c->bytecode, nested_ctx->return_data, nested_ctx->return_size);
                                c->size = nested_ctx->return_size;
                                c->virtual_address = new_address;
                                snprintf(c->path, sizeof(c->path), "dynamic");
                                ret_addr.d[0] = new_address;
                                tsfi_qing_bst_invalidate_safe();

                                for (int i = 0; i < nested_ctx->storage_count; i++) {
                                    u256_t raw_key = nested_ctx->storage_keys[i];
                                    bool found = false;
                                    for (int j = 0; j < ctx->storage_count; j++) {
                                        if (u256_eq(ctx->storage_keys[j], raw_key)) {
                                            ctx->storage_vals[j] = nested_ctx->storage_vals[i];
                                            ctx->storage_addrs[j] = nested_ctx->storage_addrs[i];
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found && ctx->storage_count < 32768) {
                                        ctx->storage_keys[ctx->storage_count] = raw_key;
                                        ctx->storage_vals[ctx->storage_count] = nested_ctx->storage_vals[i];
                                        ctx->storage_addrs[ctx->storage_count] = nested_ctx->storage_addrs[i];
                                        ctx->storage_count++;
                                    }
                                }
                            }
                        }
                        free(nested_ctx);
                    }
                    free(initcode);
                }

                ctx->stack[ctx->stack_ptr++] = ret_addr;
                break;
            }
            case 0xf5: { // CREATE2
                if (ctx->stack_ptr < 4) { printf("[DEBUG_EVM] Stack underflow at CREATE2\n"); return false; }
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                u256_t salt = ctx->stack[--ctx->stack_ptr];
                (void)value; (void)salt;

                uint8_t *initcode = NULL;
                size_t initcode_len = length.d[0];
                if (initcode_len > 0) {
                    initcode = malloc(initcode_len);
                    if (initcode) {
                        for (size_t i = 0; i < initcode_len; i++) {
                            uint64_t src = offset.d[0] + i;
                            initcode[i] = (src < 524288) ? ctx->memory[src] : 0;
                        }
                    }
                }

                uint64_t new_address = ++g_dyn_addr_counter;

                u256_t ret_addr = {{0}};
                if (initcode && initcode_len > 0) {
                    YulEvmContext *nested_ctx = calloc(1, sizeof(YulEvmContext));
                    if (nested_ctx) {
                        nested_ctx->self_address = new_address;
                        nested_ctx->caller_address.d[0] = ctx->self_address;
                        nested_ctx->storage_count = ctx->storage_count;
                        memcpy(nested_ctx->storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
                        memcpy(nested_ctx->storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
                        memcpy(nested_ctx->storage_addrs, ctx->storage_addrs, sizeof(ctx->storage_addrs));
                        nested_ctx->is_initcode = true;

                        bool success = run_yul_bytecode(nested_ctx, initcode, initcode_len, "dynamic");
                        if (success && !nested_ctx->reverted && nested_ctx->return_size > 0) {
                            if (g_cached_contracts_count < MAX_CACHED_CONTRACTS) {
                                CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
                                snprintf(c->name, sizeof(c->name), "dynamic_%lx", new_address);
                                c->bytecode = malloc(nested_ctx->return_size);
                                memcpy(c->bytecode, nested_ctx->return_data, nested_ctx->return_size);
                                c->size = nested_ctx->return_size;
                                c->virtual_address = new_address;
                                snprintf(c->path, sizeof(c->path), "dynamic");
                                ret_addr.d[0] = new_address;
                                tsfi_qing_bst_invalidate_safe();

                                for (int i = 0; i < nested_ctx->storage_count; i++) {
                                    u256_t raw_key = nested_ctx->storage_keys[i];
                                    bool found = false;
                                    for (int j = 0; j < ctx->storage_count; j++) {
                                        if (u256_eq(ctx->storage_keys[j], raw_key)) {
                                            ctx->storage_vals[j] = nested_ctx->storage_vals[i];
                                            ctx->storage_addrs[j] = nested_ctx->storage_addrs[i];
                                            found = true;
                                            break;
                                        }
                                    }
                                    if (!found && ctx->storage_count < 32768) {
                                        ctx->storage_keys[ctx->storage_count] = raw_key;
                                        ctx->storage_vals[ctx->storage_count] = nested_ctx->storage_vals[i];
                                        ctx->storage_addrs[ctx->storage_count] = nested_ctx->storage_addrs[i];
                                        ctx->storage_count++;
                                    }
                                }
                            }
                        }
                        free(nested_ctx);
                    }
                    free(initcode);
                }

                ctx->stack[ctx->stack_ptr++] = ret_addr;
                break;
            }
            default:
                printf("[DEBUG_EVM] Unsupported opcode: 0x%02X at PC %zu\n", op, pc);
                return false;
        }
        pc++;
    }
    return true;
}

bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len) {
    // Passthrough to execute the full Yul VM state machine for WinchesterMQ to keep state compatibility with SCSI tests
    pthread_mutex_lock(&g_thunk_execute_mutex);
    CachedContract *c = NULL;
    uint64_t search_addr = 0;
    bool is_addr = false;
    if (name[0] == '0' && (name[1] == 'x' || name[1] == 'X')) {
        sscanf(name + 2, "%lx", &search_addr);
        is_addr = true;
    } else if (name[0] >= '0' && name[0] <= '9') {
        search_addr = strtoull(name, NULL, 0);
        is_addr = true;
    }

    tsfi_qing_bst_populate_safe();
    if (is_addr) {
        c = tsfi_qing_bst_find_safe(search_addr);
    } else {
        for (int i = 0; i < g_cached_contracts_count; i++) {
            if (strcmp(g_cached_contracts[i].name, name) == 0) {
                c = &g_cached_contracts[i];
                break;
            }
        }
    }

    if (!c) {
        printf("[YUL_THUNK] Error: Contract %s not initialized. Registered contracts count: %d\n", name, g_cached_contracts_count);
        for (int k = 0; k < g_cached_contracts_count; k++) {
            printf("  - %s (addr: 0x%lx)\n", g_cached_contracts[k].name, g_cached_contracts[k].virtual_address);
        }
        pthread_mutex_unlock(&g_thunk_execute_mutex);
        return false;
    }

    if (!g_cache_rehydrated) {
        g_cache_rehydrated = true;
        extern void lau_yul_thunk_cache_rehydrate(void);
        lau_yul_thunk_cache_rehydrate();
    }

    if (s_execution_call_stack_depth > 0 && s_execution_call_stack_depth < 16) {
        register_dependency(s_execution_call_stack[s_execution_call_stack_depth - 1], name);
    }
    if (s_execution_call_stack_depth < 16) {
        strncpy(s_execution_call_stack[s_execution_call_stack_depth++], name, 63);
    }

    uint64_t hash = bst_fnv1a_hash(name, calldata, calldatasize);

    bool skip_cache = (strcmp(name, "cpu6502") == 0 || strcmp(name, "WinchesterMQ") == 0);
    bool cache_hit = false;
    if (!skip_cache) {
        pthread_mutex_lock(&s_thunk_memo_bst_mutex);
        g_thunk_cache_lookups++;
        LauMemoNode *found = bst_find(s_thunk_memo_bst_root, hash);
        if (found &&
            strcmp(found->contract_name, name) == 0 &&
            found->calldatasize == calldatasize &&
            memcmp(found->calldata_ptr, calldata, calldatasize) == 0) {
            
            // Resolve wired header parameters from the allocated lau_memory payload
            bool meta_match = true;
            if (found->retval_ptr) {
                LauWiredHeader *h = (LauWiredHeader*)((char*)found->retval_ptr - 8192);
                if (h) {
                    // Filter: Require sealed results, version matching system parameters
                    if (!h->sealed || h->version != 1 || h->system_id != 42) {
                        meta_match = false;
                    }
                }
            }
            
            if (meta_match) {
                g_thunk_cache_hits++;
                found->cache_hits++;
                found->ttl_ms += 1000;
                if (found->ttl_ms > 30000) found->ttl_ms = 30000;
                found->created_time_ms = current_time_ms();
                if (retval && retval_len) {
                    size_t out_len = found->retval_len;
                    if (*retval_len < out_len) out_len = *retval_len;
                    memcpy(retval, found->retval_ptr, out_len);
                    *retval_len = out_len;
                }
                cache_hit = true;
            }
        }
        pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
        if (cache_hit) {
            if (s_execution_call_stack_depth > 0) s_execution_call_stack_depth--;
            pthread_mutex_unlock(&g_thunk_execute_mutex);
            return true;
        }
    }

    extern bool g_storage_dirty;
    if (g_storage_dirty) {
        extern void persist_reconciliation_data(void);
        persist_reconciliation_data();
        g_storage_dirty = false;
    }
    g_transaction_diyat_tax_total = 0;
    g_yul_evm_context.log_count = 0;

    memset(g_yul_evm_context.calldata, 0, sizeof(g_yul_evm_context.calldata));
    size_t size_to_copy = calldatasize < sizeof(g_yul_evm_context.calldata) ? calldatasize : sizeof(g_yul_evm_context.calldata);
    memcpy(g_yul_evm_context.calldata, calldata, size_to_copy);
    g_yul_evm_context.calldatasize = size_to_copy;

    uint64_t prev_self_address = g_yul_evm_context.self_address;
    g_yul_evm_context.self_address = c->virtual_address;
    g_yul_evm_context.caller_address.d[0] = 0x4cc;
    g_yul_evm_context.caller_address.d[1] = 0;
    g_yul_evm_context.caller_address.d[2] = 0;
    g_yul_evm_context.caller_address.d[3] = 0;
    g_yul_evm_context.storage_read_occurred = false;
    bool success = run_yul_bytecode(&g_yul_evm_context, c->bytecode, c->size, name);

    if (success) {
        bool had_dirty_storage = g_storage_dirty;
        if (g_storage_dirty) {
            persist_reconciliation_data();
            g_storage_dirty = false;
            extern void lau_yul_thunk_cache_invalidate_storage(void);
            lau_yul_thunk_cache_invalidate_storage();
        }
        if (retval && retval_len) {
            size_t out_size = g_yul_evm_context.return_size < *retval_len ? g_yul_evm_context.return_size : *retval_len;
            memcpy(retval, g_yul_evm_context.return_data, out_size);
            *retval_len = out_size;
        }
        
        // Cache outputs for read-only dynamic thunk evaluations (skip if storage mutations occurred)
        if (!skip_cache && !had_dirty_storage && !g_yul_evm_context.reverted) {
            pthread_mutex_lock(&s_thunk_memo_bst_mutex);
            LauMemoNode *existing = bst_find_any(s_thunk_memo_bst_root, hash);
            if (existing) {
                existing->stale = false;
                existing->created_time_ms = current_time_ms();
                existing->read_storage = g_yul_evm_context.storage_read_occurred;
                if (existing->calldata_ptr) { lau_free(existing->calldata_ptr); existing->calldata_ptr = NULL; }
                existing->calldata_ptr = lau_malloc(calldatasize);
                memcpy(existing->calldata_ptr, calldata, calldatasize);
                existing->calldatasize = calldatasize;
                if (existing->retval_ptr) { lau_free(existing->retval_ptr); existing->retval_ptr = NULL; }
                if (retval && retval_len && *retval_len > 0) {
                    existing->retval_ptr = lau_malloc_wired(*retval_len);
                    memcpy(existing->retval_ptr, retval, *retval_len);
                    existing->retval_len = *retval_len;
                    LauWiredHeader *h = (LauWiredHeader*)((char*)existing->retval_ptr - 8192);
                    if (h) {
                        h->version = 1;
                        h->system_id = 42;
                        h->sealed = true;
                    }
                } else {
                    existing->retval_ptr = NULL;
                    existing->retval_len = 0;
                }
                extern void write_thunk_to_disk(LauMemoNode *node);
                write_thunk_to_disk(existing);
            } else {
                LauMemoNode *new_node = (LauMemoNode*)lau_malloc(sizeof(LauMemoNode));
                new_node->signature_hash = hash;
                strncpy(new_node->contract_name, name, 63);
                new_node->contract_name[63] = '\0';
                new_node->calldata_ptr = lau_malloc(calldatasize);
                memcpy(new_node->calldata_ptr, calldata, calldatasize);
                new_node->calldatasize = calldatasize;
                if (retval && retval_len && *retval_len > 0) {
                    new_node->retval_ptr = lau_malloc_wired(*retval_len);
                    memcpy(new_node->retval_ptr, retval, *retval_len);
                    new_node->retval_len = *retval_len;
                    
                    // Populate known LauWiredHeader data members to assist in search & selection
                    LauWiredHeader *h = (LauWiredHeader*)((char*)new_node->retval_ptr - 8192);
                    if (h) {
                        h->version = 1;
                        h->system_id = 42;
                        h->sealed = true;
                    }
                } else {
                    new_node->retval_ptr = NULL;
                    new_node->retval_len = 0;
                }
                new_node->height = 1;
                new_node->created_time_ms = current_time_ms();
                new_node->ttl_ms = 5000;
                new_node->stale = false;
                new_node->read_storage = g_yul_evm_context.storage_read_occurred;
                new_node->cache_hits = 0;
                new_node->left = NULL;
                new_node->right = NULL;
                
                if (count_active_nodes(s_thunk_memo_bst_root) >= 64) {
                    LauMemoNode *lru_node = NULL;
                    uint64_t oldest = -1ULL;
                    evict_lru_node(s_thunk_memo_bst_root, &lru_node, &oldest);
                    if (lru_node) {
                        lru_node->stale = true;
                        if (lru_node->calldata_ptr) { lau_free(lru_node->calldata_ptr); lru_node->calldata_ptr = NULL; }
                        if (lru_node->retval_ptr) { lau_free(lru_node->retval_ptr); lru_node->retval_ptr = NULL; }
                    }
                }
                s_thunk_memo_bst_root = lau_yul_thunk_avl_insert(s_thunk_memo_bst_root, new_node);
                extern void write_thunk_to_disk(LauMemoNode *node);
                write_thunk_to_disk(new_node);
            }
            pthread_mutex_unlock(&s_thunk_memo_bst_mutex);
        }
    } else {
        if (retval_len) *retval_len = 0;
    }
    g_yul_evm_context.self_address = prev_self_address;

    if (success && strcmp(name, "cpu6502") == 0) {
        /* Simulate FET discharge decay on registers A, X, Y (addresses 128, 129, 130) */
        uint64_t val_a = lau_yul_thunk_sload(128);
        uint64_t val_x = lau_yul_thunk_sload(129);
        uint64_t val_y = lau_yul_thunk_sload(130);

        /* Apply 2% FET discharge charge decay per CPU execution loop */
        uint64_t new_a = (uint64_t)(val_a * 0.98);
        uint64_t new_x = (uint64_t)(val_x * 0.98);
        uint64_t new_y = (uint64_t)(val_y * 0.98);

        lau_yul_thunk_sstore(128, new_a);
        lau_yul_thunk_sstore(129, new_x);
        lau_yul_thunk_sstore(130, new_y);
    }

    if (s_execution_call_stack_depth > 0) s_execution_call_stack_depth--;
    pthread_mutex_unlock(&g_thunk_execute_mutex);
    return success;
}

void lau_yul_thunk_sstore(uint64_t key, uint64_t value) {
    u256_t key_u256 = {{0}};
    key_u256.d[0] = key;
    u256_t val = {{0}};
    val.d[0] = value;
    uint64_t prev_addr = g_yul_evm_context.self_address;
    if (key >= 0xF000) {
        g_yul_evm_context.self_address = 0x200;
    }
    context_sstore(&g_yul_evm_context, key_u256, val);
    g_yul_evm_context.self_address = prev_addr;
    extern void lau_yul_thunk_cache_invalidate_storage(void);
    lau_yul_thunk_cache_invalidate_storage();
}

uint64_t lau_yul_thunk_sload(uint64_t key) {
    u256_t key_u256 = {{0}};
    key_u256.d[0] = key;
    uint64_t prev_addr = g_yul_evm_context.self_address;
    if (key >= 0xF000) {
        g_yul_evm_context.self_address = 0x200;
    }
    u256_t val = context_sload(&g_yul_evm_context, key_u256);
    g_yul_evm_context.self_address = prev_addr;
    return val.d[0];
}


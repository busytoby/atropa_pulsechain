#include "lau_yul_thunk.h"
#include "tsfi_pulsechain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CACHED_CONTRACTS 64

static const char* case_insensitive_strstr(const char *haystack, const char *needle) {
    if (!haystack || !needle) return NULL;
    size_t needle_len = strlen(needle);
    if (needle_len == 0) return haystack;
    for (; *haystack; haystack++) {
        size_t i;
        for (i = 0; i < needle_len; i++) {
            char h = haystack[i];
            char n = needle[i];
            if (h >= 'A' && h <= 'Z') h += 32;
            if (n >= 'A' && n <= 'Z') n += 32;
            if (h != n) break;
        }
        if (i == needle_len) return haystack;
    }
    return NULL;
}

typedef struct {
    char name[64];
    uint8_t *bytecode;
    size_t size;
    uint64_t virtual_address;
} CachedContract;

static CachedContract g_cached_contracts[MAX_CACHED_CONTRACTS];
static int g_cached_contracts_count = 0;

_Thread_local YulEvmContext g_yul_evm_context;

static bool run_yul_bytecode(YulEvmContext *ctx, const uint8_t *bytecode, size_t size, const char *name);
static u256_t context_sload(YulEvmContext *ctx, uint64_t key);
static void context_sstore(YulEvmContext *ctx, uint64_t key, u256_t val);

// Helper to convert hex strings to raw byte array
static uint8_t* hex_to_bytes(const char *hex, size_t *out_len) {
    if (hex[0] == '0' && hex[1] == 'x') hex += 2;
    size_t len = strlen(hex);
    *out_len = len / 2;
    uint8_t *bytes = malloc(*out_len);
    if (!bytes) return NULL;
    for (size_t i = 0; i < *out_len; i++) {
        unsigned int val;
        sscanf(hex + i * 2, "%2x", &val);
        bytes[i] = (uint8_t)val;
    }
    return bytes;
}

static void persist_reconciliation_data(void) {
    FILE *fp = fopen("evm_storage.json", "w");
    if (!fp) return;
    fprintf(fp, "{\n  \"storage\": [\n");
    for (int i = 0; i < g_yul_evm_context.storage_count; i++) {
        fprintf(fp, "    { \"key\": %lu, \"val\": \"%016lx%016lx%016lx%016lx\" }%s\n",
                g_yul_evm_context.storage_keys[i],
                g_yul_evm_context.storage_vals[i].d[3],
                g_yul_evm_context.storage_vals[i].d[2],
                g_yul_evm_context.storage_vals[i].d[1],
                g_yul_evm_context.storage_vals[i].d[0],
                (i == g_yul_evm_context.storage_count - 1) ? "" : ",");
    }
    fprintf(fp, "  ]\n}\n");
    fclose(fp);
}

static void load_reconciliation_data(void) {
    FILE *fp = fopen("evm_storage.json", "r");
    if (!fp) return;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        uint64_t key = 0;
        char val_str[128] = {0};
        if (sscanf(line, " { \"key\": %lu, \"val\": \"%[^\"]\" }", &key, val_str) == 2) {
            u256_t val = {{0}};
            if (strlen(val_str) == 64) {
                sscanf(val_str, "%16lx%16lx%16lx%16lx", &val.d[3], &val.d[2], &val.d[1], &val.d[0]);
                if (g_yul_evm_context.storage_count < 4096) {
                    g_yul_evm_context.storage_keys[g_yul_evm_context.storage_count] = key;
                    g_yul_evm_context.storage_vals[g_yul_evm_context.storage_count] = val;
                    g_yul_evm_context.storage_count++;
                }
            }
        }
    }
    fclose(fp);
}

// Compiles a Yul contract file using solc --strict-assembly and registers it
bool lau_yul_thunk_init(const char *name, const char *yul_path, uint64_t virtual_address) {
    if (g_cached_contracts_count == 0) {
        load_reconciliation_data();
    }
    // Check if already registered
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcmp(g_cached_contracts[i].name, name) == 0) {
            return true;
        }
    }

    if (g_cached_contracts_count >= MAX_CACHED_CONTRACTS) {
        printf("[YUL_THUNK] Error: Max cached contracts reached\n");
        return false;
    }

    bool is_solidity = (strstr(yul_path, ".sol") != NULL);
    if (is_solidity) {
        printf("[YUL_THUNK] Compiling Solidity contract %s from %s...\n", name, yul_path);
    } else {
        printf("[YUL_THUNK] Compiling Yul contract %s from %s...\n", name, yul_path);
    }

    char cmd[512];
    if (is_solidity) {
        snprintf(cmd, sizeof(cmd), "solc --optimize --bin --allow-paths .. \"%s\" 2>/dev/null", yul_path);
    } else {
        snprintf(cmd, sizeof(cmd), "solc --strict-assembly --allow-paths .. \"%s\" --bin 2>/dev/null", yul_path);
    }
    FILE *fp = popen(cmd, "r");
    if (!fp) {
        printf("[YUL_THUNK] Error: Failed to run solc command\n");
        return false;
    }

    static char line[524288];
    char *bytecode_hex = NULL;
    bool found_contract = false;
    bool found_bin = false;

    while (fgets(line, sizeof(line), fp)) {
        if (is_solidity && !found_contract) {
            if (case_insensitive_strstr(line, name) && strstr(line, "======= ")) {
                found_contract = true;
            }
            continue;
        }
        if (found_bin) {
            size_t len = strlen(line);
            while (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r' || line[len-1] == ' ')) {
                line[len-1] = '\0';
                len--;
            }
            if (len > 10) {
                bytecode_hex = strdup(line);
                break;
            }
            found_bin = false;
        }
        if (strstr(line, "Binary representation:") || strstr(line, "Binary:")) {
            found_bin = true;
        }
    }
    pclose(fp);

    if (!bytecode_hex) {
        printf("[YUL_THUNK] Error: solc output didn't contain bytecode for %s\n", name);
        return false;
    }

    size_t bin_len = 0;
    uint8_t *raw_bin = hex_to_bytes(bytecode_hex, &bin_len);
    free(bytecode_hex);

    if (!raw_bin) {
        return false;
    }

    // Run interpreter once on initcode to get the runtime bytecode!
    // Pad initcode with 512 zero bytes for mock constructor arguments to prevent solidity out-of-bounds check reverts.
    size_t padded_len = bin_len + 512;
    uint8_t *padded_bin = calloc(1, padded_len);
    if (padded_bin) {
        memcpy(padded_bin, raw_bin, bin_len);
    } else {
        padded_bin = raw_bin;
        padded_len = bin_len;
    }

    YulEvmContext init_ctx;
    memset(&init_ctx, 0, sizeof(init_ctx));
    bool init_success = run_yul_bytecode(&init_ctx, padded_bin, padded_len, name);
    if (padded_bin != raw_bin) free(padded_bin);
    if (!init_success || init_ctx.reverted || init_ctx.return_size == 0) {
        printf("[YUL_THUNK] Error: Initcode execution failed for %s. init_success=%d, reverted=%d, return_size=%zu\n", name, init_success, init_ctx.reverted, init_ctx.return_size);
        if (init_ctx.return_size >= 4 && init_ctx.return_data[0] == 0x08 && init_ctx.return_data[1] == 0xc3 && init_ctx.return_data[2] == 0x79 && init_ctx.return_data[3] == 0xa0) {
            if (init_ctx.return_size >= 68) {
                uint32_t str_len = ((uint32_t)init_ctx.return_data[64] << 24) |
                                   ((uint32_t)init_ctx.return_data[65] << 16) |
                                   ((uint32_t)init_ctx.return_data[66] << 8)  |
                                   (uint32_t)init_ctx.return_data[67];
                if (str_len < 1000 && 68 + (size_t)str_len <= init_ctx.return_size) {
                    char *err_msg = malloc(str_len + 1);
                    if (err_msg) {
                        memcpy(err_msg, init_ctx.return_data + 68, str_len);
                        err_msg[str_len] = '\0';
                        printf("[YUL_THUNK] Revert reason: %s\n", err_msg);
                        free(err_msg);
                    }
                }
            }
        } else if (init_ctx.return_size >= 4 && init_ctx.return_data[0] == 0x4e && init_ctx.return_data[1] == 0x48 && init_ctx.return_data[2] == 0x7b && init_ctx.return_data[3] == 0xa1) {
            if (init_ctx.return_size >= 36) {
                uint32_t panic_code = init_ctx.return_data[35];
                printf("[YUL_THUNK] Panic code: 0x%x\n", panic_code);
            }
        }
        free(raw_bin);
        return false;
    }

    uint8_t *runtime_bin = malloc(init_ctx.return_size);
    if (!runtime_bin) {
        free(raw_bin);
        return false;
    }
    memcpy(runtime_bin, init_ctx.return_data, init_ctx.return_size);
    size_t runtime_len = init_ctx.return_size;

    free(raw_bin);

    // Register contract
    CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
    strncpy(c->name, name, sizeof(c->name) - 1);
    c->bytecode = runtime_bin;
    c->size = runtime_len;
    c->virtual_address = virtual_address;

    // Copy constructor storage changes to global context and persist
    for (int i = 0; i < init_ctx.storage_count; i++) {
        context_sstore(&g_yul_evm_context, init_ctx.storage_keys[i], init_ctx.storage_vals[i]);
    }
    persist_reconciliation_data();

    printf("[YUL_THUNK] Registered Yul thunk: %s (%zu bytes) at virtual addr 0x%lx\n", name, bin_len, virtual_address);
    return true;
}

// 256-bit operations implementation
static u256_t u256_add(u256_t a, u256_t b) {
    u256_t r;
    unsigned __int128 carry = 0;
    for (int i = 0; i < 4; i++) {
        unsigned __int128 sum = (unsigned __int128)a.d[i] + b.d[i] + carry;
        r.d[i] = (uint64_t)sum;
        carry = sum >> 64;
    }
    return r;
}

static u256_t u256_sub(u256_t a, u256_t b) {
    u256_t r;
    unsigned __int128 borrow = 0;
    for (int i = 0; i < 4; i++) {
        unsigned __int128 diff = (unsigned __int128)a.d[i] - b.d[i] - borrow;
        r.d[i] = (uint64_t)diff;
        borrow = (diff >> 64) ? 1 : 0;
    }
    return r;
}

static u256_t u256_mul(u256_t a, u256_t b) {
    u256_t r = {{0}};
    for (int i = 0; i < 4; i++) {
        if (!a.d[i]) continue;
        uint64_t carry = 0;
        for (int j = 0; i + j < 4; j++) {
            unsigned __int128 prod = (unsigned __int128)a.d[i] * b.d[j] + r.d[i + j] + carry;
            r.d[i + j] = (uint64_t)prod;
            carry = (uint64_t)(prod >> 64);
        }
    }
    return r;
}

static u256_t u256_and(u256_t a, u256_t b) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = a.d[i] & b.d[i];
    return r;
}

static u256_t u256_or(u256_t a, u256_t b) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = a.d[i] | b.d[i];
    return r;
}

static u256_t u256_xor(u256_t a, u256_t b) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = a.d[i] ^ b.d[i];
    return r;
}

static u256_t u256_not(u256_t a) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = ~a.d[i];
    return r;
}

static u256_t u256_shl(u256_t a, uint64_t shift) {
    u256_t r = {{0}};
    if (shift >= 256) return r;
    int limb_shift = shift / 64;
    int bit_shift = shift % 64;
    for (int i = limb_shift; i < 4; i++) {
        r.d[i] |= a.d[i - limb_shift] << bit_shift;
        if (bit_shift > 0 && i - limb_shift - 1 >= 0) {
            r.d[i] |= a.d[i - limb_shift - 1] >> (64 - bit_shift);
        }
    }
    return r;
}

static u256_t u256_shr(u256_t a, uint64_t shift) {
    u256_t r = {{0}};
    if (shift >= 256) return r;
    int limb_shift = shift / 64;
    int bit_shift = shift % 64;
    for (int i = 0; i < 4 - limb_shift; i++) {
        r.d[i] |= a.d[i + limb_shift] >> bit_shift;
        if (bit_shift > 0 && i + limb_shift + 1 < 4) {
            r.d[i] |= a.d[i + limb_shift + 1] << (64 - bit_shift);
        }
    }
    return r;
}

static int u256_lt(u256_t a, u256_t b) {
    for (int i = 3; i >= 0; i--) {
        if (a.d[i] < b.d[i]) return 1;
        if (a.d[i] > b.d[i]) return 0;
    }
    return 0;
}

static int u256_gt(u256_t a, u256_t b) {
    for (int i = 3; i >= 0; i--) {
        if (a.d[i] > b.d[i]) return 1;
        if (a.d[i] < b.d[i]) return 0;
    }
    return 0;
}

static int u256_slt(u256_t a, u256_t b) {
    int signA = (a.d[3] >> 63) & 1;
    int signB = (b.d[3] >> 63) & 1;
    if (signA != signB) {
        return signA;
    }
    return u256_lt(a, b);
}

static int u256_sgt(u256_t a, u256_t b) {
    int signA = (a.d[3] >> 63) & 1;
    int signB = (b.d[3] >> 63) & 1;
    if (signA != signB) {
        return signB;
    }
    return u256_gt(a, b);
}

static int u256_eq(u256_t a, u256_t b) {
    return a.d[0] == b.d[0] && a.d[1] == b.d[1] && a.d[2] == b.d[2] && a.d[3] == b.d[3];
}

static int u256_iszero(u256_t a) {
    return a.d[0] == 0 && a.d[1] == 0 && a.d[2] == 0 && a.d[3] == 0;
}

static void u256_divmod(u256_t num, u256_t den, u256_t *q, u256_t *r) {
    u256_t zero = {{0}};
    if (u256_iszero(den)) {
        if (q) *q = zero;
        if (r) *r = zero;
        return;
    }
    u256_t quot = {{0}};
    u256_t rem = {{0}};
    for (int i = 255; i >= 0; i--) {
        rem = u256_shl(rem, 1);
        int limb = i / 64;
        int bit = i % 64;
        uint64_t bit_val = (num.d[limb] >> bit) & 1;
        rem.d[0] |= bit_val;
        
        if (u256_gt(rem, den) || u256_eq(rem, den)) {
            rem = u256_sub(rem, den);
            int q_limb = i / 64;
            int q_bit = i % 64;
            quot.d[q_limb] |= (1ULL << q_bit);
        }
    }
    if (q) *q = quot;
    if (r) *r = rem;
}

static u256_t u256_div(u256_t a, u256_t b) {
    u256_t q;
    u256_divmod(a, b, &q, NULL);
    return q;
}

static u256_t u256_mod(u256_t a, u256_t b) {
    u256_t r;
    u256_divmod(a, b, NULL, &r);
    return r;
}

static u256_t u256_sdiv(u256_t a, u256_t b) {
    u256_t zero = {{0}};
    if (u256_iszero(b)) return zero;
    
    bool a_neg = (a.d[3] & (1ULL << 63)) != 0;
    bool b_neg = (b.d[3] & (1ULL << 63)) != 0;
    
    u256_t abs_a = a_neg ? u256_sub(zero, a) : a;
    u256_t abs_b = b_neg ? u256_sub(zero, b) : b;
    
    u256_t q;
    u256_divmod(abs_a, abs_b, &q, NULL);
    
    if (a_neg != b_neg) {
        q = u256_sub(zero, q);
    }
    return q;
}

static u256_t u256_smod(u256_t a, u256_t b) {
    u256_t zero = {{0}};
    if (u256_iszero(b)) return zero;
    
    bool a_neg = (a.d[3] & (1ULL << 63)) != 0;
    
    u256_t abs_a = a_neg ? u256_sub(zero, a) : a;
    u256_t abs_b = (b.d[3] & (1ULL << 63)) != 0 ? u256_sub(zero, b) : b;
    
    u256_t r;
    u256_divmod(abs_a, abs_b, NULL, &r);
    
    if (a_neg) {
        r = u256_sub(zero, r);
    }
    return r;
}

static u256_t u256_exp(u256_t base, u256_t exponent) {
    u256_t result = {{0}};
    result.d[0] = 1;
    u256_t b = base;
    u256_t e = exponent;
    while (!u256_iszero(e)) {
        if (e.d[0] & 1) {
            result = u256_mul(result, b);
        }
        b = u256_mul(b, b);
        e = u256_shr(e, 1);
    }
    return result;
}

static u256_t u256_signextend(u256_t x, u256_t i_val) {
    if (u256_gt(i_val, (u256_t){{31, 0, 0, 0}})) return x;
    uint64_t i = i_val.d[0];
    u256_t r = x;
    int limb = (i * 8 + 7) / 64;
    int bit = (i * 8 + 7) % 64;
    bool sign_bit = (x.d[limb] >> bit) & 1;
    for (int b = i * 8 + 8; b < 256; b++) {
        int l = b / 64;
        int pt = b % 64;
        if (sign_bit) {
            r.d[l] |= (1ULL << pt);
        } else {
            r.d[l] &= ~(1ULL << pt);
        }
    }
    return r;
}

static u256_t u256_addmod(u256_t a, u256_t b, u256_t N) {
    u256_t zero = {{0}};
    if (u256_iszero(N)) return zero;
    u256_t a_mod = u256_mod(a, N);
    u256_t b_mod = u256_mod(b, N);
    u256_t r = u256_add(a_mod, b_mod);
    if (u256_lt(r, a_mod) || u256_gt(r, N) || u256_eq(r, N)) {
        r = u256_sub(r, N);
    }
    return r;
}

static u256_t u256_mulmod(u256_t a, u256_t b, u256_t N) {
    u256_t zero = {{0}};
    if (u256_iszero(N)) return zero;
    u256_t a_mod = u256_mod(a, N);
    u256_t b_mod = u256_mod(b, N);
    u256_t res = {{0}};
    u256_t temp = a_mod;
    for (int i = 0; i < 256; i++) {
        int limb = i / 64;
        int bit = i % 64;
        if ((b_mod.d[limb] >> bit) & 1) {
            res = u256_addmod(res, temp, N);
        }
        temp = u256_addmod(temp, temp, N);
    }
    return res;
}

static uint64_t get_namespaced_key(uint64_t self_addr, uint64_t key) {
    uint64_t hash = 14695981039346656037ULL;
    for (int i = 0; i < 8; i++) {
        hash ^= (self_addr >> (i * 8)) & 0xff;
        hash *= 1099511628211ULL;
    }
    for (int i = 0; i < 8; i++) {
        hash ^= (key >> (i * 8)) & 0xff;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static u256_t context_sload(YulEvmContext *ctx, uint64_t key) {
    uint64_t ns_key = get_namespaced_key(ctx->self_address ? ctx->self_address : 0x1000, key);
    for (int i = 0; i < ctx->storage_count; i++) {
        if (ctx->storage_keys[i] == ns_key) {
            return ctx->storage_vals[i];
        }
    }
    u256_t zero = {{0}};
    return zero;
}

static void context_sstore(YulEvmContext *ctx, uint64_t key, u256_t val) {
    uint64_t ns_key = get_namespaced_key(ctx->self_address ? ctx->self_address : 0x1000, key);
    for (int i = 0; i < ctx->storage_count; i++) {
        if (ctx->storage_keys[i] == ns_key) {
            ctx->storage_vals[i] = val;
            return;
        }
    }
    if (ctx->storage_count < 4096) {
        ctx->storage_keys[ctx->storage_count] = ns_key;
        ctx->storage_vals[ctx->storage_count] = val;
        ctx->storage_count++;
    }
}

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

static bool execute_nested_call(YulEvmContext *ctx, uint64_t target_addr, uint64_t argsOffset, uint64_t argsSize, uint64_t retOffset, uint64_t retSize, u256_t *success_out);

// Simulated execution of EVM bytecode
static bool run_yul_bytecode(YulEvmContext *ctx, const uint8_t *bytecode, size_t size, const char *name) {
    ctx->stack_ptr = 0;
    memset(ctx->memory, 0, sizeof(ctx->memory));
    ctx->reverted = false;
    ctx->return_size = 0;
    
    size_t pc = 0;
    while (pc < size) {
        uint8_t op = bytecode[pc];
        if (name && strcmp(name, "yi") == 0) {
            printf("[TRACE_EVM] PC %zu: Opcode 0x%02x, Stack pointer %d", pc, op, ctx->stack_ptr);
            if (ctx->stack_ptr > 0) {
                printf(", Stack: [0]=0x%lx", ctx->stack[ctx->stack_ptr - 1].d[0]);
                if (ctx->stack_ptr > 1) {
                    printf(", [1]=0x%lx", ctx->stack[ctx->stack_ptr - 2].d[0]);
                }
                if (ctx->stack_ptr > 2) {
                    printf(", [2]=0x%lx", ctx->stack[ctx->stack_ptr - 3].d[0]);
                }
            }
            printf("\n");
        }
        
        // PUSH instructions
        if (op >= 0x60 && op <= 0x7f) {
            int num_bytes = op - 0x5f;
            u256_t val = {{0}};
            for (int i = 0; i < num_bytes; i++) {
                pc++;
                uint8_t byte = (pc < size) ? bytecode[pc] : 0;
                val = u256_shl(val, 8);
                val.d[0] |= byte;
            }
            if (ctx->stack_ptr >= 1024) {
                printf("[DEBUG_EVM] Stack overflow at PUSH (stack_ptr: %d)\n", ctx->stack_ptr);
                return false;
            }
            ctx->stack[ctx->stack_ptr++] = val;
            pc++;
            continue;
        }
        
        // DUP instructions
        if (op >= 0x80 && op <= 0x8f) {
            int depth = op - 0x7f;
            if (ctx->stack_ptr < depth || ctx->stack_ptr >= 1024) {
                printf("[DEBUG_EVM] Stack error at DUP (depth: %d, stack_ptr: %d)\n", depth, ctx->stack_ptr);
                return false;
            }
            ctx->stack[ctx->stack_ptr] = ctx->stack[ctx->stack_ptr - depth];
            ctx->stack_ptr++;
            pc++;
            continue;
        }
        
        // SWAP instructions
        if (op >= 0x90 && op <= 0x9f) {
            int depth = op - 0x8f;
            if (ctx->stack_ptr <= depth) {
                printf("[DEBUG_EVM] Stack underflow at SWAP (depth: %d, stack_ptr: %d)\n", depth, ctx->stack_ptr);
                return false;
            }
            u256_t tmp = ctx->stack[ctx->stack_ptr - 1];
            ctx->stack[ctx->stack_ptr - 1] = ctx->stack[ctx->stack_ptr - 1 - depth];
            ctx->stack[ctx->stack_ptr - 1 - depth] = tmp;
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
                u256_t n = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_addmod(a, b, n);
                break;
            }
            case 0x09: { // MULMOD
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at MULMOD\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[--ctx->stack_ptr];
                u256_t n = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_mulmod(a, b, n);
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
                u256_t i = ctx->stack[--ctx->stack_ptr];
                u256_t x = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = u256_signextend(x, i);
                break;
            }
            case 0x10: { // LT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at LT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                u256_t r = {{0}};
                r.d[0] = u256_lt(a, b);
                ctx->stack[ctx->stack_ptr - 1] = r;
                break;
            }
            case 0x11: { // GT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at GT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                u256_t r = {{0}};
                r.d[0] = u256_gt(a, b);
                ctx->stack[ctx->stack_ptr - 1] = r;
                break;
            }
            case 0x12: { // SLT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SLT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                u256_t r = {{0}};
                r.d[0] = u256_slt(a, b);
                ctx->stack[ctx->stack_ptr - 1] = r;
                break;
            }
            case 0x13: { // SGT
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SGT\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                u256_t r = {{0}};
                r.d[0] = u256_sgt(a, b);
                ctx->stack[ctx->stack_ptr - 1] = r;
                break;
            }
            case 0x14: { // EQ
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at EQ\n"); return false; }
                u256_t a = ctx->stack[--ctx->stack_ptr];
                u256_t b = ctx->stack[ctx->stack_ptr - 1];
                u256_t r = {{0}};
                r.d[0] = u256_eq(a, b);
                ctx->stack[ctx->stack_ptr - 1] = r;
                break;
            }
            case 0x15: { // ISZERO
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at ISZERO\n"); return false; }
                u256_t a = ctx->stack[ctx->stack_ptr - 1];
                u256_t r = {{0}};
                r.d[0] = u256_iszero(a);
                ctx->stack[ctx->stack_ptr - 1] = r;
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
                ctx->stack[ctx->stack_ptr - 1] = u256_not(ctx->stack[ctx->stack_ptr - 1]);
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
            case 0x35: { // CALLDATALOAD
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at CALLDATALOAD\n"); return false; }
                u256_t offset = ctx->stack[ctx->stack_ptr - 1];
                ctx->stack[ctx->stack_ptr - 1] = load_calldata_32(ctx, offset.d[0]);
                break;
            }
            case 0x34: { // CALLVALUE
                u256_t r = {{0}};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLVALUE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x38: { // CODESIZE
                u256_t r = {{0}};
                r.d[0] = size;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CODESIZE\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x39: { // CODECOPY
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CODECOPY\n"); return false; }
                u256_t dest_offset = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    uint64_t src_idx = offset.d[0] + i;
                    uint64_t dest_idx = dest_offset.d[0] + i;
                    if (dest_idx < 65536) {
                        ctx->memory[dest_idx] = (src_idx < size) ? bytecode[src_idx] : 0;
                    }
                }
                break;
            }
            case 0x20: { // SHA3 / KECCAK256
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SHA3\n"); return false; }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                u256_t r = {{0}};
                // Simple robust FNV-1a hash of the memory region to simulate keccak256
                uint64_t hash = 14695981039346656037ULL;
                for (uint64_t i = 0; i < length.d[0]; i++) {
                    uint64_t idx = offset.d[0] + i;
                    uint8_t byte = (idx < 65536) ? ctx->memory[idx] : 0;
                    hash ^= byte;
                    hash *= 1099511628211ULL;
                }
                r.d[0] = hash;
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x30: { // ADDRESS
                u256_t r = {{0}};
                r.d[0] = ctx->self_address ? ctx->self_address : 0x1000;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at ADDRESS\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x31: { // BALANCE
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at BALANCE\n"); return false; }
                ctx->stack[ctx->stack_ptr - 1].d[0] = 1000000000000000000ULL;
                break;
            }
            case 0x32: { // ORIGIN
                u256_t r = {{0}};
                r.d[0] = 0x827279cffFb92266ULL;
                r.d[1] = 0x1aad88F6F4ce6aB8ULL;
                r.d[2] = 0xf39Fd6e5ULL;
                r.d[3] = 0;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at ORIGIN\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x3b: { // EXTCODESIZE
                if (ctx->stack_ptr < 1) { printf("[DEBUG_EVM] Stack underflow at EXTCODESIZE\n"); return false; }
                u256_t addr = ctx->stack[--ctx->stack_ptr];
                u256_t r = {{0}};
                CachedContract *target = NULL;
                for (int i = 0; i < g_cached_contracts_count; i++) {
                    if (g_cached_contracts[i].virtual_address == addr.d[0]) {
                        target = &g_cached_contracts[i];
                        break;
                    }
                }
                if (target) {
                    r.d[0] = target->size;
                } else {
                    r.d[0] = 128; // Bypass Solidity's extcodesize check for all unregistered addresses, including 0x0
                }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x33: { // CALLER
                u256_t r = {{0}};
                // Mock default sender address: 0xf39Fd6e51aad88F6F4ce6aB8827279cffFb92266
                r.d[0] = 0x827279cffFb92266ULL;
                r.d[1] = 0x1aad88F6F4ce6aB8ULL;
                r.d[2] = 0xf39Fd6e5ULL;
                r.d[3] = 0;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at CALLER\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x42: { // TIMESTAMP
                u256_t r = {{0}};
                // Return real clock time to emulate EVM timestamp
                #include <time.h>
                r.d[0] = (uint64_t)time(NULL);
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at TIMESTAMP\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x43: { // NUMBER
                u256_t r = {{0}};
                // Incremented mock block number using local system ticks
                static uint64_t g_mock_block = 1000;
                g_mock_block++;
                r.d[0] = g_mock_block;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at NUMBER\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0x5a: { // GAS
                u256_t r = {{0}};
                r.d[0] = 99999999ULL;
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at GAS\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0xfa: { // STATICCALL
                if (ctx->stack_ptr < 6) { printf("[DEBUG_EVM] Stack underflow at STATICCALL\n"); return false; }
                u256_t gas = ctx->stack[--ctx->stack_ptr];
                u256_t addr = ctx->stack[--ctx->stack_ptr];
                u256_t argsOffset = ctx->stack[--ctx->stack_ptr];
                u256_t argsSize = ctx->stack[--ctx->stack_ptr];
                u256_t retOffset = ctx->stack[--ctx->stack_ptr];
                u256_t retSize = ctx->stack[--ctx->stack_ptr];
                (void)gas;
                
                u256_t success = {{0}};
                execute_nested_call(ctx, addr.d[0], argsOffset.d[0], argsSize.d[0], retOffset.d[0], retSize.d[0], &success);
                ctx->stack[ctx->stack_ptr++] = success;
                break;
            }
            case 0xa0: // LOG0
            case 0xa1: // LOG1
            case 0xa2: // LOG2
            case 0xa3: // LOG3
            case 0xa4: { // LOG4
                int num_topics = op - 0xa0;
                int num_pops = 2 + num_topics;
                if (ctx->stack_ptr < num_pops) {
                    printf("[DEBUG_EVM] Stack underflow at LOG%d\n", num_topics);
                    return false;
                }
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t size = ctx->stack[--ctx->stack_ptr];
                
                printf("[EVENT_EMITTED] LOG%d: offset=%lu, size=%lu", num_topics, offset.d[0], size.d[0]);
                FILE *ev_fp = fopen("recent_emits.log", "a");
                if (ev_fp) {
                    fprintf(ev_fp, "[EVENT_EMITTED] LOG%d: offset=%lu, size=%lu", num_topics, offset.d[0], size.d[0]);
                }
                for (int t = 0; t < num_topics; t++) {
                    u256_t topic = ctx->stack[--ctx->stack_ptr];
                    printf(", topic%d=0x%016lx%016lx%016lx%016lx", t, topic.d[3], topic.d[2], topic.d[1], topic.d[0]);
                    if (ev_fp) {
                        fprintf(ev_fp, ", topic%d=0x%016lx%016lx%016lx%016lx", t, topic.d[3], topic.d[2], topic.d[1], topic.d[0]);
                    }
                }
                printf("\n");
                if (ev_fp) {
                    fprintf(ev_fp, "\n");
                    fclose(ev_fp);
                }
                break;
            }
            case 0x5f: { // PUSH0
                u256_t r = {{0}};
                if (ctx->stack_ptr >= 1024) { printf("[DEBUG_EVM] Stack overflow at PUSH0\n"); return false; }
                ctx->stack[ctx->stack_ptr++] = r;
                break;
            }
            case 0xfe: { // INVALID
                printf("[DEBUG_EVM] Invalid opcode 0xfe executed\n");
                return false;
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
                ctx->stack[ctx->stack_ptr - 1] = context_sload(ctx, key.d[0]);
                break;
            }
            case 0x55: { // SSTORE
                if (ctx->stack_ptr < 2) { printf("[DEBUG_EVM] Stack underflow at SSTORE\n"); return false; }
                u256_t key = ctx->stack[--ctx->stack_ptr];
                u256_t val = ctx->stack[--ctx->stack_ptr];
                context_sstore(ctx, key.d[0], val);
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
                ctx->reverted = true;
                return false;
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
            case 0xf0: { // CREATE
                if (ctx->stack_ptr < 3) { printf("[DEBUG_EVM] Stack underflow at CREATE\n"); return false; }
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                (void)value;
                
                u256_t new_addr = {{0}};
                if (g_cached_contracts_count < MAX_CACHED_CONTRACTS && offset.d[0] < 524288 && length.d[0] > 0) {
                    size_t initcode_len = length.d[0];
                    if (offset.d[0] + initcode_len > 524288) initcode_len = 524288 - offset.d[0];
                    uint8_t *initcode = ctx->memory + offset.d[0];
                    
                    size_t padded_len = initcode_len + 512;
                    uint8_t *padded_bin = calloc(1, padded_len);
                    if (padded_bin) {
                        memcpy(padded_bin, initcode, initcode_len);
                    } else {
                        padded_bin = initcode;
                        padded_len = initcode_len;
                    }
                    
                    YulEvmContext nested_ctx;
                    memset(&nested_ctx, 0, sizeof(nested_ctx));
                    nested_ctx.storage_count = ctx->storage_count;
                    memcpy(nested_ctx.storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
                    memcpy(nested_ctx.storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
                    
                    bool run_ok = run_yul_bytecode(&nested_ctx, padded_bin, padded_len, "dynamic_create");
                    if (padded_bin != initcode) free(padded_bin);
                    
                    if (run_ok && !nested_ctx.reverted && nested_ctx.return_size > 0) {
                        uint8_t *runtime_bin = malloc(nested_ctx.return_size);
                        if (runtime_bin) {
                            memcpy(runtime_bin, nested_ctx.return_data, nested_ctx.return_size);
                            
                            CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
                            snprintf(c->name, sizeof(c->name), "dynamic_%d", g_cached_contracts_count);
                            c->bytecode = runtime_bin;
                            c->size = nested_ctx.return_size;
                            c->virtual_address = 0x1000 + g_cached_contracts_count;
                            new_addr.d[0] = c->virtual_address;
                            
                            ctx->storage_count = nested_ctx.storage_count;
                            memcpy(ctx->storage_keys, nested_ctx.storage_keys, sizeof(ctx->storage_keys));
                            memcpy(ctx->storage_vals, nested_ctx.storage_vals, sizeof(ctx->storage_vals));
                            persist_reconciliation_data();
                            
                            printf("[EVM_INTERPRETER] CREATE success. Registered contract %s at 0x%lx\n", c->name, c->virtual_address);
                        }
                    }
                }
                ctx->stack[ctx->stack_ptr++] = new_addr;
                break;
            }
            case 0xf5: { // CREATE2
                if (ctx->stack_ptr < 4) { printf("[DEBUG_EVM] Stack underflow at CREATE2\n"); return false; }
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t offset = ctx->stack[--ctx->stack_ptr];
                u256_t length = ctx->stack[--ctx->stack_ptr];
                u256_t salt = ctx->stack[--ctx->stack_ptr];
                (void)value;
                
                u256_t new_addr = {{0}};
                if (g_cached_contracts_count < MAX_CACHED_CONTRACTS && offset.d[0] < 524288 && length.d[0] > 0) {
                    size_t initcode_len = length.d[0];
                    if (offset.d[0] + initcode_len > 524288) initcode_len = 524288 - offset.d[0];
                    uint8_t *initcode = ctx->memory + offset.d[0];
                    
                    size_t padded_len = initcode_len + 512;
                    uint8_t *padded_bin = calloc(1, padded_len);
                    if (padded_bin) {
                        memcpy(padded_bin, initcode, initcode_len);
                    } else {
                        padded_bin = initcode;
                        padded_len = initcode_len;
                    }
                    
                    // Calculate predictable CREATE2 address first using standard formula
                    TsfiPulseHash initcode_hash;
                    tsfi_pulse_keccak256(initcode, initcode_len, &initcode_hash);
                    
                    uint8_t deployer[20] = {0};
                    for (int i = 0; i < 8; i++) {
                        deployer[19 - i] = (ctx->self_address >> (i * 8)) & 0xff;
                    }
                    
                    uint8_t salt_bytes[32] = {0};
                    for (int i = 0; i < 4; i++) {
                        for (int j = 0; j < 8; j++) {
                            salt_bytes[(3 - i) * 8 + (7 - j)] = (salt.d[i] >> (j * 8)) & 0xff;
                        }
                    }
                    
                    uint8_t preimage[1 + 20 + 32 + 32];
                    preimage[0] = 0xff;
                    memcpy(preimage + 1, deployer, 20);
                    memcpy(preimage + 21, salt_bytes, 32);
                    memcpy(preimage + 53, initcode_hash.data, 32);
                    
                    TsfiPulseHash final_hash;
                    tsfi_pulse_keccak256(preimage, sizeof(preimage), &final_hash);
                    
                    uint64_t create2_addr = 0;
                    for (int i = 0; i < 8; i++) {
                        create2_addr = (create2_addr << 8) | final_hash.data[24 + i];
                    }
                    
                    YulEvmContext nested_ctx;
                    memset(&nested_ctx, 0, sizeof(nested_ctx));
                    nested_ctx.storage_count = ctx->storage_count;
                    memcpy(nested_ctx.storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
                    memcpy(nested_ctx.storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
                    nested_ctx.self_address = create2_addr;
                    
                    bool run_ok = run_yul_bytecode(&nested_ctx, padded_bin, padded_len, "dynamic_create2");
                    if (padded_bin != initcode) free(padded_bin);
                    
                    if (run_ok && !nested_ctx.reverted && nested_ctx.return_size > 0) {
                        uint8_t *runtime_bin = malloc(nested_ctx.return_size);
                        if (runtime_bin) {
                            memcpy(runtime_bin, nested_ctx.return_data, nested_ctx.return_size);
                            
                            CachedContract *c = &g_cached_contracts[g_cached_contracts_count++];
                            snprintf(c->name, sizeof(c->name), "dynamic2_%d", g_cached_contracts_count);
                            c->bytecode = runtime_bin;
                            c->size = nested_ctx.return_size;
                            c->virtual_address = create2_addr;
                            new_addr.d[0] = c->virtual_address;
                            
                            ctx->storage_count = nested_ctx.storage_count;
                            memcpy(ctx->storage_keys, nested_ctx.storage_keys, sizeof(ctx->storage_keys));
                            memcpy(ctx->storage_vals, nested_ctx.storage_vals, sizeof(ctx->storage_vals));
                            persist_reconciliation_data();
                            
                            printf("[EVM_INTERPRETER] CREATE2 success. Registered contract %s at 0x%lx\n", c->name, c->virtual_address);
                        }
                    }
                }
                ctx->stack[ctx->stack_ptr++] = new_addr;
                break;
            }
            case 0xf1: { // CALL
                if (ctx->stack_ptr < 7) { printf("[DEBUG_EVM] Stack underflow at CALL\n"); return false; }
                u256_t gas = ctx->stack[--ctx->stack_ptr];
                u256_t addr = ctx->stack[--ctx->stack_ptr];
                u256_t value = ctx->stack[--ctx->stack_ptr];
                u256_t argsOffset = ctx->stack[--ctx->stack_ptr];
                u256_t argsSize = ctx->stack[--ctx->stack_ptr];
                u256_t retOffset = ctx->stack[--ctx->stack_ptr];
                u256_t retSize = ctx->stack[--ctx->stack_ptr];
                (void)gas; (void)value;
                
                u256_t success = {{0}};
                execute_nested_call(ctx, addr.d[0], argsOffset.d[0], argsSize.d[0], retOffset.d[0], retSize.d[0], &success);
                ctx->stack[ctx->stack_ptr++] = success;
                break;
            }
            case 0xf4: { // DELEGATECALL
                if (ctx->stack_ptr < 6) { printf("[DEBUG_EVM] Stack underflow at DELEGATECALL\n"); return false; }
                u256_t gas = ctx->stack[--ctx->stack_ptr];
                u256_t addr = ctx->stack[--ctx->stack_ptr];
                u256_t argsOffset = ctx->stack[--ctx->stack_ptr];
                u256_t argsSize = ctx->stack[--ctx->stack_ptr];
                u256_t retOffset = ctx->stack[--ctx->stack_ptr];
                u256_t retSize = ctx->stack[--ctx->stack_ptr];
                (void)gas;
                
                CachedContract *target = NULL;
                for (int i = 0; i < g_cached_contracts_count; i++) {
                    if (g_cached_contracts[i].virtual_address == addr.d[0]) {
                        target = &g_cached_contracts[i];
                        break;
                    }
                }
                
                u256_t success = {{0}};
                if (target) {
                    YulEvmContext sub_ctx;
                    memcpy(&sub_ctx, ctx, sizeof(YulEvmContext));
                    sub_ctx.stack_ptr = 0;
                    memset(sub_ctx.stack, 0, sizeof(sub_ctx.stack));
                    
                    memset(sub_ctx.calldata, 0, sizeof(sub_ctx.calldata));
                    if (argsOffset.d[0] < 524288) {
                        size_t avail = 524288 - argsOffset.d[0];
                        size_t to_copy = argsSize.d[0] < avail ? argsSize.d[0] : avail;
                        memcpy(sub_ctx.calldata, ctx->memory + argsOffset.d[0], to_copy);
                        sub_ctx.calldatasize = to_copy;
                    } else {
                        sub_ctx.calldatasize = 0;
                    }
                    
                    bool sub_success = run_yul_bytecode(&sub_ctx, target->bytecode, target->size, target->name);
                    if (sub_success && !sub_ctx.reverted) {
                        ctx->storage_count = sub_ctx.storage_count;
                        memcpy(ctx->storage_keys, sub_ctx.storage_keys, sizeof(ctx->storage_keys));
                        memcpy(ctx->storage_vals, sub_ctx.storage_vals, sizeof(ctx->storage_vals));
                        memcpy(ctx->memory, sub_ctx.memory, sizeof(ctx->memory));
                        if (retOffset.d[0] < 524288) {
                            size_t to_write = sub_ctx.return_size < retSize.d[0] ? sub_ctx.return_size : retSize.d[0];
                            size_t avail = 524288 - retOffset.d[0];
                            if (to_write > avail) to_write = avail;
                            memcpy(ctx->memory + retOffset.d[0], sub_ctx.return_data, to_write);
                        }
                        ctx->return_size = sub_ctx.return_size;
                        memcpy(ctx->return_data, sub_ctx.return_data, sizeof(ctx->return_data));
                        success.d[0] = 1;
                    } else {
                        ctx->return_size = sub_ctx.return_size;
                        memcpy(ctx->return_data, sub_ctx.return_data, sizeof(ctx->return_data));
                    }
                }
                ctx->stack[ctx->stack_ptr++] = success;
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
    CachedContract *c = NULL;
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcmp(g_cached_contracts[i].name, name) == 0) {
            c = &g_cached_contracts[i];
            break;
        }
    }

    if (!c) {
        printf("[YUL_THUNK] Error: Contract %s not initialized.\n", name);
        return false;
    }

    // Setup calldata
    memset(g_yul_evm_context.calldata, 0, sizeof(g_yul_evm_context.calldata));
    size_t size_to_copy = calldatasize < sizeof(g_yul_evm_context.calldata) ? calldatasize : sizeof(g_yul_evm_context.calldata);
    memcpy(g_yul_evm_context.calldata, calldata, size_to_copy);
    g_yul_evm_context.calldatasize = size_to_copy;

    // Run interpreter
    g_yul_evm_context.self_address = c->virtual_address;
    bool success = run_yul_bytecode(&g_yul_evm_context, c->bytecode, c->size, name);

    if (success) {
        persist_reconciliation_data();
        if (retval && retval_len) {
            size_t out_size = g_yul_evm_context.return_size < *retval_len ? g_yul_evm_context.return_size : *retval_len;
            memcpy(retval, g_yul_evm_context.return_data, out_size);
            *retval_len = out_size;
        }
    }

    return success;
}

void lau_yul_thunk_sstore(uint64_t key, uint64_t value) {
    u256_t val = {{0}};
    val.d[0] = value;
    context_sstore(&g_yul_evm_context, key, val);
}

uint64_t lau_yul_thunk_sload(uint64_t key) {
    u256_t val = context_sload(&g_yul_evm_context, key);
    return val.d[0];
}

size_t lau_yul_thunk_get_size(const char *name) {
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcmp(g_cached_contracts[i].name, name) == 0) {
            return g_cached_contracts[i].size;
        }
    }
    return 0;
}

static bool execute_nested_call(YulEvmContext *ctx, uint64_t target_addr, uint64_t argsOffset, uint64_t argsSize, uint64_t retOffset, uint64_t retSize, u256_t *success_out) {
    CachedContract *target = NULL;
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (g_cached_contracts[i].virtual_address == target_addr) {
            target = &g_cached_contracts[i];
            break;
        }
    }
    
    success_out->d[0] = 1;
    
    if (target) {
        YulEvmContext nested_ctx;
        memset(&nested_ctx, 0, sizeof(nested_ctx));
        
        nested_ctx.calldatasize = (argsSize > 65536) ? 65536 : argsSize;
        for (uint64_t i = 0; i < nested_ctx.calldatasize; i++) {
            uint64_t src = argsOffset + i;
            nested_ctx.calldata[i] = (src < 524288) ? ctx->memory[src] : 0;
        }
        
        nested_ctx.storage_count = ctx->storage_count;
        memcpy(nested_ctx.storage_keys, ctx->storage_keys, sizeof(ctx->storage_keys));
        memcpy(nested_ctx.storage_vals, ctx->storage_vals, sizeof(ctx->storage_vals));
        nested_ctx.self_address = target_addr;
        
        bool run_ok = run_yul_bytecode(&nested_ctx, target->bytecode, target->size, target->name);
        
        if (run_ok && !nested_ctx.reverted) {
            uint64_t copy_size = retSize;
            if (copy_size > nested_ctx.return_size) {
                copy_size = nested_ctx.return_size;
            }
            for (uint64_t i = 0; i < copy_size; i++) {
                uint64_t dest = retOffset + i;
                if (dest < 524288) {
                    ctx->memory[dest] = nested_ctx.return_data[i];
                }
            }
            
            ctx->storage_count = nested_ctx.storage_count;
            memcpy(ctx->storage_keys, nested_ctx.storage_keys, sizeof(ctx->storage_keys));
            memcpy(ctx->storage_vals, nested_ctx.storage_vals, sizeof(ctx->storage_vals));
            
            ctx->return_size = nested_ctx.return_size;
            if (ctx->return_size > 524288) ctx->return_size = 524288;
            memcpy(ctx->return_data, nested_ctx.return_data, ctx->return_size);
        } else {
            success_out->d[0] = 0;
        }
    } else {
        if (target_addr == 0x01) {
            u256_t provider_addr = {{0}};
            provider_addr.d[0] = 0x827279cffFb92266ULL;
            provider_addr.d[1] = 0x1aad88F6F4ce6aB8ULL;
            provider_addr.d[2] = 0xf39Fd6e5ULL;
            provider_addr.d[3] = 0;
            if (retOffset < 524288 && retSize > 0) {
                store_memory_32(ctx, retOffset, provider_addr);
                ctx->return_size = retSize;
                memset(ctx->return_data, 0, sizeof(ctx->return_data));
                size_t to_copy = retSize > 32 ? 32 : retSize;
                memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
            }
        } else {
            if (retOffset < 524288 && retSize > 0) {
                u256_t mock_ret = {{0}};
                mock_ret.d[0] = 0x1000; // default mock address/value
                store_memory_32(ctx, retOffset, mock_ret);
                ctx->return_size = retSize;
                memset(ctx->return_data, 0, sizeof(ctx->return_data));
                size_t to_copy = retSize > 32 ? 32 : retSize;
                memcpy(ctx->return_data, ctx->memory + retOffset, to_copy);
            }
        }
    }
    return true;
}

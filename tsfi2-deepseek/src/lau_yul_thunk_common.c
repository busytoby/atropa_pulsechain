#include "lau_yul_thunk_internal.h"
#include "tsfi_pulsechain.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

CachedContract g_cached_contracts[MAX_CACHED_CONTRACTS];
int g_cached_contracts_count = 0;
bool g_storage_dirty = false;

_Thread_local uint64_t g_transaction_diyat_tax_total = 0;

static u256_t g_owner_keys[10000];
static int g_owner_keys_count = 0;

static bool u256_eq_internal(u256_t a, u256_t b) {
    return a.d[0] == b.d[0] && a.d[1] == b.d[1] && a.d[2] == b.d[2] && a.d[3] == b.d[3];
}

void add_owner_key(u256_t ns_key) {
    for (int i = 0; i < g_owner_keys_count; i++) {
        if (u256_eq_internal(g_owner_keys[i], ns_key)) return;
    }
    if (g_owner_keys_count < 10000) {
        g_owner_keys[g_owner_keys_count++] = ns_key;
    }
}

bool is_owner_key(u256_t ns_key) {
    for (int i = 0; i < g_owner_keys_count; i++) {
        if (u256_eq_internal(g_owner_keys[i], ns_key)) return true;
    }
    return false;
}

// 256-bit operations implementation
u256_t u256_add(u256_t a, u256_t b) {
    u256_t r;
    unsigned __int128 carry = 0;
    for (int i = 0; i < 4; i++) {
        unsigned __int128 sum = (unsigned __int128)a.d[i] + b.d[i] + carry;
        r.d[i] = (uint64_t)sum;
        carry = sum >> 64;
    }
    return r;
}

u256_t u256_sub(u256_t a, u256_t b) {
    u256_t r;
    unsigned __int128 borrow = 0;
    for (int i = 0; i < 4; i++) {
        unsigned __int128 diff = (unsigned __int128)a.d[i] - b.d[i] - borrow;
        r.d[i] = (uint64_t)diff;
        borrow = (diff >> 64) ? 1 : 0;
    }
    return r;
}

u256_t u256_mul(u256_t a, u256_t b) {
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

u256_t u256_and(u256_t a, u256_t b) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = a.d[i] & b.d[i];
    return r;
}

u256_t u256_or(u256_t a, u256_t b) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = a.d[i] | b.d[i];
    return r;
}

u256_t u256_xor(u256_t a, u256_t b) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = a.d[i] ^ b.d[i];
    return r;
}

u256_t u256_not(u256_t a) {
    u256_t r;
    for (int i = 0; i < 4; i++) r.d[i] = ~a.d[i];
    return r;
}

u256_t u256_shl(u256_t a, uint64_t shift) {
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

u256_t u256_shr(u256_t a, uint64_t shift) {
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

int u256_lt(u256_t a, u256_t b) {
    for (int i = 3; i >= 0; i--) {
        if (a.d[i] < b.d[i]) return 1;
        if (a.d[i] > b.d[i]) return 0;
    }
    return 0;
}

int u256_gt(u256_t a, u256_t b) {
    for (int i = 3; i >= 0; i--) {
        if (a.d[i] > b.d[i]) return 1;
        if (a.d[i] < b.d[i]) return 0;
    }
    return 0;
}

int u256_slt(u256_t a, u256_t b) {
    int signA = (a.d[3] >> 63) & 1;
    int signB = (b.d[3] >> 63) & 1;
    if (signA != signB) {
        return signA;
    }
    return u256_lt(a, b);
}

int u256_sgt(u256_t a, u256_t b) {
    int signA = (a.d[3] >> 63) & 1;
    int signB = (b.d[3] >> 63) & 1;
    if (signA != signB) {
        return signB;
    }
    return u256_gt(a, b);
}

int u256_eq(u256_t a, u256_t b) {
    return u256_eq_internal(a, b);
}

int u256_iszero(u256_t a) {
    return a.d[0] == 0 && a.d[1] == 0 && a.d[2] == 0 && a.d[3] == 0;
}

void u256_divmod(u256_t num, u256_t den, u256_t *q, u256_t *r) {
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

u256_t u256_div(u256_t a, u256_t b) {
    u256_t q;
    u256_divmod(a, b, &q, NULL);
    return q;
}

u256_t u256_mod(u256_t a, u256_t b) {
    u256_t r;
    u256_divmod(a, b, NULL, &r);
    return r;
}

u256_t u256_sdiv(u256_t a, u256_t b) {
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

u256_t u256_smod(u256_t a, u256_t b) {
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

u256_t u256_exp(u256_t base, u256_t exponent) {
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

u256_t u256_signextend(u256_t x, u256_t i_val) {
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

u256_t u256_addmod(u256_t a, u256_t b, u256_t N) {
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

u256_t u256_mulmod(u256_t a, u256_t b, u256_t N) {
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

u256_t u256_mod_pow(u256_t base, u256_t exp, u256_t mod) {
    u256_t zero = {{0}};
    u256_t one = {{1, 0, 0, 0}};
    if (u256_iszero(mod)) return zero;
    
    u256_t res = one;
    base = u256_mod(base, mod);
    while (!u256_iszero(exp)) {
        if (exp.d[0] & 1) {
            res = u256_mod(u256_mul(res, base), mod);
        }
        base = u256_mod(u256_mul(base, base), mod);
        exp = u256_shr(exp, 1);
    }
    return res;
}

u256_t read_u256_be(const uint8_t *src) {
    u256_t r = {{0}};
    for (int i = 0; i < 4; i++) {
        uint64_t val = 0;
        for (int j = 0; j < 8; j++) {
            val = (val << 8) | src[(3 - i) * 8 + j];
        }
        r.d[i] = val;
    }
    return r;
}

void write_u256_be(uint8_t *dest, u256_t val) {
    for (int i = 0; i < 4; i++) {
        uint64_t w = val.d[i];
        for (int j = 0; j < 8; j++) {
            dest[(3 - i) * 8 + (7 - j)] = (uint8_t)(w >> (j * 8));
        }
    }
}

uint64_t parse_hex64(const char *hex) {
    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X')) {
        hex += 2;
    }
    size_t len = strlen(hex);
    if (len > 16) {
        hex += (len - 16);
    }
    return strtoull(hex, NULL, 16);
}

void persist_reconciliation_data(void) {
    FILE *fp = fopen("evm_storage.json", "w");
    if (!fp) {
        fp = fopen("tsfi2-deepseek/evm_storage.json", "w");
    }
    if (!fp) return;
    fprintf(fp, "{\n  \"storage\": [\n");
    for (int i = 0; i < g_yul_evm_context.storage_count; i++) {
        fprintf(fp, "    { \"key\": \"%016lx%016lx%016lx%016lx\", \"val\": \"%016lx%016lx%016lx%016lx\", \"addr\": \"%lx\" }%s\n",
                g_yul_evm_context.storage_keys[i].d[3],
                g_yul_evm_context.storage_keys[i].d[2],
                g_yul_evm_context.storage_keys[i].d[1],
                g_yul_evm_context.storage_keys[i].d[0],
                g_yul_evm_context.storage_vals[i].d[3],
                g_yul_evm_context.storage_vals[i].d[2],
                g_yul_evm_context.storage_vals[i].d[1],
                g_yul_evm_context.storage_vals[i].d[0],
                g_yul_evm_context.storage_addrs[i],
                (i == g_yul_evm_context.storage_count - 1) ? "" : ",");
    }
    fprintf(fp, "  ]\n}\n");
    fclose(fp);
}

uint64_t get_contract_address(const char *name) {
    for (int i = 0; i < g_cached_contracts_count; i++) {
        if (strcasecmp(g_cached_contracts[i].name, name) == 0) {
            return g_cached_contracts[i].virtual_address;
        }
    }
    // Fallback constants mapped to match standard initcode deployment
    if (strcasecmp(name, "HECKE") == 0) return parse_hex64("0x24F0154C1dCe548aDf15da2098fDd8B8a3b8151D");
    if (strcasecmp(name, "VMREQ") == 0) return parse_hex64("0x9766a8d4fCe69F8b5544b99C13D1dA85bBBE762D");
    if (strcasecmp(name, "SHAFactory") == 0) return parse_hex64("0xA865c31c6333CA7C1CBBF87F3D854D420797994F");
    if (strcasecmp(name, "SHIOFactory") == 0) return parse_hex64("0x737Df6B1d40A5f52fC5203a4E245A23725731769");
    if (strcasecmp(name, "yi") == 0) return parse_hex64("0xff2A5Cc6f6c07d71933fccD5a09b893CD44F4Db7");
    if (strcasecmp(name, "zheng") == 0) return parse_hex64("0x707389622103f0C5fC438FFd217f90663fFbdF12");
    if (strcasecmp(name, "zhou") == 0) return parse_hex64("0x8dD417c945DA2a6E282237bbB0D1A55c2b95b467");
    if (strcasecmp(name, "yau") == 0) return parse_hex64("0x40055f13377cfe5017e8F34167D0e404e050f06E");
    if (strcasecmp(name, "yang") == 0) return parse_hex64("0xe6B7baF9A4e09bb1D956Bb1b4af9Bd13539DC24f");
    if (strcasecmp(name, "siu") == 0) return parse_hex64("0xab55A1628d908e9Ae55fdf5dd2cb0dDC043920Fc");
    if (strcasecmp(name, "void") == 0) return parse_hex64("0xf65a7EF723B8a91755080bd8C1c70779dB77bBb5");
    if (strcasecmp(name, "laufactory") == 0) return parse_hex64("0x4D05Cc16B140347fb67c427456791F67df6CAb39");
    if (strcasecmp(name, "strings") == 0) return parse_hex64("0xf7a3B55128ebA848D4DdeAB28DfcebfE6D1FDA10");
    if (strcasecmp(name, "libattribute") == 0) return parse_hex64("0xa30a7a10ED34722453c30da1083407a790f1D874");
    if (strcasecmp(name, "corereactions") == 0) return parse_hex64("0xFEC485Ad87db95099110C47FCCc8eB691A6FCf7c");
    if (strcasecmp(name, "cho") == 0) return parse_hex64("0xB98E414af1fC02d6B5a6bF1CD08B844864B87d4f");
    if (strcasecmp(name, "map") == 0) return parse_hex64("0x60C7215A1C325Fc5fc91cd301DD94642142Baf80");
    if (strcasecmp(name, "qi") == 0) return parse_hex64("0x91F3ca10050862eccdbb7b265cc6fB49EeDD802C");
    if (strcasecmp(name, "mai") == 0) return parse_hex64("0x2cc7e492a98cb211287A1506A55245dBa200E52a");
    if (strcasecmp(name, "xia") == 0) return parse_hex64("0x6f7D105670842d39ACc32B5AeF0C5238AA0B9F3D");
    if (strcasecmp(name, "xie") == 0) return parse_hex64("0xfa242E3A4FBAa0773559560B49bDeA35C975cBaf");
    if (strcasecmp(name, "chan") == 0) return parse_hex64("0xCb39Cc1afd2CEe8E8C82F9A330aBF37F4933d850");
    if (strcasecmp(name, "sei") == 0) return parse_hex64("0x8f4798DfDA3BC507954d768Bc33f3c456A407F30");
    if (strcasecmp(name, "choa") == 0) return parse_hex64("0xd8949704cc5acAf1600D908a2607AbeB668fAc6b");
    if (strcasecmp(name, "cheon") == 0) return parse_hex64("0xcF54035Be7F6eC2664FEeaAcfbA4470BbB23E0A0");
    return 0;
}

uint64_t lau_yul_get_diyat_tax_total(void) {
    return g_transaction_diyat_tax_total;
}

int lau_yul_thunk_get_log_count(void) {
    return g_yul_evm_context.log_count;
}

bool lau_yul_thunk_get_log(int index, uint64_t *address, int *num_topics, u256_t *topics, uint8_t *data, size_t *data_size) {
    if (index < 0 || index >= g_yul_evm_context.log_count) {
        return false;
    }
    YulEventLog *log = &g_yul_evm_context.logs[index];
    if (address) *address = log->address;
    if (num_topics) *num_topics = log->num_topics;
    if (topics) {
        for (int i = 0; i < log->num_topics; i++) {
            topics[i] = log->topics[i];
        }
    }
    if (data && data_size) {
        size_t limit = *data_size < log->data_size ? *data_size : log->data_size;
        memcpy(data, log->data, limit);
        *data_size = limit;
    } else if (data_size) {
        *data_size = log->data_size;
    }
    return true;
}

static u256_t get_namespaced_key(uint64_t self_addr, u256_t key) {
    u256_t ns_key = key;
    ns_key.d[0] ^= self_addr;
    ns_key.d[1] ^= (self_addr >> 17) | (self_addr << 47);
    ns_key.d[2] ^= (self_addr >> 31) | (self_addr << 33);
    ns_key.d[3] ^= (self_addr >> 45) | (self_addr << 19);
    return ns_key;
}

bool g_initcode_running = false;

u256_t context_sload(YulEvmContext *ctx, u256_t key) {
    u256_t ns_key = get_namespaced_key(ctx->self_address ? ctx->self_address : 0x1000, key);
    for (int i = 0; i < ctx->storage_count; i++) {
        if (u256_eq_internal(ctx->storage_keys[i], ns_key)) {
            return ctx->storage_vals[i];
        }
    }
    if (is_owner_key(ns_key)) {
        u256_t one = {{1, 0, 0, 0}};
        return one;
    }
    u256_t zero = {{0}};
    return zero;
}

void context_sstore(YulEvmContext *ctx, u256_t key, u256_t val) {
    u256_t ns_key = get_namespaced_key(ctx->self_address ? ctx->self_address : 0x1000, key);
    // printf("[DEBUG_SSTORE] Contract 0x%lx writing key 0x%lx to val 0x%lx. storage_count before: %d\n", ctx->self_address, key.d[0], val.d[0], ctx->storage_count);
    for (int i = 0; i < ctx->storage_count; i++) {
        if (u256_eq_internal(ctx->storage_keys[i], ns_key)) {
            ctx->storage_addrs[i] = ctx->self_address;
            if (!u256_eq_internal(ctx->storage_vals[i], val)) {
                ctx->storage_vals[i] = val;
                g_storage_dirty = true;
            }
            return;
        }
    }
    if (ctx->storage_count < 32768) {
        ctx->storage_keys[ctx->storage_count] = ns_key;
        ctx->storage_vals[ctx->storage_count] = val;
        ctx->storage_addrs[ctx->storage_count] = ctx->self_address;
        ctx->storage_count++;
        g_storage_dirty = true;
    }
}

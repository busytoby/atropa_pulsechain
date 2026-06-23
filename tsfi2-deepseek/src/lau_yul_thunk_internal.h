#ifndef LAU_YUL_THUNK_INTERNAL_H
#define LAU_YUL_THUNK_INTERNAL_H

#include "lau_yul_thunk.h"
#include <pthread.h>

#define MAX_CACHED_CONTRACTS 1024

typedef struct {
    char name[64];
    uint8_t *bytecode;
    size_t size;
    uint64_t virtual_address;
    char path[256];
} CachedContract;

extern CachedContract g_cached_contracts[MAX_CACHED_CONTRACTS];
extern int g_cached_contracts_count;
extern bool g_storage_dirty;
extern bool g_initcode_running;

extern _Thread_local uint64_t g_transaction_diyat_tax_total;

// Internal math helpers
u256_t u256_add(u256_t a, u256_t b);
u256_t u256_sub(u256_t a, u256_t b);
u256_t u256_mul(u256_t a, u256_t b);
u256_t u256_and(u256_t a, u256_t b);
u256_t u256_or(u256_t a, u256_t b);
u256_t u256_xor(u256_t a, u256_t b);
u256_t u256_not(u256_t a);
u256_t u256_shl(u256_t a, uint64_t shift);
u256_t u256_shr(u256_t a, uint64_t shift);
int u256_lt(u256_t a, u256_t b);
int u256_gt(u256_t a, u256_t b);
int u256_slt(u256_t a, u256_t b);
int u256_sgt(u256_t a, u256_t b);
int u256_eq(u256_t a, u256_t b);
int u256_iszero(u256_t a);
void u256_divmod(u256_t num, u256_t den, u256_t *q, u256_t *r);
u256_t u256_div(u256_t a, u256_t b);
u256_t u256_mod(u256_t a, u256_t b);
u256_t u256_sdiv(u256_t a, u256_t b);
u256_t u256_smod(u256_t a, u256_t b);
u256_t u256_exp(u256_t base, u256_t exponent);
u256_t u256_signextend(u256_t x, u256_t i_val);
u256_t u256_addmod(u256_t a, u256_t b, u256_t N);
u256_t u256_mulmod(u256_t a, u256_t b, u256_t N);
u256_t u256_mod_pow(u256_t base, u256_t exp, u256_t mod);

// Serialization and utilities
void write_u256_be(uint8_t *dest, u256_t val);
u256_t read_u256_be(const uint8_t *src);
uint64_t parse_hex64(const char *hex);
uint64_t get_contract_address(const char *name);
void persist_reconciliation_data(void);

// DB storage functions
u256_t context_sload(YulEvmContext *ctx, u256_t key);
void context_sstore(YulEvmContext *ctx, u256_t key, u256_t val);

// Execution loops
bool run_yul_bytecode(YulEvmContext *ctx, const uint8_t *bytecode, size_t size, const char *name);

#endif // LAU_YUL_THUNK_INTERNAL_H

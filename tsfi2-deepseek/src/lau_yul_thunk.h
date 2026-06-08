#ifndef LAU_YUL_THUNK_H
#define LAU_YUL_THUNK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint64_t d[4]; // 256-bit
} u256_t;

typedef struct {
    u256_t stack[1024];
    int stack_ptr;
    uint8_t memory[65536];
    uint8_t calldata[4096];
    size_t calldatasize;
    uint8_t return_data[65536];
    size_t return_size;
    
    uint64_t storage_keys[4096];
    u256_t storage_vals[4096];
    int storage_count;
    bool reverted;
} YulEvmContext;

extern _Thread_local YulEvmContext g_yul_evm_context;

// Initializes and compiles Yul source file into bytecode cache
bool lau_yul_thunk_init(const char *name, const char *yul_path, uint64_t virtual_address);

// Executes compiled bytecode as a native thunk helper on the global context
bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t calldatasize, uint8_t *retval, size_t *retval_len);

// Helper to set storage directly
void lau_yul_thunk_sstore(uint64_t key, uint64_t value);

// Helper to get storage directly
uint64_t lau_yul_thunk_sload(uint64_t key);

// Helper to get runtime bytecode size
size_t lau_yul_thunk_get_size(const char *name);

#endif

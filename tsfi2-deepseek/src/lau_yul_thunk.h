#ifndef LAU_YUL_THUNK_H
#define LAU_YUL_THUNK_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    uint64_t d[4]; // 256-bit
} u256_t;

typedef struct {
    uint64_t address;
    int num_topics;
    u256_t topics[4];
    size_t data_size;
    uint8_t data[2048];
} YulEventLog;

typedef struct {
    u256_t stack[1024];
    int stack_ptr;
    uint8_t memory[524288];
    uint8_t calldata[65536];
    size_t calldatasize;
    uint8_t return_data[524288];
    size_t return_size;
    
    u256_t storage_keys[32768];
    u256_t storage_vals[32768];
    uint64_t storage_addrs[32768];
    int storage_count;
    bool reverted;
    uint64_t self_address;
    u256_t caller_address;
    
    YulEventLog logs[64];
    int log_count;
    bool is_initcode;
    bool storage_read_occurred;
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

// Getter for the accumulated transaction diyat tax total (the voltmeter value)
uint64_t lau_yul_get_diyat_tax_total(void);

// Retrieve the number of events emitted in the last execution transaction
int lau_yul_thunk_get_log_count(void);

// Retrieve a specific event log from the last execution transaction
bool lau_yul_thunk_get_log(int index, uint64_t *address, int *num_topics, u256_t *topics, uint8_t *data, size_t *data_size);

#endif

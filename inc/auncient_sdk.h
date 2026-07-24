#ifndef AUNCIENT_SDK_H
#define AUNCIENT_SDK_H

#include <stdint.h>
#include <stdbool.h>

#define ALU_OP_READ_KERMIT    0x10
#define ALU_OP_WRITE_ABD      0x20
#define ALU_OP_EVAL_ACKERMAN  0x30
#define ALU_OP_LOAD_SUB_XPL   0x40

#define SDK_NUM_NODES 4

// Rich status and diagnostic codes for ABI packets
typedef enum {
    SDK_STATUS_OK = 0,
    SDK_STATUS_ERR_QUORUM = 1,
    SDK_STATUS_ERR_STALE = 2,
    SDK_STATUS_ERR_SECURITY = 3,
    SDK_STATUS_ERR_GENERIC = 4,
    SDK_STATUS_COPROCESSOR_ROUTE = 5
} sdk_status_code_t;

// Auncient ABI Packet Layout for Coaxial Socket Transmission
typedef struct {
    uint8_t alu_opcode;
    uint8_t status_flag; // Maps to sdk_status_code_t
    uint16_t payload_length;
    uint32_t payload_value;
    uint64_t timestamp_counter;
    uint32_t writer_id;
} auncient_abi_packet_t;

typedef enum {
    SDK_QUORUM_MAJORITY,
    SDK_QUORUM_GRID,
    SDK_QUORUM_WEIGHTED
} sdk_quorum_type_t;

typedef struct {
    uint64_t counter;
    uint32_t writer_id;
} sdk_timestamp_t;

typedef struct {
    uint32_t value;
    sdk_timestamp_t ts;
} sdk_register_t;

typedef struct {
    sdk_register_t registers[SDK_NUM_NODES];
    uint32_t weights[SDK_NUM_NODES];
    int socket_fds[2]; // Coaxial access socket pair
} sdk_coaxial_env_t;

typedef struct {
    uint32_t cached_value;
    sdk_timestamp_t cached_ts;
    bool is_warm;
} sdk_kermit_cache_t;

typedef struct {
    sdk_coaxial_env_t *env;
    sdk_kermit_cache_t *cache;
    sdk_quorum_type_t quorum_type;
    uint32_t writer_id;
    uint8_t security_clearance; // Embedded security clearance level
} sdk_cics_context_t;

// Batched operation structure
typedef struct {
    uint8_t opcode;
    uint32_t value;
    bool approvals[SDK_NUM_NODES];
} sdk_batched_op_t;

// SDK Interface Functions
bool auncient_sdk_init_coaxial(sdk_coaxial_env_t *env);
void auncient_sdk_close_coaxial(sdk_coaxial_env_t *env);

bool auncient_sdk_configure_weights(sdk_coaxial_env_t *env, const uint32_t *weights);
bool auncient_sdk_set_quorum_policy(sdk_cics_context_t *ctx, sdk_quorum_type_t policy);
void auncient_sdk_void_registers(sdk_coaxial_env_t *env);

// Active Security Clearance check
bool auncient_sdk_check_clearance(const sdk_cics_context_t *ctx, uint32_t value);

// Precedence-Aware AUTODIN Spin-Lock Interface
bool auncient_sdk_autodin_spin_lock(sdk_cics_context_t *ctx, uint32_t lock_token, char precedence);
void auncient_sdk_autodin_spin_unlock(sdk_cics_context_t *ctx, uint32_t lock_token);

// .xpl Compiler to .dat.bin stream interface
bool auncient_sdk_compile_xpl_to_dat_bin(const char *xpl_source_path, const char *dat_bin_dest_path);

// Primary Executable (.bin) Loader and Executor (allocates dedicated AUTODIN environment in memory)
bool auncient_sdk_execute_primary_bin(const char *bin_path, uint32_t *results, int max_results);

// .dat.bin Binary Stream Executor (executes on an existing AUTODIN environment)
bool auncient_sdk_execute_dat_bin(sdk_cics_context_t *ctx, const char *dat_bin_path, uint32_t *results, int max_results);

bool auncient_sdk_alu_execute(sdk_cics_context_t *ctx, uint8_t alu_opcode, uint32_t target_val, const bool *approvals, uint32_t *result_val);

bool auncient_sdk_cics_exec(sdk_cics_context_t *ctx, uint32_t value, const bool *approvals);

bool auncient_sdk_batch_exec(sdk_cics_context_t *ctx, const sdk_batched_op_t *ops, int num_ops, uint32_t *results);

#endif // AUNCIENT_SDK_H

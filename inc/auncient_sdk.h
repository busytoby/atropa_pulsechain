#ifndef AUNCIENT_SDK_H
#define AUNCIENT_SDK_H

#include <stdint.h>
#include <stdbool.h>

#define ALU_OP_READ_KERMIT    0x10
#define ALU_OP_WRITE_ABD      0x20
#define ALU_OP_EVAL_ACKERMAN  0x30

#define SDK_NUM_NODES 4

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
} sdk_cics_context_t;

// SDK Interface Functions
bool auncient_sdk_init_coaxial(sdk_coaxial_env_t *env);
void auncient_sdk_close_coaxial(sdk_coaxial_env_t *env);

bool auncient_sdk_alu_execute(sdk_cics_context_t *ctx, uint8_t alu_opcode, uint32_t target_val, const bool *approvals, uint32_t *result_val);

bool auncient_sdk_cics_exec(sdk_cics_context_t *ctx, uint32_t value, const bool *approvals);

#endif // AUNCIENT_SDK_H

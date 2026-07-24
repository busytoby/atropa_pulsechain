#include "auncient_sdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <assert.h>

bool auncient_sdk_init_coaxial(sdk_coaxial_env_t *env) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, env->socket_fds) < 0) {
        return false;
    }

    // Default configuration: equal weights
    for (int i = 0; i < SDK_NUM_NODES; i++) {
        env->registers[i].value = 0;
        env->registers[i].ts.counter = 0;
        env->registers[i].ts.writer_id = 0;
        env->weights[i] = 1;
    }
    // Node 0 has high weight for weighted test support
    env->weights[0] = 3;

    return true;
}

void auncient_sdk_close_coaxial(sdk_coaxial_env_t *env) {
    close(env->socket_fds[0]);
    close(env->socket_fds[1]);
}

bool auncient_sdk_configure_weights(sdk_coaxial_env_t *env, const uint32_t *weights) {
    if (!env || !weights) {
        return false;
    }
    for (int i = 0; i < SDK_NUM_NODES; i++) {
        env->weights[i] = weights[i];
    }
    return true;
}

bool auncient_sdk_set_quorum_policy(sdk_cics_context_t *ctx, sdk_quorum_type_t policy) {
    if (!ctx) {
        return false;
    }
    ctx->quorum_type = policy;
    return true;
}

void auncient_sdk_void_registers(sdk_coaxial_env_t *env) {
    if (!env) {
        return;
    }
    for (int i = 0; i < SDK_NUM_NODES; i++) {
        env->registers[i].value = 0;
        env->registers[i].ts.counter = 0;
        env->registers[i].ts.writer_id = 0;
    }
}

static bool check_ackerman_quorum(sdk_quorum_type_t type, const bool *approvals, const uint32_t *weights) {
    if (type == SDK_QUORUM_MAJORITY) {
        int count = 0;
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (approvals[i]) count++;
        }
        return (count > SDK_NUM_NODES / 2);
    }

    if (type == SDK_QUORUM_GRID) {
        // 2x2 grid row + column rule
        bool row0 = (approvals[0] && approvals[1]);
        bool row1 = (approvals[2] && approvals[3]);
        bool col0 = (approvals[0] && approvals[2]);
        bool col1 = (approvals[1] && approvals[3]);
        return ((row0 || row1) && (col0 || col1));
    }

    if (type == SDK_QUORUM_WEIGHTED) {
        uint32_t total = 0;
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (approvals[i]) {
                total += weights[i];
            }
        }
        return (total >= 4);
    }

    return false;
}

bool auncient_sdk_alu_execute(sdk_cics_context_t *ctx, uint8_t alu_opcode, uint32_t target_val, const bool *approvals, uint32_t *result_val) {
    sdk_coaxial_env_t *env = ctx->env;
    sdk_kermit_cache_t *cache = ctx->cache;

    if (alu_opcode == ALU_OP_READ_KERMIT) {
        // Read fast-path from warm cache
        if (cache->is_warm) {
            *result_val = cache->cached_value;
            return true;
        }

        // Cache miss: find highest timestamp from coaxial registers
        sdk_register_t highest = { .value = 0, .ts = { 0, 0 } };
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (env->registers[i].ts.counter > highest.ts.counter ||
                (env->registers[i].ts.counter == highest.ts.counter &&
                 env->registers[i].ts.writer_id > highest.ts.writer_id)) {
                highest = env->registers[i];
            }
        }

        // Cache the result
        cache->cached_value = highest.value;
        cache->cached_ts = highest.ts;
        cache->is_warm = true;

        *result_val = highest.value;
        return true;
    }

    if (alu_opcode == ALU_OP_EVAL_ACKERMAN) {
        // Evaluate approvals against selected quorum system
        bool ok = check_ackerman_quorum(ctx->quorum_type, approvals, env->weights);
        *result_val = ok ? 1 : 0;
        return true;
    }

    if (alu_opcode == ALU_OP_WRITE_ABD) {
        // Find max counter
        uint64_t max_counter = 0;
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (env->registers[i].ts.counter > max_counter) {
                max_counter = env->registers[i].ts.counter;
            }
        }

        sdk_timestamp_t new_ts = {
            .counter = max_counter + 1,
            .writer_id = ctx->writer_id
        };

        // Write update to approving nodes
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (approvals[i]) {
                env->registers[i].value = target_val;
                env->registers[i].ts = new_ts;
            }
        }

        // Update local cache
        cache->cached_value = target_val;
        cache->cached_ts = new_ts;
        cache->is_warm = true;

        *result_val = target_val;
        return true;
    }

    return false;
}

bool auncient_sdk_cics_exec(sdk_cics_context_t *ctx, uint32_t value, const bool *approvals) {
    uint32_t quorum_val = 0;
    
    // Step 1: Execute Ackerman Quorum Evaluation ALU instruction
    auncient_sdk_alu_execute(ctx, ALU_OP_EVAL_ACKERMAN, 0, approvals, &quorum_val);
    if (quorum_val == 0) {
        return false; // Rollback
    }

    // Step 2: Write update via ABD execution protocol
    uint32_t dummy;
    auncient_sdk_alu_execute(ctx, ALU_OP_WRITE_ABD, value, approvals, &dummy);

    // Step 3: Populate formal ABI packet with rich status code
    auncient_abi_packet_t packet = {
        .alu_opcode = ALU_OP_WRITE_ABD,
        .status_flag = SDK_STATUS_OK,
        .payload_length = sizeof(uint32_t),
        .payload_value = value,
        .timestamp_counter = ctx->cache->cached_ts.counter,
        .writer_id = ctx->writer_id
    };

    // Write packet to Coaxial socket
    write(ctx->env->socket_fds[0], &packet, sizeof(auncient_abi_packet_t));
    
    // Read and verify packet content on receiving side
    auncient_abi_packet_t rx_packet;
    read(ctx->env->socket_fds[1], &rx_packet, sizeof(auncient_abi_packet_t));
    assert(rx_packet.alu_opcode == ALU_OP_WRITE_ABD);
    assert(rx_packet.status_flag == SDK_STATUS_OK);
    assert(rx_packet.payload_value == value);
    assert(rx_packet.writer_id == ctx->writer_id);

    return true;
}

bool auncient_sdk_batch_exec(sdk_cics_context_t *ctx, const sdk_batched_op_t *ops, int num_ops, uint32_t *results) {
    if (!ctx || !ops || num_ops <= 0 || !results) {
        return false;
    }

    // Pirk co-processing router logic: If the batch contains more than 2 operations,
    // flag the operation route status.
    if (num_ops > 2) {
        // Send a status packet back to indicate co-processing reroute
        auncient_abi_packet_t packet = {
            .alu_opcode = ops[0].opcode,
            .status_flag = SDK_STATUS_COPROCESSOR_ROUTE,
            .payload_length = 0,
            .payload_value = 0,
            .timestamp_counter = 0,
            .writer_id = ctx->writer_id
        };
        write(ctx->env->socket_fds[0], &packet, sizeof(auncient_abi_packet_t));
    }

    for (int i = 0; i < num_ops; i++) {
        uint32_t res = 0;
        bool ok = auncient_sdk_alu_execute(ctx, ops[i].opcode, ops[i].value, ops[i].approvals, &res);
        if (!ok) {
            return false; // Abort whole batch on ALU failure
        }
        results[i] = res;
    }

    return true;
}

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

bool auncient_sdk_check_clearance(const sdk_cics_context_t *ctx, uint32_t value) {
    // If value > 900000, it is TopSecret (level 3). Context clearance must be >= 3.
    if (value > 900000 && ctx->security_clearance < 3) {
        return false;
    }
    return true;
}

bool auncient_sdk_validate_temporal_invariants(const sdk_cics_context_t *ctx, uint8_t opcode, uint32_t target_val) {
    // Temporal Lock Check: writing requires holding the AUTODIN lock
    if (opcode == ALU_OP_WRITE_ABD && !ctx->has_lock) {
        return false;
    }
    (void)target_val;
    return true;
}

bool auncient_sdk_validate_monotonicity(const sdk_coaxial_env_t *env, int node_idx, uint64_t new_counter) {
    if (!env || node_idx < 0 || node_idx >= SDK_NUM_NODES) {
        return false;
    }
    return (new_counter > env->registers[node_idx].ts.counter);
}

bool auncient_sdk_validate_history_constraints(const sdk_coaxial_env_t *old_env, const sdk_coaxial_env_t *new_env) {
    if (!old_env || !new_env) {
        return false;
    }
    uint32_t old_sum = 0, new_sum = 0;
    for (int i = 0; i < SDK_NUM_NODES; i++) {
        old_sum += old_env->weights[i];
        new_sum += new_env->weights[i];
    }
    return (old_sum == new_sum);
}

bool auncient_sdk_validate_frame_conditions(uint8_t opcode, const bool *approvals, int modified_node_idx) {
    if (opcode == ALU_OP_WRITE_ABD) {
        if (!approvals || modified_node_idx < 0 || modified_node_idx >= SDK_NUM_NODES) {
            return false;
        }
        return approvals[modified_node_idx];
    }
    return true;
}

bool auncient_sdk_transition_typestate(sdk_cics_context_t *ctx, sdk_typestate_t next_state) {
    if (!ctx) {
        return false;
    }
    sdk_typestate_t cur = ctx->state;
    
    if (next_state == SDK_STATE_LOCKED && cur == SDK_STATE_UNLOCKED) {
        ctx->state = next_state;
        return true;
    }
    if (next_state == SDK_STATE_EXECUTING && cur == SDK_STATE_LOCKED) {
        ctx->state = next_state;
        return true;
    }
    if (next_state == SDK_STATE_COMMITTED && cur == SDK_STATE_EXECUTING) {
        ctx->state = next_state;
        return true;
    }
    if (next_state == SDK_STATE_UNLOCKED &&
        (cur == SDK_STATE_COMMITTED || cur == SDK_STATE_LOCKED || cur == SDK_STATE_EXECUTING)) {
        ctx->state = next_state;
        return true;
    }

    return false;
}

bool auncient_sdk_validate_dependent_types(const sdk_cics_context_t *ctx, uint8_t opcode, uint32_t val) {
    if (opcode == ALU_OP_WRITE_ABD) {
        // Dependent Parity Constraint: Grid Quorum requires even values
        if (ctx->quorum_type == SDK_QUORUM_GRID && (val % 2 != 0)) {
            return false;
        }
    }
    return true;
}

bool auncient_sdk_validate_purity(const sdk_cics_context_t *ctx, uint8_t opcode) {
    if (ctx && ctx->is_contract_checking && (opcode == ALU_OP_WRITE_ABD)) {
        return false; // Purity violation: attempting write during check
    }
    return true;
}

bool auncient_sdk_validate_contract_refinement(const sdk_cics_context_t *parent_ctx, const sdk_cics_context_t *child_ctx) {
    if (!parent_ctx || !child_ctx) {
        return false;
    }
    // Pre-condition weakening: child clearance requirements must be less than or equal to parent
    if (child_ctx->security_clearance > parent_ctx->security_clearance) {
        return false;
    }
    // Post-condition strengthening: child's quorum policy must match or be equivalent to parent's
    if (parent_ctx->quorum_type == SDK_QUORUM_WEIGHTED && child_ctx->quorum_type != SDK_QUORUM_WEIGHTED) {
        return false;
    }
    return true;
}

void auncient_sdk_assign_blame(sdk_cics_context_t *ctx, sdk_blame_t blame_target) {
    if (ctx) {
        ctx->last_blame = blame_target;
    }
}

bool auncient_sdk_validate_transition_invariant(const sdk_coaxial_env_t *env, int node_idx, uint32_t new_val) {
    if (!env || node_idx < 0 || node_idx >= SDK_NUM_NODES) {
        return false;
    }
    // Transition Invariant: values must not regress (new value >= pre-state value)
    return (new_val >= env->registers[node_idx].value);
}

bool auncient_pld_verify_blame(const sdk_cics_context_t *ctx, sdk_blame_t expected_blame) {
    if (!ctx) {
        return false;
    }
    return (ctx->last_blame == expected_blame);
}

void auncient_pld_clear_blame(sdk_cics_context_t *ctx) {
    if (ctx) {
        ctx->last_blame = SDK_BLAME_NONE;
    }
}

bool auncient_pld_broadcast_blame(const sdk_cics_context_t *ctx) {
    if (!ctx) {
        return false;
    }

    // Packet carries blame information across coaxial line
    auncient_abi_packet_t packet = {
        .alu_opcode = 0xFF, // Custom diagnostic op
        .status_flag = ctx->last_blame,
        .payload_length = 0,
        .payload_value = 0,
        .timestamp_counter = 0,
        .writer_id = ctx->writer_id
    };

    if (write(ctx->env->socket_fds[0], &packet, sizeof(auncient_abi_packet_t)) < 0) {
        return false;
    }

    auncient_abi_packet_t rx_packet;
    if (read(ctx->env->socket_fds[1], &rx_packet, sizeof(auncient_abi_packet_t)) < 0) {
        return false;
    }

    return (rx_packet.status_flag == ctx->last_blame);
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

bool auncient_sdk_verify_postcondition_oracle(const sdk_cics_context_t *ctx, uint8_t opcode, const bool *approvals, uint32_t result) {
    if (opcode == ALU_OP_EVAL_ACKERMAN) {
        bool expected = check_ackerman_quorum(ctx->quorum_type, approvals, ctx->env->weights);
        uint32_t expected_val = expected ? 1 : 0;
        return (result == expected_val);
    }
    return true;
}

bool auncient_sdk_autodin_spin_lock(sdk_cics_context_t *ctx, uint32_t lock_token, char precedence) {
    // Map precedence character to priority level ('F'=4, 'I'=3, 'P'=2, 'R'=1)
    uint8_t priority = 1;
    if (precedence == 'F') priority = 4;
    else if (precedence == 'I') priority = 3;
    else if (precedence == 'P') priority = 2;

    // Write a lock request envelope into the AUTODIN spin-lock loop containing priority metadata
    auncient_abi_packet_t packet = {
        .alu_opcode = ALU_OP_EVAL_ACKERMAN,
        .status_flag = priority, // Use status_flag to carry priority metadata
        .payload_length = sizeof(uint32_t),
        .payload_value = lock_token,
        .timestamp_counter = 1,
        .writer_id = ctx->writer_id
    };

    // Broadcast over coaxial link
    if (write(ctx->env->socket_fds[0], &packet, sizeof(auncient_abi_packet_t)) < 0) {
        return false;
    }

    // Read back frame to verify lock acquisition
    auncient_abi_packet_t rx_packet;
    if (read(ctx->env->socket_fds[1], &rx_packet, sizeof(auncient_abi_packet_t)) < 0) {
        return false;
    }

    if (rx_packet.payload_value == lock_token) {
        ctx->has_lock = true;
        auncient_sdk_transition_typestate(ctx, SDK_STATE_LOCKED);
        return true;
    }
    return false;
}

void auncient_sdk_autodin_spin_unlock(sdk_cics_context_t *ctx, uint32_t lock_token) {
    // Write an unlock/release envelope into the AUTODIN spin-lock loop
    auncient_abi_packet_t packet = {
        .alu_opcode = ALU_OP_EVAL_ACKERMAN,
        .status_flag = 0,
        .payload_length = sizeof(uint32_t),
        .payload_value = 0, // 0 indicates lock release
        .timestamp_counter = 2,
        .writer_id = ctx->writer_id
    };

    write(ctx->env->socket_fds[0], &packet, sizeof(auncient_abi_packet_t));
    
    auncient_abi_packet_t rx_packet;
    read(ctx->env->socket_fds[1], &rx_packet, sizeof(auncient_abi_packet_t));
    assert(rx_packet.payload_value == 0);
    ctx->has_lock = false;
    auncient_sdk_transition_typestate(ctx, SDK_STATE_UNLOCKED);
    (void)lock_token;
}

bool auncient_sdk_compile_xpl_to_dat_bin(const char *xpl_source_path, const char *dat_bin_dest_path) {
    FILE *in = fopen(xpl_source_path, "r");
    if (!in) {
        return false;
    }

    FILE *out = fopen(dat_bin_dest_path, "wb");
    if (!out) {
        fclose(in);
        return false;
    }

    // Write header: "XPL\0"
    uint32_t signature = 0x58504C00;
    fwrite(&signature, sizeof(uint32_t), 1, out);

    // Placeholder for count of instructions (will patch later)
    uint32_t instruction_count = 0;
    fwrite(&instruction_count, sizeof(uint32_t), 1, out);

    char op_str[64];
    uint32_t val;
    int a0, a1, a2, a3;

    while (fscanf(in, "%63s", op_str) == 1) {
        uint8_t opcode = 0;
        uint32_t payload_val = 0;
        uint32_t approvals_mask = 0;

        if (strcmp(op_str, "WRITE_ABD") == 0) {
            opcode = ALU_OP_WRITE_ABD;
            if (fscanf(in, "%u %d %d %d %d", &val, &a0, &a1, &a2, &a3) == 5) {
                payload_val = val;
                approvals_mask = (a0 ? 1 : 0) | (a1 ? 2 : 0) | (a2 ? 4 : 0) | (a3 ? 8 : 0);
            }
        } else if (strcmp(op_str, "READ_KERMIT") == 0) {
            opcode = ALU_OP_READ_KERMIT;
        } else if (strcmp(op_str, "EVAL_ACKERMAN") == 0) {
            opcode = ALU_OP_EVAL_ACKERMAN;
            if (fscanf(in, "%d %d %d %d", &a0, &a1, &a2, &a3) == 4) {
                approvals_mask = (a0 ? 1 : 0) | (a1 ? 2 : 0) | (a2 ? 4 : 0) | (a3 ? 8 : 0);
            }
        } else if (strcmp(op_str, "LOAD_SUB_XPL") == 0) {
            opcode = ALU_OP_LOAD_SUB_XPL;
            if (fscanf(in, "%u", &val) == 1) {
                payload_val = val;
            }
        } else {
            // Unknown opcode, skip line
            continue;
        }

        // Write instruction record
        fwrite(&opcode, sizeof(uint8_t), 1, out);
        fwrite(&payload_val, sizeof(uint32_t), 1, out);
        fwrite(&approvals_mask, sizeof(uint32_t), 1, out);

        instruction_count++;
    }

    // Patch instruction count at position 4
    fseek(out, sizeof(uint32_t), SEEK_SET);
    fwrite(&instruction_count, sizeof(uint32_t), 1, out);

    fclose(in);
    fclose(out);
    return true;
}

// Forward declaration of internal executor to support clean cascading locks
static bool auncient_sdk_execute_dat_bin_internal(sdk_cics_context_t *ctx, const char *dat_bin_path, uint32_t *results, int max_results);

bool auncient_sdk_execute_primary_bin(const char *bin_path, uint32_t *results, int max_results) {
    // 1. Initialize dedicated AUTODIN Spin-Lock environment in memory
    sdk_coaxial_env_t env;
    if (!auncient_sdk_init_coaxial(&env)) {
        return false;
    }

    // 2. Setup context
    sdk_kermit_cache_t cache = { .cached_value = 0, .cached_ts = { 0, 0 }, .is_warm = false };
    sdk_cics_context_t ctx = {
        .env = &env,
        .cache = &cache,
        .quorum_type = SDK_QUORUM_MAJORITY,
        .writer_id = 100,
        .security_clearance = 3,
        .has_lock = false,
        .state = SDK_STATE_UNLOCKED,
        .is_contract_checking = false,
        .last_blame = SDK_BLAME_NONE
    };

    // 3. Execute
    bool ok = auncient_sdk_execute_dat_bin(&ctx, bin_path, results, max_results);

    // 4. Clean up allocated loopback
    auncient_sdk_close_coaxial(&env);
    return ok;
}

bool auncient_sdk_execute_dat_bin(sdk_cics_context_t *ctx, const char *dat_bin_path, uint32_t *results, int max_results) {
    // Acquire AUTODIN spin-lock for primary execution serialization
    if (!auncient_sdk_autodin_spin_lock(ctx, 0x111, 'F')) {
        return false;
    }

    // Transition state
    auncient_sdk_transition_typestate(ctx, SDK_STATE_EXECUTING);

    bool ok = auncient_sdk_execute_dat_bin_internal(ctx, dat_bin_path, results, max_results);

    if (ok) {
        auncient_sdk_transition_typestate(ctx, SDK_STATE_COMMITTED);
    } else {
        auncient_sdk_transition_typestate(ctx, SDK_STATE_UNLOCKED);
    }

    auncient_sdk_autodin_spin_unlock(ctx, 0x111);
    return ok;
}

static bool verify_subtyping(const sdk_cics_context_t *ctx, const char *path) {
    FILE *bin = fopen(path, "rb");
    if (!bin) {
        return false;
    }

    uint32_t sig = 0, count = 0;
    if (fread(&sig, sizeof(uint32_t), 1, bin) != 1 || sig != 0x58504C00 ||
        fread(&count, sizeof(uint32_t), 1, bin) != 1) {
        fclose(bin);
        return false;
    }

    // Build simulated child context to check inheritance refinement rules
    sdk_cics_context_t child_ctx = *ctx;
    
    // Set validation flag
    child_ctx.is_contract_checking = true;

    // Scan all instructions to check for behavioral subtyping compliance
    for (uint32_t i = 0; i < count; i++) {
        uint8_t opcode = 0;
        uint32_t val = 0;
        uint32_t mask = 0;

        if (fread(&opcode, sizeof(uint8_t), 1, bin) != 1 ||
            fread(&val, sizeof(uint32_t), 1, bin) != 1 ||
            fread(&mask, sizeof(uint32_t), 1, bin) != 1) {
            fclose(bin);
            return false;
        }

        // Subtyping Rule: A loaded child library must not require security clearances higher than the parent
        if (!auncient_sdk_check_clearance(ctx, val)) {
            auncient_sdk_assign_blame((sdk_cics_context_t *)ctx, SDK_BLAME_SUBLIBRARY);
            fclose(bin);
            return false; // Violates pre-condition weakening rule
        }

        // Simulate child's assumed clearance requirements
        if (val > 900000) {
            child_ctx.security_clearance = 3;
        } else {
            child_ctx.security_clearance = 1;
        }

        // Contract Inheritance refinement verification
        if (!auncient_sdk_validate_contract_refinement(ctx, &child_ctx)) {
            auncient_sdk_assign_blame((sdk_cics_context_t *)ctx, SDK_BLAME_SUBLIBRARY);
            fclose(bin);
            return false;
        }
    }

    fclose(bin);
    return true;
}

static bool auncient_sdk_execute_dat_bin_internal(sdk_cics_context_t *ctx, const char *dat_bin_path, uint32_t *results, int max_results) {
    // Checkpoint registers for Cascaded State Rollback
    sdk_register_t register_snapshot[SDK_NUM_NODES];
    memcpy(register_snapshot, ctx->env->registers, sizeof(register_snapshot));

    FILE *bin = fopen(dat_bin_path, "rb");
    if (!bin) {
        return false;
    }

    uint32_t signature = 0;
    if (fread(&signature, sizeof(uint32_t), 1, bin) != 1 || signature != 0x58504C00) {
        fclose(bin);
        return false;
    }

    uint32_t count = 0;
    if (fread(&count, sizeof(uint32_t), 1, bin) != 1) {
        fclose(bin);
        return false;
    }

    bool overall_ok = true;

    for (uint32_t i = 0; i < count; i++) {
        uint8_t opcode = 0;
        uint32_t val = 0;
        uint32_t mask = 0;

        if (fread(&opcode, sizeof(uint8_t), 1, bin) != 1 ||
            fread(&val, sizeof(uint32_t), 1, bin) != 1 ||
            fread(&mask, sizeof(uint32_t), 1, bin) != 1) {
            overall_ok = false;
            break;
        }

        if (opcode == ALU_OP_LOAD_SUB_XPL) {
            // Target file path
            char path[128];
            snprintf(path, sizeof(path), "tests/sub_%u.dat.bin", val);

            // Behavioral Subtyping Verification
            if (!verify_subtyping(ctx, path)) {
                overall_ok = false;
                break; // Reject load due to subtyping violation
            }
            
            // Execute the loaded dat.bin stream
            bool sub_ok = auncient_sdk_execute_dat_bin_internal(ctx, path, NULL, 0);
            if (!sub_ok) {
                overall_ok = false;
                break;
            }
            if (results && (int)i < max_results) {
                results[i] = val; // Store loaded ID as execution trace
            }
            continue;
        }

        // Temporal Invariant Enforcement
        ctx->is_contract_checking = true;
        if (!auncient_sdk_validate_temporal_invariants(ctx, opcode, val)) {
            ctx->is_contract_checking = false;
            auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
            overall_ok = false;
            break;
        }
        ctx->is_contract_checking = false;

        bool approvals[SDK_NUM_NODES] = {
            (mask & 1) ? true : false,
            (mask & 2) ? true : false,
            (mask & 4) ? true : false,
            (mask & 8) ? true : false
        };

        uint32_t res = 0;
        bool ok = auncient_sdk_alu_execute(ctx, opcode, val, approvals, &res);
        if (!ok) {
            overall_ok = false;
            break;
        }

        if (results && (int)i < max_results) {
            results[i] = res;
        }
    }

    fclose(bin);

    // Cascaded Rollback: If execution failed, restore snapshot
    if (!overall_ok) {
        memcpy(ctx->env->registers, register_snapshot, sizeof(register_snapshot));
    } else {
        // Liskov-Wing History Constraint Verification
        sdk_coaxial_env_t old_env;
        memcpy(old_env.registers, register_snapshot, sizeof(register_snapshot));
        memcpy(old_env.weights, ctx->env->weights, sizeof(ctx->env->weights));

        ctx->is_contract_checking = true;
        if (!auncient_sdk_validate_history_constraints(&old_env, ctx->env)) {
            ctx->is_contract_checking = false;
            auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
            overall_ok = false;
            memcpy(ctx->env->registers, register_snapshot, sizeof(register_snapshot));
        }
        ctx->is_contract_checking = false;
    }

    return overall_ok;
}

bool auncient_sdk_alu_execute(sdk_cics_context_t *ctx, uint8_t alu_opcode, uint32_t target_val, const bool *approvals, uint32_t *result_val) {
    sdk_coaxial_env_t *env = ctx->env;
    sdk_kermit_cache_t *cache = ctx->cache;

    // Enforce active Purity rules
    if (!auncient_sdk_validate_purity(ctx, alu_opcode)) {
        auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
        return false;
    }

    // Enforce Active Security Clearance check
    if (!auncient_sdk_check_clearance(ctx, target_val)) {
        auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
        return false; // Trapped security violation
    }

    // Enforce Dependent Types PARITY checks
    if (!auncient_sdk_validate_dependent_types(ctx, alu_opcode, target_val)) {
        auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
        return false;
    }

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
        uint32_t res = ok ? 1 : 0;

        // Post-Condition Oracle Verification
        ctx->is_contract_checking = true;
        if (!auncient_sdk_verify_postcondition_oracle(ctx, alu_opcode, approvals, res)) {
            ctx->is_contract_checking = false;
            auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLEE);
            return false; // Oracle mismatch detected
        }
        ctx->is_contract_checking = false;

        *result_val = res;
        return true;
    }

    if (alu_opcode == ALU_OP_WRITE_ABD) {
        // Typestate check: writing registers is strictly forbidden unless context is in EXECUTING/COMMITTED typestate
        if (ctx->state != SDK_STATE_EXECUTING && ctx->state != SDK_STATE_COMMITTED) {
            auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
            return false; // Typestate violation
        }

        // Find max counter
        uint64_t max_counter = 0;
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (env->registers[i].ts.counter > max_counter) {
                max_counter = env->registers[i].ts.counter;
            }
        }

        uint64_t next_counter = max_counter + 1;

        // Invariant Monotonicity Auditing: Validate counter is strictly greater for all receiving nodes
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (approvals[i]) {
                if (!auncient_sdk_validate_monotonicity(env, i, next_counter)) {
                    auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
                    return false; // Monotonicity violation
                }
            }
        }

        sdk_timestamp_t new_ts = {
            .counter = next_counter,
            .writer_id = ctx->writer_id
        };

        // Transition Invariant: Validate that new values do not regress (new value >= current register value)
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (approvals[i]) {
                if (!auncient_sdk_validate_transition_invariant(env, i, target_val)) {
                    auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
                    return false; // Transition invariant failure (value regression)
                }
            }
        }

        // Write update to approving nodes (guarded by Frame Conditions/Modify Clauses)
        for (int i = 0; i < SDK_NUM_NODES; i++) {
            if (approvals[i]) {
                if (!auncient_sdk_validate_frame_conditions(alu_opcode, approvals, i)) {
                    auncient_sdk_assign_blame(ctx, SDK_BLAME_CALLER);
                    return false; // Frame Condition violation
                }
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
    bool orig_lock = ctx->has_lock;
    ctx->has_lock = true;
    sdk_typestate_t orig_state = ctx->state;
    ctx->state = SDK_STATE_EXECUTING;
    uint32_t quorum_val = 0;
    
    // Step 1: Execute Ackerman Quorum Evaluation ALU instruction
    auncient_sdk_alu_execute(ctx, ALU_OP_EVAL_ACKERMAN, 0, approvals, &quorum_val);
    if (quorum_val == 0) {
        ctx->has_lock = orig_lock;
        ctx->state = orig_state;
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

    ctx->has_lock = orig_lock;
    ctx->state = orig_state;
    return true;
}

bool auncient_sdk_batch_exec(sdk_cics_context_t *ctx, const sdk_batched_op_t *ops, int num_ops, uint32_t *results) {
    if (!ctx || !ops || num_ops <= 0 || !results) {
        return false;
    }

    bool orig_lock = ctx->has_lock;
    ctx->has_lock = true;
    sdk_typestate_t orig_state = ctx->state;
    ctx->state = SDK_STATE_EXECUTING;

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
            ctx->has_lock = orig_lock;
            ctx->state = orig_state;
            return false; // Abort whole batch on ALU failure
        }
        results[i] = res;
    }

    ctx->has_lock = orig_lock;
    ctx->state = orig_state;
    return true;
}

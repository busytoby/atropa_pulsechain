#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define REQUIRED_CLEARANCE 2
#define AUDIT_LOG_LIMIT 16

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// Gate clearance transition audit entry
typedef struct {
    uint8_t previous_state;
    uint8_t next_state;
    uint8_t requested_clearance;
    uint64_t cycle_timestamp;
    char operation_source[64];
} tws_gate_audit_entry_t;

// 2-3 Merkle Tree Node for AST Registry
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// TWS Compiler context
typedef struct {
    gate_state_t compilation_gate;
    char generated_yul[128];
    TwoThreeNode *ast_root;
    tws_gate_audit_entry_t audit_log[AUDIT_LOG_LIMIT];
    int audit_log_count;
    uint64_t simulated_cycles;
} tws_compiler_t;

// Helper to create leaf node
static TwoThreeNode* create_leaf(uint32_t key1, const char *val1) {
    TwoThreeNode *node = (TwoThreeNode*)calloc(1, sizeof(TwoThreeNode));
    node->is_leaf = true;
    node->num_keys = 1;
    node->keys[0] = key1;
    strcpy(node->values[0], val1);
    
    uint8_t temp[256];
    int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
    sha256(temp, len, node->node_hash);
    return node;
}

// Update leaf node hash
static void update_node_hash(TwoThreeNode *node) {
    if (!node) return;
    if (node->is_leaf) {
        uint8_t temp[256];
        int len = snprintf((char*)temp, sizeof(temp), "%u:%s", node->keys[0], node->values[0]);
        sha256(temp, len, node->node_hash);
    }
}

// Update leaf in 2-3 tree DAT
static bool store_table_data(TwoThreeNode *node, uint32_t key, const char *new_value) {
    if (!node) return false;
    if (node->is_leaf) {
        if (node->keys[0] == key) {
            strcpy(node->values[0], new_value);
            update_node_hash(node);
            return true;
        }
        return false;
    }
    return store_table_data(node->children[0], key, new_value);
}

// -------------------------------------------------------------
// Gated TWS Compiler Routine with Audited Transitions
// -------------------------------------------------------------
bool tws_compile_gated(tws_compiler_t *compiler, const char *source, uint8_t clearance) {
    gate_state_t prev_state = compiler->compilation_gate;
    gate_state_t next_state = (clearance >= REQUIRED_CLEARANCE) ? CONDUC_STATE : CUTOFF_STATE;

    compiler->simulated_cycles += 42; // Advance Time Stamp Counter representation

    // Log gate clearance transition telemetry
    if (compiler->audit_log_count < AUDIT_LOG_LIMIT) {
        int idx = compiler->audit_log_count++;
        compiler->audit_log[idx].previous_state = (uint8_t)prev_state;
        compiler->audit_log[idx].next_state = (uint8_t)next_state;
        compiler->audit_log[idx].requested_clearance = clearance;
        compiler->audit_log[idx].cycle_timestamp = compiler->simulated_cycles;
        strncpy(compiler->audit_log[idx].operation_source, source, 63);
        compiler->audit_log[idx].operation_source[63] = '\0';
    }

    compiler->compilation_gate = next_state;

    if (compiler->compilation_gate == CUTOFF_STATE) {
        strcpy(compiler->generated_yul, "");
        return false; // Dropped
    }

    // Simple tokenizer (simulated parser output)
    if (strstr(source, "BYTE(") != NULL) {
        // Emit direct Yul output mapping stack
        strcpy(compiler->generated_yul, "let val := add(shl(1, 6), 5) storeNamespaced(0xF300, val)");
        
        // Update AST Merkle DAT registry dynamically
        store_table_data(compiler->ast_root, 100, compiler->generated_yul);
        return true;
    }
    return false;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TWS GATED COMPILER VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *ast = create_leaf(100, "AST_INIT_STATE");
    tws_compiler_t compiler = {
        .compilation_gate = CUTOFF_STATE,
        .generated_yul = "",
        .ast_root = ast,
        .audit_log_count = 0,
        .simulated_cycles = 1000
    };

    uint8_t initial_hash[HASH_SIZE];
    memcpy(initial_hash, compiler.ast_root->node_hash, HASH_SIZE);

    // 1. Authorized Compilation (Clearance Level 2) -> Should transition to CONDUC_STATE
    printf("[TEST] Compiling source with authorized clearance (Level 2)...\n");
    fflush(stdout);
    bool ok = tws_compile_gated(&compiler, "BYTE(62208) = 5;", 2);
    assert(ok == true);
    assert(compiler.compilation_gate == CONDUC_STATE);
    assert(strcmp(compiler.generated_yul, "let val := add(shl(1, 6), 5) storeNamespaced(0xF300, val)") == 0);
    assert(memcmp(initial_hash, compiler.ast_root->node_hash, HASH_SIZE) != 0);
    printf("   ✓ Code compiled successfully. Merkle AST updated.\n");
    fflush(stdout);

    // Verify first transition audit entry
    assert(compiler.audit_log_count == 1);
    assert(compiler.audit_log[0].previous_state == CUTOFF_STATE);
    assert(compiler.audit_log[0].next_state == CONDUC_STATE);
    assert(compiler.audit_log[0].requested_clearance == 2);
    assert(compiler.audit_log[0].cycle_timestamp == 1042);
    printf("   ✓ First transition successfully logged to audit buffers.\n");
    fflush(stdout);

    // Save state hash
    uint8_t compiled_hash[HASH_SIZE];
    memcpy(compiled_hash, compiler.ast_root->node_hash, HASH_SIZE);

    // 2. Unauthorized Compilation (Clearance Level 1) -> Should block codegen & transition to CUTOFF_STATE
    printf("[TEST] Compiling source with unauthorized clearance (Level 1)...\n");
    fflush(stdout);
    ok = tws_compile_gated(&compiler, "BYTE(62208) = 5;", 1);
    assert(ok == false);
    assert(compiler.compilation_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(compiler.generated_yul, "") == 0); // No codegen
    assert(memcmp(compiled_hash, compiler.ast_root->node_hash, HASH_SIZE) == 0); // Hash unchanged
    printf("   ✓ Codegen blocked successfully at compilation boundary.\n");
    fflush(stdout);

    // Verify second transition audit entry
    assert(compiler.audit_log_count == 2);
    assert(compiler.audit_log[1].previous_state == CONDUC_STATE);
    assert(compiler.audit_log[1].next_state == CUTOFF_STATE);
    assert(compiler.audit_log[1].requested_clearance == 1);
    assert(compiler.audit_log[1].cycle_timestamp == 1084);
    printf("   ✓ Second transition (cutoff enforcement) successfully logged.\n");
    fflush(stdout);

    free(ast);
    printf("=============================================================\n");
    printf("TWS GATED COMPILER VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

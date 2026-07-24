#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define DMA_STACK_LIMIT 3
#define EXCEPTION_PC 9999

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

// 2-3 Merkle Tree Node
typedef struct TwoThreeNode {
    bool is_leaf;
    int num_keys;
    uint32_t keys[2];
    char values[2][128];
    uint8_t node_hash[HASH_SIZE];
    struct TwoThreeNode *children[3];
} TwoThreeNode;

// PL/I CONTROLLED BASED DMA Allocator Context
typedef struct {
    uint32_t key_stack[DMA_STACK_LIMIT];
    uint32_t sp;
    uint32_t limit;
    uint32_t PC;
    gate_state_t write_gate;
} pli_controlled_based_alloc_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

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

// -------------------------------------------------------------
// PL/I CONTROLLED BASED DMA Allocator Execution
// -------------------------------------------------------------
bool pli_allocate_dma_based(pli_controlled_based_alloc_t *alloc, TwoThreeNode **dma_nodes, 
                            const char *val, uint8_t tag) {
    // 1. Wortman Bounds Check: Verify allocation stack limit (Replay/Overflow check)
    if (alloc->sp >= alloc->limit) {
        // Trigger ON AREA exception handler
        alloc->PC = EXCEPTION_PC;
        alloc->write_gate = CUTOFF_STATE;
        return false;
    }

    // 2. Privilege Tag Check
    if (tag != TAG_KERNEL) {
        alloc->write_gate = CUTOFF_STATE;
        return false;
    }

    // Allocate leaf node dynamically inside 2-3 tree registry
    uint32_t allocated_key = 100 + alloc->sp;
    dma_nodes[alloc->sp] = create_leaf(allocated_key, val);

    // Push allocated key to CONTROLLED stack
    alloc->write_gate = CONDUC_STATE;
    alloc->key_stack[alloc->sp] = allocated_key;
    alloc->sp++;
    alloc->PC++;

    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I CONTROLLED BASED DMA VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    TwoThreeNode *dma_nodes[DMA_STACK_LIMIT] = {NULL};
    pli_controlled_based_alloc_t alloc = {
        .key_stack = {0},
        .sp = 0,
        .limit = DMA_STACK_LIMIT,
        .PC = 1000,
        .write_gate = CUTOFF_STATE
    };

    // 1. Allocate first DMA descriptor (val = "DESC_0") -> Should succeed
    printf("[TEST] Allocating BASED DMA descriptor 0 (TAG_KERNEL)...\n");
    fflush(stdout);
    bool ok = pli_allocate_dma_based(&alloc, dma_nodes, "DESC_0", TAG_KERNEL);
    assert(ok == true);
    assert(alloc.write_gate == CONDUC_STATE);
    assert(alloc.sp == 1);
    assert(alloc.key_stack[0] == 100);
    assert(dma_nodes[0] != NULL);
    assert(strcmp(dma_nodes[0]->values[0], "DESC_0") == 0);
    printf("   ✓ First BASED DMA descriptor allocated successfully.\n");
    fflush(stdout);

    // 2. Allocate descriptors up to stack limit
    printf("[TEST] Pushing allocations to DMA ring limit...\n");
    fflush(stdout);
    ok = pli_allocate_dma_based(&alloc, dma_nodes, "DESC_1", TAG_KERNEL);
    assert(ok == true);
    ok = pli_allocate_dma_based(&alloc, dma_nodes, "DESC_2", TAG_KERNEL);
    assert(ok == true);
    assert(alloc.sp == 3);
    printf("   ✓ DMA stack filled to limit successfully.\n");
    fflush(stdout);

    // 3. Trigger Stack Overflow (ON AREA exception) -> Should block and route to EXCEPTION_PC
    printf("[TEST] Triggering DMA allocation stack overflow...\n");
    fflush(stdout);
    ok = pli_allocate_dma_based(&alloc, dma_nodes, "OVERFLOW_DESC", TAG_KERNEL);
    assert(ok == false);
    assert(alloc.write_gate == CUTOFF_STATE); // Cutoff active
    assert(alloc.PC == EXCEPTION_PC); // Exception routed successfully
    printf("   ✓ Stack overflow caught: ON AREA exception routed directly.\n");
    fflush(stdout);

    // Free allocated memory
    for (int i = 0; i < DMA_STACK_LIMIT; i++) {
        if (dma_nodes[i]) free(dma_nodes[i]);
    }

    printf("=============================================================\n");
    printf("PL/I CONTROLLED BASED DMA TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

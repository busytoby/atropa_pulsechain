#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_COAXIAL_BASE 0x4000
#define MAX_COAXIAL_NETWORKS 4

typedef struct {
    uint8_t memory[0x20000]; // Expanded 128KB VM memory
    uint32_t active_coaxial_offsets[MAX_COAXIAL_NETWORKS];
    uint32_t network_count;
} SimulatedRAUContext;

// Initialize the RAU context with the default network
void rau_init(SimulatedRAUContext *ctx) {
    memset(ctx, 0, sizeof(SimulatedRAUContext));
    ctx->active_coaxial_offsets[0] = DEFAULT_COAXIAL_BASE;
    ctx->network_count = 1;
}

// ALU dynamically instantiates a new coaxial network and registers its memory page offset
uint32_t alu_create_coaxial_network(SimulatedRAUContext *ctx, uint32_t new_page_offset) {
    if (ctx->network_count >= MAX_COAXIAL_NETWORKS) return 0;
    
    printf("[ALU BUS] Dynamically instantiating new coaxial network at offset 0x%04X...\n", new_page_offset);
    ctx->active_coaxial_offsets[ctx->network_count] = new_page_offset;
    return ctx->network_count++;
}

// Peek operation over a specific coaxial network ID
uint64_t peek_coaxial_reg(SimulatedRAUContext *ctx, uint32_t network_id, uint32_t v_reg) {
    assert(network_id < ctx->network_count);
    uint32_t base = ctx->active_coaxial_offsets[network_id];
    uint32_t offset = base + (v_reg * 32);
    
    uint64_t val = 0;
    memcpy(&val, &ctx->memory[offset], sizeof(uint64_t));
    return val;
}

// Poke operation over a specific coaxial network ID
void poke_coaxial_reg(SimulatedRAUContext *ctx, uint32_t network_id, uint32_t v_reg, uint64_t val) {
    assert(network_id < ctx->network_count);
    uint32_t base = ctx->active_coaxial_offsets[network_id];
    uint32_t offset = base + (v_reg * 32);
    
    memcpy(&ctx->memory[offset], &val, sizeof(uint64_t));
}

int main(void) {
    printf("=============================================================\n");
    printf("xplos ALU DYNAMIC COAXIAL NETWORK CREATION TEST\n");
    printf("=============================================================\n");

    SimulatedRAUContext ctx;
    rau_init(&ctx);

    // 1. Write to the default coaxial network (Network 0 at 0x4000)
    printf("[TEST] Writing to Default Coaxial Network (ID 0)...\n");
    poke_coaxial_reg(&ctx, 0, 5, 11111);
    assert(peek_coaxial_reg(&ctx, 0, 5) == 11111);

    // 2. ALU dynamically instantiates a new coaxial network page at 0x5000 (ID 1)
    uint32_t new_net_id = alu_create_coaxial_network(&ctx, 0x5000);
    assert(new_net_id == 1);

    // 3. Write to the new coaxial network (ID 1)
    printf("[TEST] Writing to dynamically created Coaxial Network (ID 1)...\n");
    poke_coaxial_reg(&ctx, 1, 5, 99999);
    assert(peek_coaxial_reg(&ctx, 1, 5) == 99999);

    // 4. Verify context isolation: Network 0 remains unmodified
    printf("\n[TEST] Verifying network isolation invariants...\n");
    uint64_t old_net_val = peek_coaxial_reg(&ctx, 0, 5);
    uint64_t new_net_val = peek_coaxial_reg(&ctx, 1, 5);

    assert(old_net_val == 11111);
    assert(new_net_val == 99999);

    printf("   ✓ Network 0 (Default) Register V5 value: %lu\n", old_net_val);
    printf("   ✓ Network 1 (Dynamic) Register V5 value: %lu\n", new_net_val);
    printf("   ✓ Context Isolation successfully preserved!\n");

    printf("=============================================================\n");
    printf("DYNAMIC COAXIAL NETWORK CREATION TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

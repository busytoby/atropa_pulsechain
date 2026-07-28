#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

// Simulated ALU Coaxial memory page offsets
#define COAXIAL_BUS_BASE 0x4000
#define WMQ_BASE 0x4800

typedef struct {
    uint8_t memory[0x10000]; // 64KB VM Memory page
    uint32_t active_core_id;
} SimulatedVMContext;

// Access Pattern 1: Direct Memory-Mapped Read/Write (Coaxial Gating Peek-Poke)
void alu_access_pattern_direct(SimulatedVMContext *ctx, uint32_t v_reg, uint64_t val) {
    uint32_t offset = COAXIAL_BUS_BASE + (v_reg * 32);
    // Write 64-bit value to the 32-byte slot (little-endian write simulation)
    memcpy(&ctx->memory[offset], &val, sizeof(uint64_t));
}

uint64_t alu_access_pattern_direct_read(SimulatedVMContext *ctx, uint32_t v_reg) {
    uint32_t offset = COAXIAL_BUS_BASE + (v_reg * 32);
    uint64_t val = 0;
    memcpy(&val, &ctx->memory[offset], sizeof(uint64_t));
    return val;
}

// Access Pattern 2: Transaction-Level Call (Dynamic Dispatch using selectors)
void alu_access_pattern_dispatch_write(SimulatedVMContext *ctx, uint32_t v_reg, uint64_t val) {
    // Simulates calling selector 0x022b9c34 (write_reg)
    if (v_reg == 0) return; // V0 Invariant (ignored)
    alu_access_pattern_direct(ctx, v_reg, val);
}

uint64_t alu_access_pattern_dispatch_read(SimulatedVMContext *ctx, uint32_t v_reg) {
    // Simulates calling selector 0x011a8b23 (read_reg)
    if (v_reg == 0) return 0; // V0 Invariant (constant 0)
    return alu_access_pattern_direct_read(ctx, v_reg);
}

// Access Pattern 3: WinchesterMQ Synchronization Routing
void alu_access_pattern_wmq_sync(SimulatedVMContext *ctx, uint32_t target_core_id, uint32_t v_reg) {
    uint64_t val = alu_access_pattern_dispatch_read(ctx, v_reg);
    
    // Write synchronization packet to WMQ SCSI Ports:
    // 0x4800 -> target_core_id
    // 0x4820 -> v_reg
    // 0x4840 -> val
    memcpy(&ctx->memory[WMQ_BASE], &target_core_id, sizeof(uint32_t));
    memcpy(&ctx->memory[WMQ_BASE + 0x20], &v_reg, sizeof(uint32_t));
    memcpy(&ctx->memory[WMQ_BASE + 0x40], &val, sizeof(uint64_t));
}

int main(void) {
    printf("=============================================================\n");
    printf("xplos ALU-TO-RAU INTERACTION AND ACCESS PATTERNS UNIT TEST\n");
    printf("=============================================================\n");

    SimulatedVMContext ctx;
    memset(&ctx, 0, sizeof(SimulatedVMContext));
    ctx.active_core_id = 1;

    // --- TEST PATTERN 1: Direct Coaxial Gating ---
    printf("[TEST] Access Pattern 1: Direct Coaxial Gating (Peek/Poke)...\n");
    alu_access_pattern_direct(&ctx, 5, 77777);
    uint64_t val1 = alu_access_pattern_direct_read(&ctx, 5);
    assert(val1 == 77777);
    printf("   ✓ Direct Coaxial Write and Read succeeded (V5 value: %lu)\n", val1);

    // --- TEST PATTERN 2: Selector Dynamic Dispatch ---
    printf("\n[TEST] Access Pattern 2: Selector Dynamic Dispatch (V0 check)...\n");
    // Write value to V0 (should be ignored)
    alu_access_pattern_dispatch_write(&ctx, 0, 9999);
    uint64_t val_v0 = alu_access_pattern_dispatch_read(&ctx, 0);
    assert(val_v0 == 0); // V0 must remain 0
    printf("   ✓ V0 Invariant read returned: %lu (Expected: 0)\n", val_v0);

    // Write value to V10 and dispatch read
    alu_access_pattern_dispatch_write(&ctx, 10, 88888);
    uint64_t val_v10 = alu_access_pattern_dispatch_read(&ctx, 10);
    assert(val_v10 == 88888);
    printf("   ✓ Dispatch Write and Read succeeded (V10 value: %lu)\n", val_v10);

    // --- TEST PATTERN 3: WinchesterMQ Synchronization ---
    printf("\n[TEST] Access Pattern 3: WinchesterMQ Synchronization Routing...\n");
    alu_access_pattern_wmq_sync(&ctx, 2, 10);

    // Verify WMQ port outputs
    uint32_t target_core = 0;
    uint32_t target_vreg = 0;
    uint64_t target_val = 0;

    memcpy(&target_core, &ctx.memory[WMQ_BASE], sizeof(uint32_t));
    memcpy(&target_vreg, &ctx.memory[WMQ_BASE + 0x20], sizeof(uint32_t));
    memcpy(&target_val, &ctx.memory[WMQ_BASE + 0x40], sizeof(uint64_t));

    assert(target_core == 2);
    assert(target_vreg == 10);
    assert(target_val == 88888);

    printf("   ✓ WMQ Target Core ID: %u (Expected: 2)\n", target_core);
    printf("   ✓ WMQ Target V_Reg  : %u (Expected: 10)\n", target_vreg);
    printf("   ✓ WMQ Data Value    : %lu (Expected: 88888)\n", target_val);

    printf("=============================================================\n");
    printf("ALU-TO-RAU INTEGRATION TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

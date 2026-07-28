#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_ALUS 4
#define MOTZKIN_PRIME 953467954114363ULL

// Simulated ALU state mapping
typedef struct {
    uint32_t alu_id;
    uint64_t registers[4]; // R0-R3
    uint64_t defcon_latch;
    uint64_t fieldata_model;
    uint64_t range_value;
    bool is_active;
} SimulatedALU;

// Simulated XCOM Active Bus
typedef struct {
    SimulatedALU alus[MAX_ALUS];
    int alu_count;
    uint32_t active_transactions;
} XCOMActiveBus;

// Initialize the Active Bus
void xplos_bus_init(XCOMActiveBus *bus) {
    if (!bus) return;
    memset(bus, 0, sizeof(XCOMActiveBus));
    bus->active_transactions = 0;
}

// Register an ALU component on the bus
int xplos_bus_register_alu(XCOMActiveBus *bus, uint32_t alu_id) {
    if (!bus || bus->alu_count >= MAX_ALUS) return -1;
    SimulatedALU *alu = &bus->alus[bus->alu_count++];
    alu->alu_id = alu_id;
    memset(alu->registers, 0, sizeof(alu->registers));
    alu->defcon_latch = 0;
    alu->fieldata_model = 100; // Standard base model weight
    alu->range_value = 0;
    alu->is_active = true;
    return 0;
}

// Simulate XCOM active message transport with inline ALU-to-ALU routing
int xplos_xcom_route_active_message(XCOMActiveBus *bus, uint32_t src_alu_idx, uint32_t dst_alu_idx, uint16_t target_reg, uint64_t payload) {
    if (!bus || src_alu_idx >= (uint32_t)bus->alu_count || dst_alu_idx >= (uint32_t)bus->alu_count) {
        return -1;
    }

    SimulatedALU *src = &bus->alus[src_alu_idx];
    SimulatedALU *dst = &bus->alus[dst_alu_idx];

    // Assert that source and destination are active
    if (!src->is_active || !dst->is_active) return -2;

    bus->active_transactions++;

    // Translate and write directly to target ALU's register space (Inter-ALU access)
    if (target_reg == 0x4080) {
        dst->defcon_latch = payload;
    } else if (target_reg == 0x4800) {
        dst->fieldata_model = payload;
    } else if (target_reg == 0x4840) {
        dst->range_value = payload;
    } else {
        // Default general register routing
        dst->registers[0] = payload;
    }

    return 0;
}

int main(void) {
    printf("=============================================================\n");
    printf("xplos ACTIVE XCOM BUS & MULTI-ALU INTEGRATION TEST\n");
    printf("=============================================================\n");

    XCOMActiveBus bus;
    xplos_bus_init(&bus);

    // 1. Register multiple ALU components on the active bus
    printf("[TEST] Registering ALU_0 (Primary Compiler Core) on the bus...\n");
    int res0 = xplos_bus_register_alu(&bus, 100);
    assert(res0 == 0);

    printf("[TEST] Registering ALU_1 (Security Auditor Core) on the bus...\n");
    int res1 = xplos_bus_register_alu(&bus, 101);
    assert(res1 == 0);

    assert(bus.alu_count == 2);
    printf("   ✓ Registered ALUs: %d\n", bus.alu_count);

    // 2. Perform active transmission (ALU_0 sends a DEFCON_LATCH trigger to ALU_1)
    printf("[TEST] ALU_0 emitting DEFCON_LATCH update to ALU_1 over XCOM Bus...\n");
    int route_res1 = xplos_xcom_route_active_message(&bus, 0, 1, 0x4080, 1);
    assert(route_res1 == 0);

    // Verify state propagation on target ALU
    assert(bus.alus[1].defcon_latch == 1);
    printf("   ✓ ALU_1 Latch State updated to: %lu (Expected: 1)\n", bus.alus[1].defcon_latch);

    // 3. Perform inter-ALU cross-conduction (ALU_1 modifies ALU_0's FIELDATA_MODEL directly)
    printf("[TEST] ALU_1 updating ALU_0's FIELDATA_MODEL to 250 over XCOM Bus...\n");
    int route_res2 = xplos_xcom_route_active_message(&bus, 1, 0, 0x4800, 250);
    assert(route_res2 == 0);

    // Verify state propagation on target ALU
    assert(bus.alus[0].fieldata_model == 250);
    printf("   ✓ ALU_0 Model Weight updated to: %lu (Expected: 250)\n", bus.alus[0].fieldata_model);

    // Verify transaction count on active bus
    assert(bus.active_transactions == 2);
    printf("   ✓ Total Bus Transactions Handled: %d\n", bus.active_transactions);

    printf("=============================================================\n");
    printf("xplos ACTIVE BUS INTEGRATION TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");

    return 0;
}

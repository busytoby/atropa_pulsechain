#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    bool telemetry_lock; // AUTODIN network lock state
    uint32_t shared_bus_value;
} autodin_bus_t;

typedef struct {
    uint32_t pc;
    uint32_t accumulator;
    bool wait_state_active; // Suspended due to DYSEAC interrupt gating
    bool halted;
} dyseac_node_t;

// Tick the DYSEAC CPU node under AUTODIN control
void tick_dyseac_node(dyseac_node_t *node, autodin_bus_t *bus, uint8_t opcode, uint32_t operand) {
    if (node->halted) return;

    // S. N. Alexander dynamic interrupt gating: pause clock if wait-state is active
    if (node->wait_state_active) {
        if (bus->telemetry_lock) {
            printf("   [CLK GATED] Node in wait-state. Autodin bus is locked.\n");
            return; // Clock cycles suspended: do nothing
        } else {
            node->wait_state_active = false;
            printf("   [CLK RESUME] Autodin lock released. Node resuming execution.\n");
        }
    }

    // Execute instruction
    if (opcode == 0x01) { // Normal write to accumulator
        node->accumulator = operand;
        node->pc++;
    } else if (opcode == 0x02) { // Write to shared AUTODIN bus (ALU_OP_WRITE_ABD)
        if (bus->telemetry_lock) {
            node->wait_state_active = true;
            printf("   [WAIT STATE] Autodin bus locked. Suspending write of value %u.\n", operand);
            return; // Enter wait-state, do not increment PC
        }
        bus->shared_bus_value = operand;
        node->pc++;
        printf("   [BUS WRITE] Successfully wrote value %u to Autodin bus.\n", operand);
    }
}

int main(void) {
    printf("=============================================================\n");
    printf("AUTODIN DYSEAC WAIT-STATE SYNCHRONIZATION SUITE\n");
    printf("=============================================================\n");

    autodin_bus_t bus = { .telemetry_lock = false, .shared_bus_value = 0 };
    dyseac_node_t node = { .pc = 0, .accumulator = 0, .wait_state_active = false, .halted = false };

    // 1. Normal execution write
    printf("[TEST] Executing write to unlocked Autodin bus...\n");
    tick_dyseac_node(&node, &bus, 0x02, 12345);
    assert(bus.shared_bus_value == 12345);
    assert(node.wait_state_active == false);
    assert(node.pc == 1);
    printf("   ✓ Write completed instantly. PC updated to 1.\n");

    // 2. Conflict execution: Write to locked Autodin bus (enters wait-state)
    printf("[TEST] Locking Autodin bus and attempting write (expecting suspension)...\n");
    bus.telemetry_lock = true;
    
    // Attempt write of 99999
    tick_dyseac_node(&node, &bus, 0x02, 99999);
    assert(node.wait_state_active == true);
    assert(bus.shared_bus_value == 12345); // Value should NOT have changed yet
    assert(node.pc == 1); // PC should NOT have incremented
    printf("   ✓ Write successfully suspended and node entered wait-state.\n");

    // 3. Ticking while locked (remains gated)
    printf("[TEST] Ticking node while bus remains locked...\n");
    tick_dyseac_node(&node, &bus, 0x02, 99999);
    assert(node.wait_state_active == true);
    assert(bus.shared_bus_value == 12345);
    assert(node.pc == 1);
    printf("   ✓ Gating confirmed. Execution cycle stayed paused.\n");

    // 4. Release lock and resume execution
    printf("[TEST] Releasing Autodin lock and retrying write...\n");
    bus.telemetry_lock = false;
    
    // Ticking now will resume and complete the write
    tick_dyseac_node(&node, &bus, 0x02, 99999);
    assert(node.wait_state_active == false);
    assert(bus.shared_bus_value == 99999); // Value updated
    assert(node.pc == 2); // PC incremented
    printf("   ✓ Node resumed automatically and completed bus write.\n");

    printf("=============================================================\n");
    printf("AUTODIN DYSEAC WAIT-STATE TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

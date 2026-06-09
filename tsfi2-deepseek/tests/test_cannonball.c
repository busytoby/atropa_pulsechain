#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

// VM Poke Helper
void vm_poke(TsfiZmmVmState *vm, uint64_t addr, uint64_t val) {
    char cmd[512];
    char addr_hex[65];
    char val_hex[65];
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval_addr = (shift < 64) ? ((addr >> shift) & 0xFF) : 0;
        uint8_t byteval_val = (shift < 64) ? ((val >> shift) & 0xFF) : 0;
        sprintf(&addr_hex[i * 2], "%02x", byteval_addr);
        sprintf(&val_hex[i * 2], "%02x", byteval_val);
    }
    sprintf(cmd, "YULEXEC \"cpu6502\", \"8029e7c0%s%s\"", addr_hex, val_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
}

// VM Peek Helper
uint64_t vm_peek(TsfiZmmVmState *vm, uint64_t addr) {
    char cmd[512];
    char addr_hex[65];
    for (int i = 0; i < 32; i++) {
        int shift = (31 - i) * 8;
        uint8_t byteval = (shift < 64) ? ((addr >> shift) & 0xFF) : 0;
        sprintf(&addr_hex[i * 2], "%02x", byteval);
    }
    sprintf(cmd, "YULEXEC \"cpu6502\", \"7861d269%s\"", addr_hex);
    vm->output_pos = 0;
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t res = 0;
    size_t len = strlen(vm->output_buffer);
    if (len >= 64) {
        char val_str[17];
        strncpy(val_str, &vm->output_buffer[len - 16], 16);
        val_str[16] = '\0';
        res = strtoull(val_str, NULL, 16);
    }
    return res;
}

int main() {
    printf("=== TSFi Cannonball Blitz Emulation Verification Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    printf("[BLITZ] Compiling and Initializing cpu6502.yul VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // Cannonball Blitz Registers:
    // 54912 ($D680): Player X Position (0-39)
    // 54913 ($D681): Player Y Position (0-24)
    // 54914 ($D682): Player Velocity X
    // 54915 ($D683): Player Velocity Y
    // 54916 ($D684): Cannonball 1 X Position
    // 54917 ($D685): Cannonball 1 Y Position
    // 54918 ($D686): Cannonball 1 Active (bool)
    // 54919 ($D687): Cannonball 2 X Position
    // 54920 ($D688): Cannonball 2 Y Position
    // 54921 ($D689): Cannonball 2 Active (bool)
    // 54925 ($D68D): Player Score
    // 54927 ($D68F): Physics Tick Trigger
    // 54928 ($D690): Collision Flag

    // Initial setup
    vm_poke(&vm, 54912, 10);  // Player X = 10
    vm_poke(&vm, 54913, 24);  // Player Y = 24 (on scaffold)
    vm_poke(&vm, 54914, 1);   // Player Vel X = 1
    vm_poke(&vm, 54915, 0);   // Player Vel Y = 0
    vm_poke(&vm, 54925, 0);   // Score = 0
    vm_poke(&vm, 54928, 0);   // Collision = 0

    // Set Cannonball 1 (Active, Y = 24, X = 5)
    vm_poke(&vm, 54916, 5);
    vm_poke(&vm, 54917, 24);
    vm_poke(&vm, 54918, 1);

    // Set Cannonball 2 (Active, Y = 24, X = 15)
    vm_poke(&vm, 54919, 15);
    vm_poke(&vm, 54920, 24);
    vm_poke(&vm, 54921, 1);

    // --- TEST 1: Physics Tick updates coordinates (Speed, Direction, and Score) ---
    printf("[BLITZ] Test 1: Simulating physics tick (No Collisions)...\n");
    vm_poke(&vm, 54927, 1); // Trigger Physics Tick
    
    uint64_t px = vm_peek(&vm, 54912);
    uint64_t ox1 = vm_peek(&vm, 54916);
    uint64_t ox2 = vm_peek(&vm, 54919);
    uint64_t score = vm_peek(&vm, 54925);
    uint64_t collided = vm_peek(&vm, 54928);
    
    printf("  Player X: %lu (Expected 11)\n", px);
    printf("  Cannonball 1 X: %lu (Expected 6)\n", ox1);
    printf("  Cannonball 2 X: %lu (Expected 13)\n", ox2);
    printf("  Player Score: %lu (Expected 10)\n", score);
    printf("  Collision Flag: %lu (Expected 0)\n", collided);
    
    assert(px == 11);
    assert(ox1 == 6);
    assert(ox2 == 13);
    assert(score == 10);
    assert(collided == 0);
    printf("PASS\n");

    // --- TEST 2: Collision with Cannonball 2 ---
    // Player is at X=11. Cannonball 2 is at X=13.
    // If we trigger another tick:
    //   Player moves to X=12
    //   Cannonball 2 moves to X=11 (subtracts 2)
    //   If we trigger a third tick:
    //     Player moves to X=13
    //     Cannonball 2 moves to X=9
    // Wait! Let's force a direct collision by placing Cannonball 2 at X=12 and Y=24.
    printf("[BLITZ] Test 2: Simulating direct collision with Cannonball 2...\n");
    vm_poke(&vm, 54912, 11); // Player X = 11
    vm_poke(&vm, 54914, 1);  // Player Vel X = 1 (will move to 12)
    vm_poke(&vm, 54919, 14); // Cannonball 2 X = 14 (will move to 12)
    
    vm_poke(&vm, 54927, 1);  // Trigger Physics Tick
    
    collided = vm_peek(&vm, 54928);
    px = vm_peek(&vm, 54912);
    uint64_t py = vm_peek(&vm, 54913);
    printf("  Collision Flag: %lu (Expected 1)\n", collided);
    printf("  Reset Player X: %lu (Expected 0)\n", px);
    printf("  Reset Player Y: %lu (Expected 24)\n", py);
    
    assert(collided == 1);
    assert(px == 0);
    assert(py == 24);
    printf("PASS\n");

    // --- TEST 3: Jump Air-time Routine ---
    printf("[BLITZ] Test 3: Simulating jump air-time routine...\n");
    // Reset player position and disable obstacles
    vm_poke(&vm, 54912, 10);  // Player X = 10
    vm_poke(&vm, 54913, 24);  // Player Y = 24 (on scaffold)
    vm_poke(&vm, 54914, 1);   // Player Vel X = 1
    vm_poke(&vm, 54915, 0);   // Player Vel Y = 0
    vm_poke(&vm, 54928, 0);   // Clear collision
    vm_poke(&vm, 54918, 0);   // Deactivate Cannonball 1
    vm_poke(&vm, 54921, 0);   // Deactivate Cannonball 2
    vm_poke(&vm, 54926, 0);   // Reset jump timer

    // Trigger Jump action
    vm_poke(&vm, 54929, 1);   // Action = 1 (Jump)

    // Tick 1
    vm_poke(&vm, 54927, 1);
    py = vm_peek(&vm, 54913);
    uint64_t timer = vm_peek(&vm, 54926);
    printf("  Tick 1 - Player Y: %lu (Expected 22), Jump Timer: %lu (Expected 3)\n", py, timer);
    assert(py == 22);
    assert(timer == 3);

    // Tick 2
    vm_poke(&vm, 54927, 1);
    py = vm_peek(&vm, 54913);
    timer = vm_peek(&vm, 54926);
    printf("  Tick 2 - Player Y: %lu (Expected 20), Jump Timer: %lu (Expected 2)\n", py, timer);
    assert(py == 20);
    assert(timer == 2);

    // Tick 3
    vm_poke(&vm, 54927, 1);
    py = vm_peek(&vm, 54913);
    timer = vm_peek(&vm, 54926);
    printf("  Tick 3 - Player Y: %lu (Expected 18), Jump Timer: %lu (Expected 1)\n", py, timer);
    assert(py == 18);
    assert(timer == 1);

    // Tick 4
    vm_poke(&vm, 54927, 1);
    py = vm_peek(&vm, 54913);
    timer = vm_peek(&vm, 54926);
    printf("  Tick 4 - Player Y: %lu (Expected 16 - Scaffold Landed), Jump Timer: %lu (Expected 0)\n", py, timer);
    assert(py == 16);
    assert(timer == 0);

    // Tick 5: Timer is 0. Since Player is at Y=16 (scaffold), they should stay at Y=16
    vm_poke(&vm, 54927, 1);
    py = vm_peek(&vm, 54913);
    timer = vm_peek(&vm, 54926);
    printf("  Tick 5 - Player Y: %lu (Expected 16), Jump Timer: %lu (Expected 0)\n", py, timer);
    assert(py == 16);
    assert(timer == 0);
    printf("PASS\n");

    printf("=== CANNONBALL BLITZ TESTS ALL PASSED ===\n");
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}

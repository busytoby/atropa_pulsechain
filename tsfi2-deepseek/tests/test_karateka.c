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
    printf("=== TSFi Karateka Emulation Verification Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    printf("[KARATEKA] Compiling and Initializing cpu6502.yul VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // Karateka MMIO Registers ($D700 to $D706):
    // 55040 ($D700): Player Stance Mode (0: Normal, 1: Combat)
    // 55041 ($D701): Player Strike (0: Idle, 1: Punch, 2: Kick)
    // 55042 ($D702): Enemy Stance Mode (0: Normal, 1: Combat)
    // 55043 ($D703): Distance between characters (0-255)
    // 55044 ($D704): Player Health (0-100)
    // 55045 ($D705): Enemy Health (0-100)
    // 55046 ($D706): Combat Tick Trigger

    // --- TEST 1: Punch in Range (Distance = 10, Enemy in Combat Stance) ---
    printf("[KARATEKA] Test 1: Punching enemy in combat stance at distance 10...\n");
    vm_poke(&vm, 55040, 1);   // Player Stance: Combat
    vm_poke(&vm, 55042, 1);   // Enemy Stance: Combat
    vm_poke(&vm, 55043, 10);  // Distance: 10 (Punch range <= 15)
    vm_poke(&vm, 55044, 100); // Player Health: 100
    vm_poke(&vm, 55045, 100); // Enemy Health: 100
    vm_poke(&vm, 55041, 1);   // Strike: Punch

    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    
    uint64_t enemy_health = vm_peek(&vm, 55045);
    uint64_t strike_state = vm_peek(&vm, 55041);
    printf("  Enemy Health: %lu (Expected 90)\n", enemy_health);
    printf("  Strike Register: %lu (Expected 0)\n", strike_state);
    assert(enemy_health == 90);
    assert(strike_state == 0);
    printf("PASS\n");

    // --- TEST 2: Kick in Range (Distance = 20, Enemy in Combat Stance) ---
    printf("[KARATEKA] Test 2: Kicking enemy in combat stance at distance 20...\n");
    vm_poke(&vm, 55040, 1);   // Player Stance: Combat
    vm_poke(&vm, 55042, 1);   // Enemy Stance: Combat
    vm_poke(&vm, 55043, 20);  // Distance: 20 (Kick range <= 25)
    vm_poke(&vm, 55045, 90);  // Enemy Health: 90
    vm_poke(&vm, 55041, 2);   // Strike: Kick

    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    
    enemy_health = vm_peek(&vm, 55045);
    printf("  Enemy Health: %lu (Expected 75)\n", enemy_health);
    assert(enemy_health == 75);
    printf("PASS\n");

    // --- TEST 3: Strike Misses due to Distance (Distance = 30) ---
    printf("[KARATEKA] Test 3: Kicking enemy at distance 30 (out of range)...\n");
    vm_poke(&vm, 55043, 30);  // Distance: 30
    vm_poke(&vm, 55045, 75);  // Enemy Health: 75
    vm_poke(&vm, 55041, 2);   // Strike: Kick

    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    
    enemy_health = vm_peek(&vm, 55045);
    printf("  Enemy Health: %lu (Expected 75 - no damage)\n", enemy_health);
    assert(enemy_health == 75);
    printf("PASS\n");

    // --- TEST 4: Instant Defeat (Vulnerable Running Stance, Enemy Stance = 0) ---
    printf("[KARATEKA] Test 4: Punching enemy in running/vulnerable stance (stance 0)...\n");
    vm_poke(&vm, 55042, 0);   // Enemy Stance: Running (vulnerable!)
    vm_poke(&vm, 55043, 10);  // Distance: 10
    vm_poke(&vm, 55045, 75);  // Enemy Health: 75
    vm_poke(&vm, 55041, 1);   // Strike: Punch

    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    
    enemy_health = vm_peek(&vm, 55045);
    printf("  Enemy Health: %lu (Expected 0 - Instant Defeat)\n", enemy_health);
    assert(enemy_health == 0);
    printf("PASS\n");

    // --- TEST 5: Stance Dodging (Low Stance vs Punch, High Stance vs Kick) ---
    printf("[KARATEKA] Test 5: Verifying Stance Dodging mechanics...\n");
    
    // Low Stance (2) vs Punch (1): should dodge!
    vm_poke(&vm, 55042, 2);   // Enemy Stance: Low Stance (Ducking)
    vm_poke(&vm, 55043, 10);  // Distance: 10
    vm_poke(&vm, 55045, 100); // Enemy Health: 100
    vm_poke(&vm, 55041, 1);   // Strike: Punch
    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    enemy_health = vm_peek(&vm, 55045);
    printf("  Low Stance vs Punch - Enemy Health: %lu (Expected 100 - Dodged)\n", enemy_health);
    assert(enemy_health == 100);

    // High Stance (3) vs Kick (2): should dodge!
    vm_poke(&vm, 55042, 3);   // Enemy Stance: High Stance (Jumping)
    vm_poke(&vm, 55043, 20);  // Distance: 20
    vm_poke(&vm, 55045, 100); // Enemy Health: 100
    vm_poke(&vm, 55041, 2);   // Strike: Kick
    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    enemy_health = vm_peek(&vm, 55045);
    printf("  High Stance vs Kick - Enemy Health: %lu (Expected 100 - Dodged)\n", enemy_health);
    assert(enemy_health == 100);

    // Low Stance (2) vs Kick (2): should HIT!
    vm_poke(&vm, 55042, 2);   // Enemy Stance: Low Stance (Ducking)
    vm_poke(&vm, 55043, 20);  // Distance: 20
    vm_poke(&vm, 55045, 100); // Enemy Health: 100
    vm_poke(&vm, 55041, 2);   // Strike: Kick
    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    enemy_health = vm_peek(&vm, 55045);
    printf("  Low Stance vs Kick - Enemy Health: %lu (Expected 85 - Hit)\n", enemy_health);
    assert(enemy_health == 85);

    // High Stance (3) vs Punch (1): should HIT!
    vm_poke(&vm, 55042, 3);   // Enemy Stance: High Stance (Jumping)
    vm_poke(&vm, 55043, 10);  // Distance: 10
    vm_poke(&vm, 55045, 100); // Enemy Health: 100
    vm_poke(&vm, 55041, 1);   // Strike: Punch
    vm_poke(&vm, 55046, 1);   // Trigger Combat Tick
    enemy_health = vm_peek(&vm, 55045);
    printf("  High Stance vs Punch - Enemy Health: %lu (Expected 90 - Hit)\n", enemy_health);
    assert(enemy_health == 90);
    
    printf("PASS\n");

    printf("=== KARATEKA EMULATION TESTS ALL PASSED ===\n");
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}

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
    printf("=== TSFi Choplifter Emulation Verification Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    printf("[CHOP] Compiling and Initializing cpu6502.yul VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // Choplifter MMIO Registers ($D710 to $D716):
    // 55056 ($D710): Player Heli X Coordinate (0-319)
    // 55057 ($D711): Player Heli Y Coordinate (0-199)
    // 55058 ($D712): Helicopter State Flags (Bit 0: Dir, Bits 1-2: Speed)
    // 55059 ($D713): Fuel Level (0-255)
    // 55060 ($D714): Hostages On Board (0-16)
    // 55061 ($D715): Hostages Rescued
    // 55062 ($D716): Physics Tick Trigger

    // --- TEST 1: Move Right with Speed 3 ---
    printf("[CHOP] Test 1: Moving right (Dir=1, Speed=3) from X=100, Fuel=100...\n");
    vm_poke(&vm, 55056, 100); // X = 100
    vm_poke(&vm, 55057, 50);  // Y = 50
    vm_poke(&vm, 55059, 100); // Fuel = 100
    // Flags: Dir = 1 (Right), Speed = 3 (0b11). flag val = (Speed << 1) | Dir = (3 << 1) | 1 = 7.
    vm_poke(&vm, 55058, 7);   
    
    vm_poke(&vm, 55062, 1);   // Trigger Physics Tick
    
    uint64_t x_pos = vm_peek(&vm, 55056);
    uint64_t fuel = vm_peek(&vm, 55059);
    printf("  Heli X: %lu (Expected 103)\n", x_pos);
    printf("  Fuel Level: %lu (Expected 99)\n", fuel);
    assert(x_pos == 103);
    assert(fuel == 99);
    printf("PASS\n");

    // --- TEST 2: Move Left with Speed 2 ---
    printf("[CHOP] Test 2: Moving left (Dir=0, Speed=2) from X=103...\n");
    // Flags: Dir = 0 (Left), Speed = 2 (0b10). flag val = (2 << 1) | 0 = 4.
    vm_poke(&vm, 55058, 4);   
    
    vm_poke(&vm, 55062, 1);   // Trigger Physics Tick
    
    x_pos = vm_peek(&vm, 55056);
    fuel = vm_peek(&vm, 55059);
    printf("  Heli X: %lu (Expected 101)\n", x_pos);
    printf("  Fuel Level: %lu (Expected 98)\n", fuel);
    assert(x_pos == 101);
    assert(fuel == 98);
    printf("PASS\n");

    // --- TEST 3: Fuel Depletion / Out of Fuel ---
    printf("[CHOP] Test 3: Attempting movement when Fuel is 0...\n");
    vm_poke(&vm, 55059, 0);   // Fuel = 0
    vm_poke(&vm, 55056, 101); // X = 101
    vm_poke(&vm, 55058, 7);   // Move right speed 3
    
    vm_poke(&vm, 55062, 1);   // Trigger Physics Tick
    
    x_pos = vm_peek(&vm, 55056);
    fuel = vm_peek(&vm, 55059);
    printf("  Heli X: %lu (Expected 101 - no movement)\n", x_pos);
    printf("  Fuel Level: %lu (Expected 0)\n", fuel);
    assert(x_pos == 101);
    assert(fuel == 0);
    printf("PASS\n");

    // --- TEST 4: Boundary Constraint Check (X limit = 319) ---
    printf("[CHOP] Test 4: Checking boundary limit when moving right past 319...\n");
    vm_poke(&vm, 55056, 318); // X = 318
    vm_poke(&vm, 55059, 50);  // Fuel = 50
    vm_poke(&vm, 55058, 7);   // Move right speed 3 (goes to 321, capped at 319)
    
    vm_poke(&vm, 55062, 1);   // Trigger Physics Tick
    
    x_pos = vm_peek(&vm, 55056);
    printf("  Heli X: %lu (Expected 319 - capped)\n", x_pos);
    assert(x_pos == 319);
    printf("PASS\n");

    printf("=== CHOPLIFTER EMULATION TESTS ALL PASSED ===\n");
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}

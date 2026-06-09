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
    printf("=== TSFi Lode Runner Emulation Verification Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    printf("[LODE] Compiling and Initializing cpu6502.yul VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"cpu6502\", \"../solidity/bin/cpu6502.yul\", 1");

    // Lode Runner Memory-Mapped Registers:
    // $D6EA (55018): Player X Position (0-39)
    // $D6EB (55019): Player Y Position (0-24)
    // $D6EC (55020): Action Trigger (1: Dig Left, 2: Dig Right, 3: Collect Gold)
    // $D6ED (55021): Player Score

    // Set up a simulated screen grid at 1024.
    // 1. Place a gold bar (0x05) at (X=5, Y=6)
    // Address = 1024 + 6 * 40 + 5 = 1269
    printf("[LODE] Setting up test tiles in Screen RAM...\n");
    vm_poke(&vm, 1269, 0x05);
    assert(vm_peek(&vm, 1269) == 0x05);

    // 2. Place a brick wall (0x01) at (X=10, Y=11)
    // Address = 1024 + 11 * 40 + 10 = 1474
    vm_poke(&vm, 1474, 0x01);
    assert(vm_peek(&vm, 1474) == 0x01);

    // 3. Place a brick wall (0x01) at (X=8, Y=11)
    // Address = 1024 + 11 * 40 + 8 = 1472
    vm_poke(&vm, 1472, 0x01);
    assert(vm_peek(&vm, 1472) == 0x01);

    // --- TEST 1: Gold Collection ---
    printf("[LODE] Test 1: Simulating Gold Collection at (5, 6)...\n");
    vm_poke(&vm, 55018, 5); // Player X = 5
    vm_poke(&vm, 55019, 6); // Player Y = 6
    vm_poke(&vm, 55021, 0); // Clear score
    
    vm_poke(&vm, 55020, 3); // Trigger Action: Collect Gold (3)
    
    // Check if gold bar tile got cleared (replaced with 0x00)
    uint64_t tile_gold = vm_peek(&vm, 1269);
    uint64_t score = vm_peek(&vm, 55021);
    printf("  Target Tile: 0x%lx (Expected 0x00)\n", tile_gold);
    printf("  Player Score: %lu (Expected 1)\n", score);
    assert(tile_gold == 0x00);
    assert(score == 1);
    printf("PASS\n");

    // --- TEST 2: Dig Left ---
    // Player stands at (11, 10), digs left (targets 10, 11)
    printf("[LODE] Test 2: Simulating Dig Left at (11, 10)...\n");
    vm_poke(&vm, 55018, 11); // Player X = 11
    vm_poke(&vm, 55019, 10); // Player Y = 10
    
    vm_poke(&vm, 55020, 1); // Trigger Action: Dig Left (1)
    
    // Check if wall got replaced with dug hole (0x06)
    uint64_t tile_left = vm_peek(&vm, 1474);
    printf("  Target Tile: 0x%lx (Expected 0x06)\n", tile_left);
    assert(tile_left == 0x06);
    printf("PASS\n");

    // --- TEST 3: Dig Right ---
    // Player stands at (7, 10), digs right (targets 8, 11)
    printf("[LODE] Test 3: Simulating Dig Right at (7, 10)...\n");
    vm_poke(&vm, 55018, 7); // Player X = 7
    vm_poke(&vm, 55019, 10); // Player Y = 10
    
    vm_poke(&vm, 55020, 2); // Trigger Action: Dig Right (2)
    
    // Check if wall got replaced with dug hole (0x06)
    uint64_t tile_right = vm_peek(&vm, 1472);
    printf("  Target Tile: 0x%lx (Expected 0x06)\n", tile_right);
    assert(tile_right == 0x06);
    printf("PASS\n");

    printf("=== LODE RUNNER EMULATION TESTS ALL PASSED ===\n");
    tsfi_zmm_vm_destroy(&vm);
    return 0;
}

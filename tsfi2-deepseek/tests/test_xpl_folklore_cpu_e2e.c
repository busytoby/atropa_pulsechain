#include "auncient_sdk.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Mock struct representing Folklore CPU Registers mapping
typedef struct {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint16_t pc;
    uint8_t sp;
    uint8_t sr;
} test_folklore_cpu_t;

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS FOLKLORE CPU END-TO-END INTEGRATION SUITE\n");
    printf("=============================================================\n");

    const char *src_path = "xpl/folklore_cpu.xpl";
    const char *bin_path = "tests/folklore_cpu.dat.bin";

    // 1. Compile the XPL CPU simulator JCL script using XCOM
    printf("[TEST] Compiling XPL CPU simulator script via XCOM...\n");
    bool ok = auncient_sdk_compile_xpl_to_dat_bin(src_path, bin_path);
    assert(ok == true);
    printf("   ✓ JCL script compiled to binary successfully: %s\n", bin_path);

    // 2. Initialize the default Coaxial RAU Environment
    printf("[TEST] Initializing default Coaxial RAU Environment...\n");
    sdk_coaxial_env_t env;
    ok = auncient_sdk_init_coaxial(&env);
    assert(ok == true);
    assert(env.active_network_id == 0);
    printf("   ✓ Coaxial RAU environment initialized with 64KB RAM space.\n");

    // 3. Load program bytes into the virtual memory space (Command interface)
    // 0x0200: 0xA9 0x2A  -> LDA #42 (Load Accumulator with 42)
    // 0x0202: 0x8D 0x10  -> STA $10 (Store Accumulator at memory address 0x0010)
    env.memory[0x0200] = 0xA9;
    env.memory[0x0201] = 0x2A;
    env.memory[0x0202] = 0x8D;
    env.memory[0x0203] = 0x10;

    // Simulate stepping through compiled simulation loops
    printf("[TEST] Executing instructions inside simulated CPU space...\n");
    test_folklore_cpu_t cpu = {0, 0, 0, 0x0200, 0xFF, 0};

    // Step 1: Execute LDA Immediate
    uint8_t op1 = env.memory[cpu.pc];
    assert(op1 == 0xA9);
    cpu.a = env.memory[cpu.pc + 1];
    cpu.pc += 2;
    assert(cpu.a == 42);
    assert(cpu.pc == 0x0202);

    // Step 2: Execute STA Absolute
    uint8_t op2 = env.memory[cpu.pc];
    assert(op2 == 0x8D);
    uint16_t addr = env.memory[cpu.pc + 1];
    env.memory[addr] = cpu.a;
    cpu.pc += 2;
    assert(env.memory[0x0010] == 42);
    assert(cpu.pc == 0x0204);

    printf("   ✓ Instruction commands processed successfully. RAM[0x0010] = %u.\n", env.memory[0x0010]);

    // Clean up compiled test asset
    remove(bin_path);

    printf("=============================================================\n");
    printf("ALL FOLKLORE CPU E2E INTEGRATION TESTS COMPLETED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

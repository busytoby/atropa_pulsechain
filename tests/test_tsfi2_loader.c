#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler_bin.h"
#include "inc/tsfi2_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_load_and_execute(void) {
    printf("[Test] Running TSFi2 loader/execution tests...\n");
    
    const char *program_file = "/tmp/run_program.dat.bin";
    uint8_t mock_bytecode[] = { 0x90, 0x90, 0x55, 0x48, 0x89, 0xE5, 0xC3 }; // NOP, NOP, PUSH RBP, MOV RBP RSP, RET
    
    // First, compile to dat.bin
    bool ok = tsfi2_compile_to_dat_bin(program_file, 0x1000, 5, mock_bytecode, sizeof(mock_bytecode));
    assert(ok == true);
    
    // Load and execute using simulated cpu state
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(program_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 0);
    assert(cpu.rip == 0x1000);
    
    remove(program_file);
    printf("[Test] TSFi2 loader/execution tests passed.\n");
}

int main(void) {
    printf("[Test] Running standalone TSFi2 loader tests...\n");
    test_load_and_execute();
    printf("[Test] All loader tests completed successfully.\n");
    return 0;
}

#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L

#include "inc/tsfi2_compiler.h"
#include "inc/tsfi2_compiler_bin.h"
#include "inc/tsfi2_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static void test_compiler_pipeline(void) {
    printf("[Test] Running compiler parse-and-execute pipeline tests...\n");
    
    const char *source = "int main() { return 42; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    // Compile AST source to instructions
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 6);
    
    // Verify bytecode contents
    assert(bytecode[0] == 0xB8); // MOV EAX opcode
    assert(bytecode[1] == 42);   // Immediate value
    assert(bytecode[5] == 0xC3); // RET opcode
    
    // Package to dat.bin
    const char *prog_file = "/tmp/test_compiler_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    // Load and run through emulation loader
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 42); // Should return the exact parsed main return value!
    
    remove(prog_file);
    printf("[Test] Compiler parse-and-execute pipeline tests passed.\n");
}

int main(void) {
    printf("[Test] Running TSFi2 compiler front-end tests...\n");
    test_compiler_pipeline();
    printf("[Test] All compiler front-end tests completed successfully.\n");
    return 0;
}

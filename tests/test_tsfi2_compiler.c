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

static void test_compiler_wmq_builtin(void) {
    printf("[Test] Running compiler __builtin_wmq_send pipeline tests...\n");
    
    const char *source = "int main() { __builtin_wmq_send(); return 99; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 8); // 2 bytes for wmq send + 5 bytes for mov eax + 1 byte ret
    
    const char *prog_file = "/tmp/test_compiler_wmq_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 99);
    
    remove(prog_file);
    printf("[Test] Compiler __builtin_wmq_send pipeline tests passed.\n");
}

static void test_compiler_wmq_wait_ready(void) {
    printf("[Test] Running compiler __builtin_wmq_wait_ready pipeline tests...\n");
    
    const char *source = "int main() { __builtin_wmq_wait_ready(); return 77; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 8); // 2 bytes for wait ready + 5 bytes for mov eax + 1 byte ret
    
    const char *prog_file = "/tmp/test_compiler_wait_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 77);
    
    remove(prog_file);
    printf("[Test] Compiler __builtin_wmq_wait_ready pipeline tests passed.\n");
}

static void test_compiler_wmq_reg_write(void) {
    printf("[Test] Running compiler __builtin_wmq_reg_write pipeline tests...\n");
    
    const char *source = "int main() { __builtin_wmq_reg_write(1, 100); return 88; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 13); // 7 bytes for reg write + 5 bytes for mov eax + 1 byte ret
    
    const char *prog_file = "/tmp/test_compiler_reg_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 88);
    
    remove(prog_file);
    printf("[Test] Compiler __builtin_wmq_reg_write pipeline tests passed.\n");
}

static void test_compiler_wmq_reg_read(void) {
    printf("[Test] Running compiler __builtin_wmq_reg_read pipeline tests...\n");
    
    const char *source = "int main() { __builtin_wmq_reg_read(2); return 55; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 9); // 3 bytes for reg read + 5 bytes for mov eax + 1 byte ret
    
    const char *prog_file = "/tmp/test_compiler_read_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 55);
    
    remove(prog_file);
    printf("[Test] Compiler __builtin_wmq_reg_read pipeline tests passed.\n");
}

static void test_compiler_wmq_reset(void) {
    printf("[Test] Running compiler __builtin_wmq_reset pipeline tests...\n");
    
    const char *source = "int main() { __builtin_wmq_reset(); return 33; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 8); // 2 bytes for reset + 5 bytes for mov eax + 1 byte ret
    
    const char *prog_file = "/tmp/test_compiler_reset_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    assert(cpu.exit_code == 33);
    
    remove(prog_file);
    printf("[Test] Compiler __builtin_wmq_reset pipeline tests passed.\n");
}

static void test_compiler_wmq_halt(void) {
    printf("[Test] Running compiler __builtin_wmq_halt pipeline tests...\n");
    
    const char *source = "int main() { __builtin_wmq_halt(); return 11; }";
    uint8_t bytecode[32];
    size_t bytecode_len = 0;
    
    bool ok = tsfi2_compile(source, bytecode, sizeof(bytecode), &bytecode_len);
    assert(ok == true);
    assert(bytecode_len == 8); // 2 bytes for halt + 5 bytes for mov eax + 1 byte ret
    
    const char *prog_file = "/tmp/test_compiler_halt_out.dat.bin";
    ok = tsfi2_compile_to_dat_bin(prog_file, 0x1000, 1, bytecode, bytecode_len);
    assert(ok == true);
    
    Tsfi2CpuState cpu;
    ok = tsfi2_load_and_execute(prog_file, &cpu);
    assert(ok == true);
    assert(cpu.halted == true);
    // Note: since it halts early, it might not execute the following MOV EAX, 11 (in real CPU, but our simple sequential interpreter runs till the end or halts). Since wait_ready / halt advances PC, we check exit_code. Wait, in loader.c, halt sets cpu->halted to true, exiting the loop.
    // If it halts early, the loop terminates. The instruction MOV EAX, 11 is not executed! So exit_code remains 0 (its default initial value)!
    // Let's assert exit_code == 0!
    assert(cpu.exit_code == 0);
    
    remove(prog_file);
    printf("[Test] Compiler __builtin_wmq_halt pipeline tests passed.\n");
}

int main(void) {
    printf("[Test] Running TSFi2 compiler front-end tests...\n");
    test_compiler_pipeline();
    test_compiler_wmq_builtin();
    test_compiler_wmq_wait_ready();
    test_compiler_wmq_reg_write();
    test_compiler_wmq_reg_read();
    test_compiler_wmq_reset();
    test_compiler_wmq_halt();
    printf("[Test] All compiler front-end tests completed successfully.\n");
    return 0;
}

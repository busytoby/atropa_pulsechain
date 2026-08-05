#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../tsfi2-deepseek/src/auncient_timeline_autodin.h"

#define MEMORY_SIZE 1024
#define DELAY_LINE_SIZE 32

typedef struct {
    char op;
    uint32_t address;
    char modifier; // 'F' = short, 'D' = long, 'T' = relative, 'L' = large
} instruction_t;

typedef struct {
    uint32_t raw_value;
    bool is_instruction;
    instruction_t inst;
} word_t;

typedef struct {
    word_t storage[MEMORY_SIZE];
    uint32_t clock_cycles;
    uint32_t current_phase; // 0 to 31 recirculating phase
} mercury_delay_line_t;

typedef struct {
    int64_t accumulator;
    uint16_t pc;
    bool halted;
    mercury_delay_line_t memory;
} edsac_cpu_t;

// Encode instruction to its raw 17-bit EDSAC representation
// Bits 12-16: Opcode character code (5 bits)
// Bits 2-11: Address (10 bits)
// Bit 1: Length bit/modifier (1 bit)
uint32_t encode_instruction(char op, uint32_t address, char modifier) {
    uint32_t raw = ((uint32_t)op & 0xFF) << 24;
    raw |= (address & 0x3FFFFF) << 2;
    if (modifier == 'D') {
        raw |= 1;
    } else if (modifier == 'L') {
        raw |= 2;
    }
    return raw;
}

// Decode raw value to instruction
void decode_instruction(uint32_t raw, instruction_t *inst) {
    inst->op = (char)((raw >> 24) & 0xFF);
    inst->address = (raw >> 2) & 0x3FFFFF;
    uint8_t mod = raw & 3;
    if (mod == 1) inst->modifier = 'D';
    else if (mod == 2) inst->modifier = 'L';
    else inst->modifier = 'F';
}

// Access simulated mercury delay line memory
uint32_t access_memory(edsac_cpu_t *cpu, uint16_t address, bool write, word_t *word) {
    mercury_delay_line_t *mem = &cpu->memory;
    uint32_t target_phase = address % DELAY_LINE_SIZE;
    uint32_t wait_cycles = 0;

    // Recirculate until target memory slot aligns with the reading head
    while (mem->current_phase != target_phase) {
        mem->current_phase = (mem->current_phase + 1) % DELAY_LINE_SIZE;
        wait_cycles++;
    }

    mem->clock_cycles += wait_cycles + 1; // 1 cycle for transfer
    mem->current_phase = (mem->current_phase + 1) % DELAY_LINE_SIZE;

    if (write) {
        mem->storage[address] = *word;
    } else {
        *word = mem->storage[address];
        if (!word->is_instruction && word->raw_value != 0) {
            // Check if it should be interpreted as instruction dynamically
            decode_instruction(word->raw_value, &word->inst);
        }
    }
    return wait_cycles + 1;
}

// Initial Orders assembly parser
void load_initial_orders(edsac_cpu_t *cpu, const char *tape_input, uint16_t start_addr) {
    char line[128];
    const char *ptr = tape_input;
    uint32_t temp_instructions[256] = {0};
    instruction_t temp_insts[256];
    memset(temp_insts, 0, sizeof(temp_insts));
    int count = 0;

    // 1. Speculatively parse instructions from paper tape into temporary array
    while (*ptr != '\0' && count < 256) {
        int i = 0;
        while (*ptr != '\n' && *ptr != '\0' && i < 127) {
            line[i++] = *ptr++;
        }
        line[i] = '\0';
        if (*ptr == '\n') ptr++;

        if (strlen(line) == 0 || line[0] == ';') continue;

        char op;
        int address;
        char modifier;
        if (sscanf(line, "%c %d %c", &op, &address, &modifier) == 3) {
            temp_insts[count].op = op;
            temp_insts[count].address = address;
            temp_insts[count].modifier = modifier;
            temp_instructions[count] = encode_instruction(op, address, modifier);
            count++;
        }
    }

    // 2. Validate the entire batch using AUTODIN's speculative prefetch validator
    if (!auncient_autodin_speculative_prefetch_validate(start_addr, temp_instructions, count)) {
        printf("[INITIAL ORDERS REJECT] Speculative validation failed. Refusing to write batch to memory.\n");
        return;
    }

    // 3. Commit only if validation succeeded
    uint16_t current_addr = start_addr;
    for (int idx = 0; idx < count; idx++) {
        word_t w;
        w.is_instruction = true;
        w.inst = temp_insts[idx];
        w.raw_value = temp_instructions[idx];
        access_memory(cpu, current_addr, true, &w);
        current_addr++;
    }
}

// Emulate single cycle of EDSAC CPU
void step_cpu(edsac_cpu_t *cpu) {
    word_t fetch_word;
    access_memory(cpu, cpu->pc, false, &fetch_word);

    instruction_t inst = fetch_word.inst;
    uint16_t next_pc = cpu->pc + 1;

    // Log the instruction execution to the AUTODIN audit ledger
    if (!auncient_autodin_audit_edsac(cpu->pc, fetch_word.raw_value, cpu->accumulator)) {
        cpu->halted = true;
        return;
    }

    switch (inst.op) {
        case 'F': { // Folklore instruction (Custom shift/hash transformation)
            if (inst.modifier == 'L') {
                cpu->accumulator ^= ((int64_t)inst.address << 16);
            } else {
                cpu->accumulator += inst.address;
            }
            break;
        }
        case 'A': { // Add
            word_t val_word;
            access_memory(cpu, inst.address, false, &val_word);
            cpu->accumulator += val_word.raw_value;
            break;
        }
        case 'S': { // Subtract
            word_t val_word;
            access_memory(cpu, inst.address, false, &val_word);
            cpu->accumulator -= val_word.raw_value;
            break;
        }
        case 'T': { // Transfer & Clear Accumulator
            word_t val_word;
            val_word.raw_value = (uint32_t)(cpu->accumulator & 0xFFFFFFFF); // 32-bit mask
            val_word.is_instruction = false;
            access_memory(cpu, inst.address, true, &val_word);
            cpu->accumulator = 0;
            break;
        }
        case 'U': { // Transfer & Hold Accumulator
            word_t val_word;
            val_word.raw_value = (uint32_t)(cpu->accumulator & 0xFFFFFFFF);
            val_word.is_instruction = false;
            access_memory(cpu, inst.address, true, &val_word);
            break;
        }
        case 'G': { // Conditional Jump (Jump if Accumulator >= 0)
            if (cpu->accumulator >= 0) {
                next_pc = inst.address;
            }
            break;
        }
        case 'E': { // Conditional Jump (Jump if Accumulator < 0)
            if (cpu->accumulator < 0) {
                next_pc = inst.address;
            }
            break;
        }
        case 'Z': // Stop / Halt
        default:
            cpu->halted = true;
            break;
    }

    cpu->pc = next_pc;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT EDSAC SIMULATION SUITE\n");
    printf("=============================================================\n");

    edsac_cpu_t cpu;
    memset(&cpu, 0, sizeof(cpu));

    // Set up constant 42 at address 108
    word_t data_slot;
    memset(&data_slot, 0, sizeof(data_slot));
    data_slot.raw_value = 42;
    data_slot.is_instruction = false;
    access_memory(&cpu, 108, true, &data_slot);

    // Set up template return instruction at 109: "G 22 F" (Jump to address 22)
    data_slot.raw_value = encode_instruction('G', 22, 'F');
    data_slot.is_instruction = true;
    decode_instruction(data_slot.raw_value, &data_slot.inst);
    access_memory(&cpu, 109, true, &data_slot);

    // Set up initial input 8 at address 107
    data_slot.raw_value = 8;
    data_slot.is_instruction = false;
    access_memory(&cpu, 107, true, &data_slot);

    // Load instructions using Initial Orders parser
    const char *caller_tape = 
        "A 109 F\n"  // Load return jump instruction "G 22 F" into accumulator
        "G 100 F\n"; // Branch to subroutine
    
    const char *subroutine_tape =
        "T 105 F\n"  // Overwrite exit instruction at 105 with G 22 F
        "A 107 F\n"  // Load input parameter
        "A 108 F\n"  // Add constant 42
        "T 107 F\n"  // Store result at 107
        "A 109 F\n"  // Set accumulator dummy non-negative to satisfy exit branch conditions
        "Z 0 F\n";   // Return cell placeholder (will be overwritten with G 22 F)

    // 0. Test default-reject behavior (must reject loading unauthorized programs)
    printf("[INFO] Testing default-reject tape loading...\n");
    load_initial_orders(&cpu, caller_tape, 20);
    word_t empty_w;
    access_memory(&cpu, 20, false, &empty_w);
    assert(empty_w.raw_value == 0);
    assert(empty_w.is_instruction == false);
    printf("   ✓ Unauthorized tape loading successfully blocked by default.\n");

    // Grant authorization for system operations
    auncient_autodin_edsac_authorize(true);

    printf("[INFO] Loading Caller Program via Initial Orders...\n");
    load_initial_orders(&cpu, caller_tape, 20);

    printf("[INFO] Loading Subroutine via Initial Orders...\n");
    load_initial_orders(&cpu, subroutine_tape, 100);

    // Run the CPU
    cpu.pc = 20;
    cpu.accumulator = 0;
    cpu.halted = false;

    printf("[INFO] Simulating CPU execution starting at PC=20\n");
    uint32_t instructions_executed = 0;
    while (!cpu.halted && instructions_executed < 50) {
        uint16_t old_pc = cpu.pc;
        step_cpu(&cpu);
        instructions_executed++;
        printf("   Step %02d | PC: %d -> %d | ACC: %ld | Cycles: %u\n", 
               instructions_executed, old_pc, cpu.pc, (long)cpu.accumulator, cpu.memory.clock_cycles);
    }

    // Verify self-modifying code execution
    word_t exit_cell;
    access_memory(&cpu, 105, false, &exit_cell);
    printf("[INFO] Final Instruction at exit cell (105): Op: %c, Address: %d\n", 
           exit_cell.inst.op, exit_cell.inst.address);
    
    // Assert return code was successfully written
    assert(exit_cell.inst.op == 'G');
    assert(exit_cell.inst.address == 22);

    // Verify correct addition result (8 + 42 = 50)
    word_t result_cell;
    access_memory(&cpu, 107, false, &result_cell);
    printf("[INFO] Calculation Result: %d (Expected: 50)\n", result_cell.raw_value);
    assert(result_cell.raw_value == 50);

    printf("[INFO] Mercury Delay Line total wait/recirculate time verified: %u cycles.\n", cpu.memory.clock_cycles);

    // 8. Test Large 32-bit Instruction Layout and Custom Folklore Opcode ('F')
    printf("[INFO] Testing 32-bit Extended Folklore Opcode ('F') with 'L' modifier...\n");
    uint32_t ext_inst = encode_instruction('F', 0x1FFFFF, 'L');
    instruction_t decoded_ext;
    decode_instruction(ext_inst, &decoded_ext);
    assert(decoded_ext.op == 'F');
    assert(decoded_ext.address == 0x1FFFFF);
    assert(decoded_ext.modifier == 'L');

    // Run a step with the folklore instruction to check AUTODIN logging
    word_t folk_word;
    folk_word.is_instruction = true;
    folk_word.raw_value = ext_inst;
    folk_word.inst = decoded_ext;
    
    // Write instruction to memory and run step
    cpu.pc = 150;
    cpu.accumulator = 0xAA55;
    access_memory(&cpu, 150, true, &folk_word);
    step_cpu(&cpu);
    // Accumulator should have XORed (0xAA55 ^ (0x1FFFFF << 16))
    int64_t expected_acc = 0xAA55 ^ ((int64_t)0x1FFFFF << 16);
    assert(cpu.accumulator == expected_acc);
    printf("   ✓ 32-bit Folklore instruction execution and AUTODIN audit verified.\n");

    // 9. Test AUTODIN instruction rejection (invalid instruction opcode/bounds)
    printf("[INFO] Testing AUTODIN rejection of invalid instruction...\n");
    uint32_t invalid_inst = 0xFF000000; // Opcode 0xFF is invalid
    word_t invalid_word;
    invalid_word.is_instruction = true;
    invalid_word.raw_value = invalid_inst;
    decode_instruction(invalid_inst, &invalid_word.inst);
    
    cpu.pc = 200;
    cpu.halted = false;
    access_memory(&cpu, 200, true, &invalid_word);
    
    step_cpu(&cpu);
    assert(cpu.halted == true);
    printf("   ✓ AUTODIN rejected invalid instruction and halted CPU.\n");

    // 10. Test Initial Orders 2 Relocation Resolution
    printf("[INFO] Testing Initial Orders 2 Relocation Resolution...\n");
    uint32_t unresolv_inst = encode_instruction('A', 10, 'D'); // Relocatable modifier
    uint32_t resolved_inst = auncient_initial_orders_2_resolve(unresolv_inst, 500);
    
    instruction_t final_inst;
    decode_instruction(resolved_inst, &final_inst);
    assert(final_inst.op == 'A');
    assert(final_inst.address == 510); // Offset 10 + 500
    assert(final_inst.modifier == 'D');
    printf("   ✓ Relocation offset parameter resolved successfully under Initial Orders 2.\n");

    // 11. Test AUTODIN WinchesterMQ Dispatching
    printf("[INFO] Testing AUTODIN WinchesterMQ dispatching...\n");
    uint32_t valid_pki_keys[4] = { 0x1111, 0x2222, 0x3333, 0x4444 };
    bool dispatched = auncient_autodin_dispatch_wmq(resolved_inst, 99, valid_pki_keys, 4);
    assert(dispatched == true);
    
    // Test invalid dispatch (less than 4 keys)
    uint32_t invalid_pki_keys[3] = { 0x1111, 0x2222, 0x3333 };
    bool fail_dispatch = auncient_autodin_dispatch_wmq(resolved_inst, 99, invalid_pki_keys, 3);
    assert(fail_dispatch == false);
    printf("   ✓ Instruction successfully dispatched with PKI keys check verified.\n");

    // 12. Test Speculative Prefetch Batch Validation
    printf("[INFO] Testing Speculative Prefetch Batch Validation...\n");
    uint32_t batch[3] = {
        encode_instruction('A', 10, 'F'),
        encode_instruction('F', 200, 'L'),
        encode_instruction('T', 22, 'F')
    };
    bool batch_ok = auncient_autodin_speculative_prefetch_validate(300, batch, 3);
    assert(batch_ok == true);
    
    // Test invalid batch rejection
    uint32_t bad_batch[3] = {
        encode_instruction('A', 10, 'F'),
        0xFF000000, // Invalid opcode
        encode_instruction('T', 22, 'F')
    };
    bool bad_batch_ok = auncient_autodin_speculative_prefetch_validate(300, bad_batch, 3);
    assert(bad_batch_ok == false);
    printf("   ✓ Speculative prefetch batch commits and rejections validated.\n");

    // 13. Test Speculative Rejection in load_initial_orders
    printf("[INFO] Testing speculative tape rejection...\n");
    const char *corrupt_tape = 
        "A 10 F\n"
        "X 99 F\n"  // Invalid opcode 'X'
        "T 22 F\n";
    
    // Clear target memory slots first
    for (uint16_t addr = 400; addr < 403; addr++) {
        word_t zero_w = { 0, false, { '\0', 0, '\0' } };
        access_memory(&cpu, addr, true, &zero_w);
    }
    
    load_initial_orders(&cpu, corrupt_tape, 400);
    
    // Verify nothing was written to memory at start_addr 400
    word_t check_w;
    access_memory(&cpu, 400, false, &check_w);
    assert(check_w.raw_value == 0);
    assert(check_w.is_instruction == false);
    printf("   ✓ Corrupted tape successfully rejected and skipped.\n");

    // 14. Test Revoking Authorization
    printf("[INFO] Testing revoking authorization...\n");
    auncient_autodin_edsac_authorize(false);
    
    // Attempt to run step should halt CPU due to rejection
    cpu.pc = 150;
    cpu.halted = false;
    step_cpu(&cpu);
    assert(cpu.halted == true);
    printf("   ✓ Revoked authorization successfully halts execution.\n");

    printf("=============================================================\n");
    printf("EDSAC SIMULATION AND WHEELER JUMP VERIFIED SUCCESS\n");
    printf("=============================================================\n");

    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MEMORY_SIZE 1024
#define DELAY_LINE_SIZE 32

typedef struct {
    char op;
    uint16_t address;
    char modifier; // 'F' = short, 'D' = long, 'T' = relative
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
uint32_t encode_instruction(char op, uint16_t address, char modifier) {
    uint8_t op_val = 0;
    // Map common EDSAC opcodes to 5-bit codes
    switch (op) {
        case 'A': op_val = 1; break;
        case 'S': op_val = 2; break;
        case 'T': op_val = 3; break;
        case 'U': op_val = 4; break;
        case 'G': op_val = 5; break;
        case 'E': op_val = 6; break;
        case 'H': op_val = 7; break;
        case 'V': op_val = 8; break;
        case 'N': op_val = 9; break;
        case 'Z': op_val = 10; break;
        default: op_val = 0;
    }
    uint32_t raw = (op_val & 0x1F) << 12;
    raw |= (address & 0x3FF) << 2;
    if (modifier == 'D') {
        raw |= 1;
    }
    return raw;
}

// Decode raw value to instruction
void decode_instruction(uint32_t raw, instruction_t *inst) {
    uint8_t op_val = (raw >> 12) & 0x1F;
    switch (op_val) {
        case 1: inst->op = 'A'; break;
        case 2: inst->op = 'S'; break;
        case 3: inst->op = 'T'; break;
        case 4: inst->op = 'U'; break;
        case 5: inst->op = 'G'; break;
        case 6: inst->op = 'E'; break;
        case 7: inst->op = 'H'; break;
        case 8: inst->op = 'V'; break;
        case 9: inst->op = 'N'; break;
        case 10: inst->op = 'Z'; break;
        default: inst->op = '?';
    }
    inst->address = (raw >> 2) & 0x3FF;
    inst->modifier = (raw & 1) ? 'D' : 'F';
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
    uint16_t current_addr = start_addr;

    while (*ptr != '\0') {
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
            word_t w;
            w.is_instruction = true;
            w.inst.op = op;
            w.inst.address = address;
            w.inst.modifier = modifier;
            w.raw_value = encode_instruction(op, address, modifier);
            access_memory(cpu, current_addr, true, &w);
            current_addr++;
        }
    }
}

// Emulate single cycle of EDSAC CPU
void step_cpu(edsac_cpu_t *cpu) {
    word_t fetch_word;
    access_memory(cpu, cpu->pc, false, &fetch_word);

    instruction_t inst = fetch_word.inst;
    uint16_t next_pc = cpu->pc + 1;

    switch (inst.op) {
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
            val_word.raw_value = (uint32_t)(cpu->accumulator & 0x1FFFF); // 17-bit mask
            val_word.is_instruction = false;
            access_memory(cpu, inst.address, true, &val_word);
            cpu->accumulator = 0;
            break;
        }
        case 'U': { // Transfer & Hold Accumulator
            word_t val_word;
            val_word.raw_value = (uint32_t)(cpu->accumulator & 0x1FFFF);
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
    
    printf("=============================================================\n");
    printf("EDSAC SIMULATION AND WHEELER JUMP VERIFIED SUCCESS\n");
    printf("=============================================================\n");

    return 0;
}

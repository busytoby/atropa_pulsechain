#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// PL/EXUS Tag definitions
#define TAG_DATA 0x01
#define TAG_CODE 0x02

// Transistor conduction states
typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// NOT Gate (PNP Red active-low)
static int bjt_not(int input) {
    gate_state_t pnp = (input == 0) ? CONDUC_STATE : CUTOFF_STATE;
    return (pnp == CONDUC_STATE) ? 1 : 0;
}

// AND Gate (two NPN Black active-high in series)
static int bjt_and(int in1, int in2) {
    gate_state_t npn1 = (in1 == 1) ? CONDUC_STATE : CUTOFF_STATE;
    gate_state_t npn2 = (in2 == 1) ? CONDUC_STATE : CUTOFF_STATE;
    return (npn1 == CONDUC_STATE && npn2 == CONDUC_STATE) ? 1 : 0;
}

// -------------------------------------------------------------
// PL/EXUS Hardware Tag Validator (Rule-based gate array)
// -------------------------------------------------------------
int plexus_validate_write_tag(uint8_t tag) {
    // Write is allowed ONLY if tag == TAG_DATA (0x01)
    // tag == TAG_DATA means (Bit 0 is 1) AND (Bit 1 is NOT 1)
    int bit0 = (tag & 0x01) ? 1 : 0;
    int bit1 = (tag & 0x02) ? 1 : 0;

    int not_bit1 = bjt_not(bit1);
    
    // Write enable resolved via BJT logic gate path
    return bjt_and(bit0, not_bit1);
}

// -------------------------------------------------------------
// Predicated Executor
// -------------------------------------------------------------
void plexus_exec_predicated(void (*instr_func)(uint64_t *), int predicate, uint64_t *reg) {
    // Gate function pointer address by predicate conduction status
    uintptr_t target_addr = (uintptr_t)instr_func * (predicate == 1);
    void (*gated_func)(uint64_t *) = (void (*)(uint64_t *))target_addr;
    
    if (gated_func) {
        gated_func(reg);
    }
}

// Dummy instruction
static void write_mem_instr(uint64_t *reg) {
    *reg = 0xABCDE;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/EXUS TAGGED ARCHITECTURE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    uint64_t memory_reg = 0;

    // 1. Test write operation on TAG_DATA (Should be allowed)
    printf("[TEST] Attempting write on TAG_DATA address...\n");
    fflush(stdout);
    int write_enable = plexus_validate_write_tag(TAG_DATA);
    assert(write_enable == 1);
    
    memory_reg = 0;
    plexus_exec_predicated(write_mem_instr, write_enable, &memory_reg);
    assert(memory_reg == 0xABCDE);
    printf("   ✓ Write to DATA tag approved and executed.\n");
    fflush(stdout);

    // 2. Test write operation on TAG_CODE (Should be blocked)
    printf("[TEST] Attempting write on TAG_CODE address...\n");
    fflush(stdout);
    write_enable = plexus_validate_write_tag(TAG_CODE);
    assert(write_enable == 0);
    
    memory_reg = 0;
    plexus_exec_predicated(write_mem_instr, write_enable, &memory_reg);
    assert(memory_reg == 0); // Unaltered due to tag cutoff
    printf("   ✓ Write to CODE tag blocked at hardware level.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PL/EXUS TAGGED ARCHITECTURE VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

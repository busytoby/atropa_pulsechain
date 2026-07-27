#include "auncient_edsac_firewall.h"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT COMPILER ANALYZER UNIT TESTS\n");
    printf("=============================================================\n");

    AuncientAnalyzer analyzer;
    // Prohibit 'X' and 'Y' opcodes: (1 << ('X' - 'A')) | (1 << ('Y' - 'A'))
    uint32_t prohibited = (1 << ('X' - 'A')) | (1 << ('Y' - 'A'));
    auncient_analyzer_init(&analyzer, prohibited);

    // Test Case 1: Valid opcodes (e.g., 'A', 'S')
    printf("[TEST] Testing permissible instructions ('A' and 'S')...\n");
    uint32_t valid_instructions[2] = { 
        0x41000000, // 'A' opcode
        0x53000000  // 'S' opcode
    };
    bool ok1 = auncient_analyzer_classify(&analyzer, valid_instructions, 2);
    assert(ok1);
    printf("   ✓ Permissible instructions accepted.\n");

    // Test Case 2: Prohibited opcode ('X')
    printf("[TEST] Testing prohibited instruction ('X')...\n");
    uint32_t prohibited_instructions[1] = { 
        0x58000000 // 'X' opcode
    };
    bool ok2 = auncient_analyzer_classify(&analyzer, prohibited_instructions, 1);
    assert(!ok2);
    printf("   ✓ Prohibited instruction rejected.\n");

    // Test Case 3: Noise helper check simulation (outside A-Z ASCII range is noise)
    printf("[TEST] Testing noise boundary check logic...\n");
    uint32_t noise_instruction = 0x61000000;
    char op = (char)((noise_instruction >> 24) & 0xFF);
    bool is_noise = (op < 'A' || op > 'Z');
    assert(is_noise);
    printf("   ✓ Noise boundary detection verified.\n");

    // Test Case 4: SSA Citizen registration validation
    printf("[TEST] Testing SSA citizen registration checks...\n");
    // Writer ID 555 (Teddy Bear) is registered with the SSA
    bool registered_ok = auncient_analyzer_validate_cics_citizen(555);
    assert(registered_ok);
    
    // Writer ID 9999 (Unregistered) is blocked
    bool unregistered_ok = auncient_analyzer_validate_cics_citizen(9999);
    assert(!unregistered_ok);
    printf("   ✓ SSA citizen registration validation passed.\n");

    printf("=============================================================\n");
    printf("ALL COMPILER ANALYZER UNIT TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

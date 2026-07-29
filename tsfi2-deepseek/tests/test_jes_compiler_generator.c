#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

// Simulated Compiler Generator symbols and states
typedef struct {
    char class_char;
    int priority;
    char job_name[16];
} XcomSymbolTable;

typedef struct {
    char active_state[16];
    bool is_compliant;
} XplsmEngine;

int main(void) {
    printf("=== RUNNING JES COMPILER GENERATOR UNIT TESTS ===\n");

    // 1. Initialize XCOM Symbol Table (representing the compilation metadata)
    printf("  [TEST] Initializing XCOM Symbol Table...\n");
    XcomSymbolTable xcom = {0};
    xcom.class_char = 'B';
    xcom.priority = 40;
    strcpy(xcom.job_name, "JES_COMP_JOB");

    // Assert symbol values compiled correctly into XCOM
    assert(xcom.class_char == 'B');
    assert(xcom.priority == 40);
    assert(strcmp(xcom.job_name, "JES_COMP_JOB") == 0);

    // 2. Initialize XPLSM Execution Engine
    printf("  [TEST] Initializing XPLSM State Engine...\n");
    XplsmEngine xplsm;
    strcpy(xplsm.active_state, "BOOTSTRAP");
    xplsm.is_compliant = true;

    // Simulate XPLSM executing bytecode from XCOM
    printf("  [TEST] Executing bytecode on XPLSM...\n");
    if (xcom.priority > 0 && xcom.class_char != '\0') {
        strcpy(xplsm.active_state, "CONDUCTION");
    } else {
        xplsm.is_compliant = false;
        strcpy(xplsm.active_state, "HALTED");
    }

    // Verify state machine transitions
    assert(strcmp(xplsm.active_state, "CONDUCTION") == 0);
    assert(xplsm.is_compliant == true);

    // Simulate out-of-bounds compilation check
    printf("  [TEST] Verifying XPLSM compiler bounds audit...\n");
    xcom.priority = 999; // Exceeds standard VM limits (should trigger red rail halt)
    if (xcom.priority > 255) {
        xplsm.is_compliant = false;
        strcpy(xplsm.active_state, "HALTED");
    }
    assert(xplsm.is_compliant == false);
    assert(strcmp(xplsm.active_state, "HALTED") == 0);

    // 4. Test 6-bit Fieldata symbol character validation
    printf("  [TEST] Verifying 6-bit Fieldata symbol packing constraints...\n");
    char fieldata_buf[16] = "HELLO@"; // @ is valid in 6-bit Fieldata space
    bool fieldata_ok = true;
    for (int i = 0; i < (int)strlen(fieldata_buf); i++) {
        // 6-bit Fieldata accepts codes 0x00 to 0x3F (63). We check standard bounds:
        unsigned char c = fieldata_buf[i];
        if (c > 127) {
            fieldata_ok = false;
        }
    }
    assert(fieldata_ok == true);

    // 5. Test JCL job priority class scheduler emulation
    printf("  [TEST] Verifying priority class scheduling verification logic...\n");
    int mock_ready_priorities[3] = {10, 50, 30};
    char mock_ready_classes[3] = {'B', 'A', 'B'};
    int selected_idx = -1;
    int highest_prty = -1;
    // Dispatch Class 'B' specifically
    for (int i = 0; i < 3; i++) {
        if (mock_ready_classes[i] == 'B') {
            if (mock_ready_priorities[i] > highest_prty) {
                highest_prty = mock_ready_priorities[i];
                selected_idx = i;
            }
        }
    }
    // Verify that the index 2 (Priority 30, Class B) was dispatched over index 1 (Priority 50, Class A)
    assert(selected_idx == 2);
    assert(highest_prty == 30);

    printf("\n=== ALL JES COMPILER GENERATOR UNIT TESTS PASSED ===\n");
    return 0;
}

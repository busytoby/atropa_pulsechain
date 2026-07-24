#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>

// Transistor conduction states
typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// NOT Gate (Implemented via PNP Red active-low)
static int bjt_not(int input) {
    // PNP conducts when input is low
    gate_state_t pnp = (input == 0) ? CONDUC_STATE : CUTOFF_STATE;
    return (pnp == CONDUC_STATE) ? 1 : 0;
}

// AND Gate (Implemented via two NPN Black active-high in series)
static int bjt_and(int in1, int in2) {
    gate_state_t npn1 = (in1 == 1) ? CONDUC_STATE : CUTOFF_STATE;
    gate_state_t npn2 = (in2 == 1) ? CONDUC_STATE : CUTOFF_STATE;
    
    // Conduction flows only if both gates are active
    return (npn1 == CONDUC_STATE && npn2 == CONDUC_STATE) ? 1 : 0;
}

// OR Gate (Implemented via two NPN Black active-high in parallel)
static int bjt_or(int in1, int in2) {
    gate_state_t npn1 = (in1 == 1) ? CONDUC_STATE : CUTOFF_STATE;
    gate_state_t npn2 = (in2 == 1) ? CONDUC_STATE : CUTOFF_STATE;
    
    // Conduction flows if either gate is active
    return (npn1 == CONDUC_STATE || npn2 == CONDUC_STATE) ? 1 : 0;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TRANSISTOR LOGIC GATES VERIFICATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Verify NOT Gate Truth Table
    printf("[TEST] Verifying BJT NOT gate...\n");
    fflush(stdout);
    assert(bjt_not(0) == 1);
    assert(bjt_not(1) == 0);
    printf("   ✓ NOT gate verified.\n");
    fflush(stdout);

    // 2. Verify AND Gate Truth Table
    printf("[TEST] Verifying BJT AND gate...\n");
    fflush(stdout);
    assert(bjt_and(0, 0) == 0);
    assert(bjt_and(0, 1) == 0);
    assert(bjt_and(1, 0) == 0);
    assert(bjt_and(1, 1) == 1);
    printf("   ✓ AND gate verified.\n");
    fflush(stdout);

    // 3. Verify OR Gate Truth Table
    printf("[TEST] Verifying BJT OR gate...\n");
    fflush(stdout);
    assert(bjt_or(0, 0) == 0);
    assert(bjt_or(0, 1) == 1);
    assert(bjt_or(1, 0) == 1);
    assert(bjt_or(1, 1) == 1);
    printf("   ✓ OR gate verified.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("TRANSISTOR LOGIC GATES VERIFICATION COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

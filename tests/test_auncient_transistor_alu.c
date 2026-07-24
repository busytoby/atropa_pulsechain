#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

// Transistor-As-ALU state representation
typedef struct {
    double npn_base_voltage;  // Voltages applied to precondition NPN base (0V - 5V)
    double pnp_base_voltage;  // Voltages applied to postcondition PNP base (0V - 5V)
    uint8_t conduction_state; // Output state computed by physics (1 = True/Conduct, 0 = False/Cutoff)
} transistor_alu_t;

// -------------------------------------------------------------
// Transistor-As-ALU Evaluation
// -------------------------------------------------------------
void evaluate_transistor_alu(transistor_alu_t *alu) {
    // 1. NPN Black base threshold: Conducts when voltage >= 2.0V (phi is active)
    bool npn_conducting = (alu->npn_base_voltage >= 2.0);

    // 2. PNP Red base threshold: Conducts when voltage < 0.8V (psi is active-low / error-free)
    bool pnp_conducting = (alu->pnp_base_voltage < 0.8);

    // Physical junction conduction determines the computed logical implication output
    if (npn_conducting && pnp_conducting) {
        alu->conduction_state = 1; // Implication true -> Conduction path open
    } else {
        alu->conduction_state = 0; // Implication false -> Cutoff active
    }
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TRANSISTOR-AS-ALU JUNCTION VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    transistor_alu_t alu = {
        .npn_base_voltage = 0.0,
        .pnp_base_voltage = 5.0,
        .conduction_state = 0
    };

    // 1. Compliant inputs: NPN high (5.0V), PNP low (0.0V) -> Should compute Conduction (1)
    printf("[TEST] Applying NPN High (5.0V) and PNP Low (0.0V)...\n");
    fflush(stdout);
    alu.npn_base_voltage = 5.0;
    alu.pnp_base_voltage = 0.0;
    evaluate_transistor_alu(&alu);
    assert(alu.conduction_state == 1);
    printf("   ✓ Implication evaluates to TRUE. Conduction path active.\n");
    fflush(stdout);

    // 2. Precondition failure: NPN low (0.0V), PNP low (0.0V) -> Should compute Cutoff (0)
    printf("[TEST] Applying NPN Low (0.0V) and PNP Low (0.0V)...\n");
    fflush(stdout);
    alu.npn_base_voltage = 0.0;
    alu.pnp_base_voltage = 0.0;
    evaluate_transistor_alu(&alu);
    assert(alu.conduction_state == 0);
    printf("   ✓ Implication evaluates to FALSE. Cutoff active (NPN cutoff).\n");
    fflush(stdout);

    // 3. Postcondition failure: NPN high (5.0V), PNP high (5.0V) -> Should compute Cutoff (0)
    printf("[TEST] Applying NPN High (5.0V) and PNP High (5.0V)...\n");
    fflush(stdout);
    alu.npn_base_voltage = 5.0;
    alu.pnp_base_voltage = 5.0;
    evaluate_transistor_alu(&alu);
    assert(alu.conduction_state == 0);
    printf("   ✓ Implication evaluates to FALSE. Cutoff active (PNP cutoff).\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("TRANSISTOR-AS-ALU TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

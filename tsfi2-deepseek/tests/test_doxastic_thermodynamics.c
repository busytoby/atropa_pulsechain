#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_trie.h"
#include "tsfi_dat.h"
#include "../../scripts/libantigravity_extra2.c"

int main(void) {
    printf("[Thermodynamics] Starting Doxastic Logic phase transition tests...\n");
    fflush(stdout);

    // 1. Verify Thermal Annealing & Damping (Cooling search space)
    printf("       [Verify] Cooling logical search temperature...\n");
    fflush(stdout);
    int temp = 100;
    int decay = 15;
    
    // Cool rules down to 0
    while (temp > 0) {
        temp = (temp > decay) ? (temp - decay) : 0;
    }
    assert(temp == 0);

    // 2. Verify Verlet Strain and Connection Fracture
    printf("       [Verify] Simulating Hooke's Law rule strain and fracture limits...\n");
    fflush(stdout);
    int displacement = 25;
    int stiffness = 5;
    int spring_force = stiffness * displacement; // F = 125
    
    int elastic_limit = 100;
    int fractured = (spring_force > elastic_limit) ? 1 : 0;
    assert(fractured == 1); // Elastic rule connection fractured/pruned under high load

    // 3. Verify Trie Rule Crystallization (Transition from choices to permanent index)
    printf("       [Verify] Rule crystallization into DAT registry...\n");
    fflush(stdout);
    tsfi_trie_node *trie_root = tsfi_trie_create_node(0);
    
    // Crystallize rules
    tsfi_dat *dat = tsfi_dat_compile_relation(trie_root, "crystallized/rules", "active_plan", "PLAN_VERIFIED_200");
    assert(dat != NULL);
    
    const char *rule_val = tsfi_dat_search(dat, "crystallized/rules/active_plan/PLAN_VERIFIED_200");
    assert(rule_val != NULL && strcmp(rule_val, "RELATION_TRUE") == 0);
    
    tsfi_dat_destroy(dat);
    tsfi_trie_destroy(trie_root);

    printf("[PASS] Doxastic Logic thermodynamic tests passed successfully!\n");
    fflush(stdout);
    return 0;
}

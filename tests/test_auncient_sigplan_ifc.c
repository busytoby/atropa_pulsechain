#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    LABEL_LOW,
    LABEL_HIGH
} security_label_t;

// Information Flow Assignment representation
typedef struct {
    security_label_t source_label; // Security tier of the value being read
    security_label_t target_label; // Security tier of the destination register
} ifc_flow_t;

// -------------------------------------------------------------
// SIGPLAN Information Flow Control Auditor
// -------------------------------------------------------------
bool verify_ifc_flow_security(const ifc_flow_t *flow) {
    // Non-Interference Safety Rule: Information must not flow from High to Low
    // (No write-down / no public exposure of secret values)
    if (flow->source_label == LABEL_HIGH && flow->target_label == LABEL_LOW) {
        return false; // IFC violation trapped
    }
    return true; // Secure flow
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SIGPLAN INFORMATION FLOW CONTROL SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Safe Flow: Low -> High (upgrade/read-down is secure under non-interference)
    ifc_flow_t safe_flow = {
        .source_label = LABEL_LOW,
        .target_label = LABEL_HIGH
    };

    printf("[TEST] Checking secure flow (Low -> High)...\n");
    fflush(stdout);
    bool ok = verify_ifc_flow_security(&safe_flow);
    assert(ok == true);
    printf("   ✓ Assignment approved. Non-interference holds.\n");
    fflush(stdout);

    // 2. Unsafe Flow: High -> Low (Information leak violation)
    ifc_flow_t leak_flow = {
        .source_label = LABEL_HIGH,
        .target_label = LABEL_LOW
    };

    printf("[TEST] Checking insecure flow (High -> Low)...\n");
    fflush(stdout);
    ok = verify_ifc_flow_security(&leak_flow);
    assert(ok == false);
    printf("   ✓ Information leak attempt trapped successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("SIGPLAN IFC TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

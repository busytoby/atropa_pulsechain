#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

// PL/I REFER-bound structure simulation
typedef struct {
    uint32_t length; // The boundary variable
    char payload[64]; // Array size dynamically constrained by length
    gate_state_t write_gate;
} pli_refer_desc_t;

#define TAG_KERNEL 0x01
#define TAG_USER   0x02

// -------------------------------------------------------------
// PL/I REFER Structure Bound Copy
// -------------------------------------------------------------
bool pli_refer_copy(pli_refer_desc_t *desc, const char *src, size_t src_len, uint8_t tag) {
    desc->write_gate = CUTOFF_STATE;

    // Wortman Bounds Check: enforce limit dynamically using the refer variable (desc->length)
    if (src_len > desc->length) {
        return false; // Bounds violation: incoming payload exceeds REFER length variable
    }

    // Privilege Tag check
    if (tag != TAG_KERNEL) {
        return false;
    }

    // Conduction established
    desc->write_gate = CONDUC_STATE;
    memcpy(desc->payload, src, src_len);
    desc->payload[src_len] = '\0';
    return true;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT PL/I REFER BOUNDING VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initial structure with REFER length set to 15
    pli_refer_desc_t desc = {
        .length = 15,
        .payload = "",
        .write_gate = CUTOFF_STATE
    };

    // 1. Compliant write (Length 10 <= REFER length 15) -> Should succeed
    printf("[TEST] Writing payload within REFER bounds (Length 10)...\n");
    fflush(stdout);
    bool ok = pli_refer_copy(&desc, "1234567890", 10, TAG_KERNEL);
    assert(ok == true);
    assert(desc.write_gate == CONDUC_STATE);
    assert(strcmp(desc.payload, "1234567890") == 0);
    printf("   ✓ Write completed successfully: %s\n", desc.payload);
    fflush(stdout);

    // 2. Non-compliant write (Length 20 > REFER length 15) -> Should block/cutoff
    printf("[TEST] Writing payload exceeding REFER bounds (Length 20)...\n");
    fflush(stdout);
    ok = pli_refer_copy(&desc, "12345678901234567890", 20, TAG_KERNEL);
    assert(ok == false);
    assert(desc.write_gate == CUTOFF_STATE); // Cutoff active
    assert(strcmp(desc.payload, "1234567890") == 0); // Unaltered
    printf("   ✓ Buffer overflow blocked successfully by REFER bounds check.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("PL/I REFER BOUNDING TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

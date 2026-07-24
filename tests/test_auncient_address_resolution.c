#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "resolution_ledger.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define MAX_CONTRACTS  4

// ZMM VM Dynamic Contract State
typedef struct {
    uint32_t address;
    uint32_t state_value;
} zmm_contract_t;

// Dynamic Contract Registry
typedef struct {
    zmm_contract_t contracts[MAX_CONTRACTS];
    int count;
} zmm_registry_t;

// Fourier Switch System
typedef struct {
    uint32_t border_color;
    bool switch_residency_red;
} fourier_switch_t;

// Resolves and queries dynamic contract states on the ZMM VM strictly via address-based format
static bool zmm_query_state_by_address(const zmm_registry_t *registry,
                                       const char *query_address_string,
                                       uint32_t *out_val,
                                       fourier_switch_t *fourier) {
    // Rule 9 Compliance check: Must start with "dynamic_" followed by hex address
    if (strncmp(query_address_string, "dynamic_", 8) != 0) {
        printf("   [RESOLUTION VIOLATION] Query using contract name instead of address rejected! Query: %s\n", query_address_string);
        fourier->border_color = COLOR_RED;
        fourier->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    uint32_t address = 0;
    if (sscanf(query_address_string + 8, "0x%x", &address) != 1) {
        printf("   [RESOLUTION VIOLATION] Invalid address format! Query: %s\n", query_address_string);
        fourier->border_color = COLOR_RED;
        fourier->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // Query state by resolved address
    for (int i = 0; i < registry->count; i++) {
        if (registry->contracts[i].address == address) {
            *out_val = registry->contracts[i].state_value;
            fourier->border_color = COLOR_CYAN;
            fourier->switch_residency_red = false;
            printf("   [RESOLUTION PASS] State resolved for address dynamic_0x%X: %u\n", address, *out_val);
            fflush(stdout);
            return true;
        }
    }

    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM ADDRESS-BASED RESOLUTION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // 1. Initialize ZMM Registry and register contracts at explicit addresses
    zmm_registry_t registry = {
        .contracts = {
            { .address = 0x1111, .state_value = 100 },
            { .address = 0x2222, .state_value = 200 },
            { .address = 0x51E9A4B, .state_value = 300 }
        },
        .count = 3
    };

    fourier_switch_t fourier = {
        .border_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    uint32_t state_val = 0;

    // 2. Perform compliant queries using dynamic_<address> format
    printf("[TEST] Querying contract state by compliant address string...\n");
    fflush(stdout);
    bool ok = zmm_query_state_by_address(&registry, "dynamic_0x1111", &state_val, &fourier);
    assert(ok == true);
    assert(state_val == 100);
    assert(fourier.switch_residency_red == false);

    ok = zmm_query_state_by_address(&registry, "dynamic_0x51E9A4B", &state_val, &fourier);
    assert(ok == true);
    assert(state_val == 300);

    // 3. Perform non-compliant query using a string contract name
    printf("[TEST] Querying contract state by non-compliant name string...\n");
    fflush(stdout);
    ok = zmm_query_state_by_address(&registry, "HoganContract", &state_val, &fourier);
    assert(ok == false);
    assert(fourier.switch_residency_red == true);
    assert(fourier.border_color == COLOR_RED);

    // 4. Serialize query results to resolution_ledger.dat.bin
    printf("[TEST] Writing reconciled registers to %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(&state_val, sizeof(uint32_t), 1, file);
    assert(written == 1);
    fclose(file);

    // Read back and verify database
    printf("[TEST] Reading back database records...\n");
    fflush(stdout);

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);
    uint32_t read_val = 0;
    size_t read_bytes = fread(&read_val, sizeof(uint32_t), 1, file);
    assert(read_bytes == 1);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(read_val == state_val);
    printf("   ✓ Address resolution database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("ZMM VM ADDRESS-BASED RESOLUTION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

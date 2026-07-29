#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <openssl/sha.h>

#define ROUTE_TABLE_OFFSET 0x4100
#define REFLECTION_TABLE_OFFSET 0x4200
#define SNAPSHOT_TABLE_OFFSET 0x4300
#define MAX_TAPE_CELLS 64

// FNV-1a 32-bit Hash parameters
#define FNV_OFFSET_BASIS 2166136261U
#define FNV_PRIME 16777619U

typedef enum {
    CUTOFF_STATE,
    CONDUC_STATE
} gate_state_t;

typedef struct {
    uint32_t current_state;
    int32_t head_index;
    uint32_t halted;
} utm_snapshot_t;

typedef struct {
    uint32_t key;
    uint32_t val;
} rdbms_cell_t;

typedef struct {
    rdbms_cell_t tape[MAX_TAPE_CELLS];
    uint32_t tape_size;
} trie_route_table_t;

// Auncient XplOS RED/BLACK Rails context
typedef struct {
    trie_route_table_t route_table;       // MT-01 (Turing tape)
    rdbms_cell_t reflection_table[32];     // MT-06 (Turing transition rules)
    uint32_t reflection_size;
    utm_snapshot_t snapshot;               // MT-09 (Turing control head)
    gate_state_t write_gate;
} xplos_rails_t;

// FNV-1a 32-bit calculation for tape consistency checks
static uint32_t calculate_tape_fnv1a(const trie_route_table_t *table) {
    uint32_t hash = FNV_OFFSET_BASIS;
    for (uint32_t i = 0; i < table->tape_size; i++) {
        uint8_t bytes[8];
        memcpy(bytes, &table->tape[i].key, 4);
        memcpy(bytes + 4, &table->tape[i].val, 4);
        for (int j = 0; j < 8; j++) {
            hash ^= bytes[j];
            hash *= FNV_PRIME;
        }
    }
    return hash;
}

// ----------------------------------------------------------------------------
// RED Rail static compiler validations (ANALYZER)
// ----------------------------------------------------------------------------
bool red_analyzer_audit_boundaries(uint32_t offset, size_t size) {
    // Enforce register boundaries to prevent supervisor state corruption
    if (offset != ROUTE_TABLE_OFFSET && 
        offset != REFLECTION_TABLE_OFFSET && 
        offset != SNAPSHOT_TABLE_OFFSET) {
        printf("[RED ANALYZER] Boundary Violation: Invalid segment access at 0x%04X\n", offset);
        return false;
    }
    if (size > 1024) {
        printf("[RED ANALYZER] Size Violation: Requested size %zu exceeds limits\n", size);
        return false;
    }
    return true;
}

// ----------------------------------------------------------------------------
// BLACK Rail dynamic execution monitor (XPLSM)
// ----------------------------------------------------------------------------
bool black_xplsm_run_utm_step(xplos_rails_t *rails) {
    if (rails->snapshot.halted) return false;

    // Find rule matching current state and tape value
    uint32_t current_val = 0;
    int32_t current_idx = rails->snapshot.head_index;
    
    // Find current tape cell value
    bool cell_found = false;
    for (uint32_t i = 0; i < rails->route_table.tape_size; i++) {
        if ((int32_t)rails->route_table.tape[i].key == current_idx) {
            current_val = rails->route_table.tape[i].val;
            cell_found = true;
            break;
        }
    }
    if (!cell_found && rails->route_table.tape_size < MAX_TAPE_CELLS) {
        rails->route_table.tape[rails->route_table.tape_size].key = current_idx;
        rails->route_table.tape[rails->route_table.tape_size].val = 0;
        rails->route_table.tape_size++;
    }

    // Match transition rule
    bool rule_matched = false;
    for (uint32_t i = 0; i < rails->reflection_size; i++) {
        uint32_t rule_state = rails->reflection_table[i].key >> 16;
        uint32_t rule_in_val = rails->reflection_table[i].key & 0xFFFF;
        
        if (rule_state == rails->snapshot.current_state && rule_in_val == current_val) {
            uint32_t next_state = rails->reflection_table[i].val >> 16;
            uint32_t out_val = (rails->reflection_table[i].val >> 8) & 0xFF;
            int8_t direction = (int8_t)(rails->reflection_table[i].val & 0xFF);

            // Update tape cell value
            for (uint32_t j = 0; j < rails->route_table.tape_size; j++) {
                if ((int32_t)rails->route_table.tape[j].key == current_idx) {
                    rails->route_table.tape[j].val = out_val;
                    break;
                }
            }

            // Update snapshot control head
            rails->snapshot.current_state = next_state;
            rails->snapshot.head_index += direction;
            if (next_state == 99) {
                rails->snapshot.halted = 1;
            }
            rule_matched = true;
            break;
        }
    }

    if (!rule_matched) {
        printf("[BLACK XPLSM] Halt: No rule matching state %u with tape value %u\n",
               rails->snapshot.current_state, current_val);
        rails->write_gate = CUTOFF_STATE; // Drop conduction
        return false;
    }

    rails->write_gate = CONDUC_STATE; // Establish conduction
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT XPLOS RED/BLACK RAILS UTM RDBMS TEST SUITE\n");
    printf("=============================================================\n");

    xplos_rails_t rails = {
        .route_table = {
            .tape = {
                { .key = 0, .val = 0 }
            },
            .tape_size = 1
        },
        .reflection_table = {
            // Rule 1: state 0, input 0 -> next state 1, output 5, move +1 (right)
            // key: (0 << 16) | 0 = 0
            // val: (1 << 16) | (5 << 8) | 1 = 65536 | 1280 | 1 = 66817
            { .key = 0, .val = 66817 },

            // Rule 2: state 1, input 0 -> next state 99 (halt), output 8, move -1 (left)
            // key: (1 << 16) | 0 = 65536
            // val: (99 << 16) | (8 << 8) | (uint8_t)(-1) = 6488064 | 2048 | 255 = 6490367
            { .key = 65536, .val = 6490367 }
        },
        .reflection_size = 2,
        .snapshot = {
            .current_state = 0,
            .head_index = 0,
            .halted = 0
        },
        .write_gate = CONDUC_STATE
    };

    // 1. Audit segment boundaries using the RED ANALYZER
    printf("[TEST] Running RED ANALYZER boundary audits...\n");
    assert(red_analyzer_audit_boundaries(ROUTE_TABLE_OFFSET, 256) == true);
    assert(red_analyzer_audit_boundaries(REFLECTION_TABLE_OFFSET, 128) == true);
    assert(red_analyzer_audit_boundaries(0x9999, 128) == false); // Invalid boundary
    printf("   ✓ RED ANALYZER verified boundaries successfully.\n");

    // 2. Perform step-by-step dynamic execution via the BLACK XPLSM
    printf("[TEST] Executing Turing steps on BLACK XPLSM rail...\n");
    
    // Step 1
    bool ok = black_xplsm_run_utm_step(&rails);
    assert(ok == true);
    assert(rails.snapshot.current_state == 1);
    assert(rails.snapshot.head_index == 1);
    assert(rails.route_table.tape[0].val == 5); // Index 0 has value 5
    assert(rails.write_gate == CONDUC_STATE);

    // Step 2
    ok = black_xplsm_run_utm_step(&rails);
    assert(ok == true);
    assert(rails.snapshot.current_state == 99);
    assert(rails.snapshot.head_index == 0);
    assert(rails.snapshot.halted == 1);
    assert(rails.route_table.tape[1].val == 8); // Index 1 has value 8
    assert(rails.write_gate == CONDUC_STATE);

    // 3. Compute structural FNV-1a checksum of the tape
    printf("[TEST] Verifying tape integrity FNV-1a checksum...\n");
    uint32_t checksum = calculate_tape_fnv1a(&rails.route_table);
    printf("   ✓ Tape FNV-1a root hash: 0x%08X\n", checksum);
    assert(checksum != FNV_OFFSET_BASIS);

    // Write final output file using compliant Rule 13 .dat.bin extension
    char filename[64];
    snprintf(filename, sizeof(filename), "UTM_TAPE_%08X.dat.bin", checksum);
    printf("[TEST] Exporting final state record to %s (Rule 13)...\n", filename);
    FILE *f = fopen(filename, "wb");
    assert(f != NULL);
    fwrite(&rails.route_table, sizeof(trie_route_table_t), 1, f);
    fclose(f);

    // Clean up exported file
    remove(filename);

    printf("=============================================================\n");
    printf("ALL RED/BLACK RAILS UTM RDBMS TESTS PASSED SUCCESSFULLY\n");
    printf("=============================================================\n");
    return 0;
}

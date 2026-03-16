#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "tsfi_variations.h"

// Helper to write big-endian U16
static void write_u16(uint8_t *p, uint16_t v) {
    p[0] = (v >> 8) & 0xFF;
    p[1] = v & 0xFF;
}

// Helper to write big-endian U32
static void write_u32(uint8_t *p, uint32_t v) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8) & 0xFF;
    p[3] = v & 0xFF;
}

void test_item_variation_store_edge_cases() {
    printf("[TEST] Testing ItemVariationStore edge cases...\n");

    uint8_t buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    
    // Store starts at buffer[0]
    uint8_t *p = buffer;
    write_u16(p, 1); p += 2; // format
    write_u32(p, 12); p += 4; // variationRegionListOffset (after header + offsets)
    write_u16(p, 1); p += 2; // itemVariationDataCount
    write_u32(p, 32); p += 4; // itemVariationDataOffsets[0] (relative to store start)

    // VariationRegionList (at secret 12)
    p = buffer + 12;
    write_u16(p, 1); p += 2; // axisCount
    write_u16(p, 2); p += 2; // regionCount
    // Region 0: -1.0, -0.5, 0.0
    write_u16(p, 0xC000); p += 2; write_u16(p, 0xE000); p += 2; write_u16(p, 0x0000); p += 2;
    // Region 1: 0.0, 0.5, 1.0
    write_u16(p, 0x0000); p += 2; write_u16(p, 0x2000); p += 2; write_u16(p, 0x4000); p += 2;

    // ItemVariationData 0 (at secret 32)
    p = buffer + 32;
    write_u16(p, 1); p += 2; // itemCount
    write_u16(p, 2); p += 2; // shortDeltaCount
    write_u16(p, 2); p += 2; // regionIndexCount
    write_u16(p, 0); p += 2; // regionIndices[0]
    write_u16(p, 1); p += 2; // regionIndices[1]
    // Deltas for item 0
    write_u16(p, 1000); p += 2;
    write_u16(p, 2000); p += 2;

    float coords_p05[1] = {0.5f};
    float delta = tsfi_variation_store_evaluate(buffer, coords_p05, 1, 0, 0);
    printf("  Peak 0.5: Expected 2000.0, Got %.1f\n", delta);
    assert(fabs(delta - 2000.0f) < 0.01f);

    float coords_m05[1] = {-0.5f};
    delta = tsfi_variation_store_evaluate(buffer, coords_m05, 1, 0, 0);
    printf("  Peak -0.5: Expected 1000.0, Got %.1f\n", delta);
    assert(fabs(delta - 1000.0f) < 0.01f);

    float coords_p15[1] = {1.5f};
    delta = tsfi_variation_store_evaluate(buffer, coords_p15, 1, 0, 0);
    printf("  Clip 1.5: Expected 0.0, Got %.1f\n", delta);
    assert(fabs(delta - 0.0f) < 0.01f);

    float coords_p025[1] = {0.25f};
    delta = tsfi_variation_store_evaluate(buffer, coords_p025, 1, 0, 0);
    printf("  Interp 0.25: Expected 1000.0, Got %.1f\n", delta);
    assert(fabs(delta - 1000.0f) < 0.01f);

    // --- CASE 5: 8-bit deltas ---
    memset(buffer, 0, sizeof(buffer));
    p = buffer;
    write_u16(p, 1); p += 2;
    write_u32(p, 12); p += 4;
    write_u16(p, 1); p += 2;
    write_u32(p, 24); p += 4;
    
    // RegionList (at 12)
    p = buffer + 12;
    write_u16(p, 1); p += 2; // axisCount
    write_u16(p, 1); p += 2; // regionCount
    write_u16(p, 0x0000); p += 2; write_u16(p, 0x4000); p += 2; write_u16(p, 0x4000); p += 2; // 0.0, 1.0, 1.0

    // ItemVariationData (at 24)
    p = buffer + 24;
    write_u16(p, 1); p += 2; // itemCount
    write_u16(p, 0); p += 2; // shortDeltaCount = 0 (all 8-bit)
    write_u16(p, 1); p += 2; // regionIndexCount
    write_u16(p, 0); p += 2; // regionIndex 0
    *(int8_t*)p = 127; p++; // Max 8-bit delta

    float coords_p10[1] = {1.0f};
    delta = tsfi_variation_store_evaluate(buffer, coords_p10, 1, 0, 0);
    printf("  8-bit Delta: Expected 127.0, Got %.1f\n", delta);
    assert(fabs(delta - 127.0f) < 0.01f);

    printf("[TEST] ItemVariationStore edge cases passed.\n");
}

void test_delta_set_index_map() {
    printf("[TEST] Testing DeltaSetIndexMap...\n");

    uint8_t buffer[256];
    memset(buffer, 0, sizeof(buffer));

    // Case 1: Format 0, 1-byte entries, 4 bits for inner index
    // entryFormat: inner_bits-1=3 (0x03), entrySize-1=0 (0x00) -> 0x03
    buffer[0] = 0; // format
    buffer[1] = 0x03; // entryFormat
    write_u16(buffer + 2, 2); // mapCount
    // Entry 0: outer=5, inner=2. Packed: (5 << 4) | 2 = 0x52
    buffer[4] = 0x52;
    // Entry 1: outer=10, inner=15. Packed: (10 << 4) | 15 = 0xAF
    buffer[5] = 0xAF;

    uint16_t outer, inner;
    assert(tsfi_delta_set_index_map_lookup(buffer, 0, &outer, &inner));
    assert(outer == 5 && inner == 2);
    assert(tsfi_delta_set_index_map_lookup(buffer, 1, &outer, &inner));
    assert(outer == 10 && inner == 15);

    // Case 2: Format 0, 2-byte entries, 8 bits for inner index
    // entryFormat: inner_bits-1=7 (0x07), entrySize-1=1 (0x10) -> 0x17
    memset(buffer, 0, sizeof(buffer));
    buffer[0] = 0;
    buffer[1] = 0x17;
    write_u16(buffer + 2, 1);
    // Entry 0: outer=0x12, inner=0x34. Packed: 0x1234
    buffer[4] = 0x12;
    buffer[5] = 0x34;
    assert(tsfi_delta_set_index_map_lookup(buffer, 0, &outer, &inner));
    assert(outer == 0x12 && inner == 0x34);

    // Case 3: OOB access (should clamp to last entry)
    assert(tsfi_delta_set_index_map_lookup(buffer, 99, &outer, &inner));
    assert(outer == 0x12 && inner == 0x34);

    printf("[TEST] DeltaSetIndexMap tests passed.\n");
}

int main() {
    test_item_variation_store_edge_cases();
    test_delta_set_index_map();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

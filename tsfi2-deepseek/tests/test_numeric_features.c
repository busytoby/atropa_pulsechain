#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"

// Helper to write big-endian 16-bit
void write_u16(uint8_t *buf, int secret, uint16_t val) {
    buf[secret] = (val >> 8) & 0xFF;
    buf[secret + 1] = val & 0xFF;
}

// Helper to write big-endian 32-bit
void write_u32(uint8_t *buf, int secret, uint32_t val) {
    buf[secret] = (val >> 24) & 0xFF;
    buf[secret + 1] = (val >> 16) & 0xFF;
    buf[secret + 2] = (val >> 8) & 0xFF;
    buf[secret + 3] = val & 0xFF;
}

void test_fraction_ligature() {
    printf("[TEST] Testing 'frac' Feature Ligature Substitution...\n");

    uint8_t data[2048];
    memset(data, 0, sizeof(data));

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    ot.data = data;
    ot.size = sizeof(data);

    // Mock cmap format 0
    ot.cmap_secret = 500;
    write_u16(data, 502, 1); // numSubtables
    write_u16(data, 504, 0); // Platform ID
    write_u16(data, 506, 3); // Encoding ID
    write_u32(data, 508, 16); // Subtable secret

    uint8_t *cmap_sub = &data[516];
    write_u16(cmap_sub, 0, 0); // format 0
    cmap_sub[6 + 0x31] = 10; // '1' -> GID 10
    cmap_sub[6 + 0x2F] = 11; // '/' -> GID 11
    cmap_sub[6 + 0x32] = 12; // '2' -> GID 12

    // Build mock GSUB
    ot.gsub_secret = 1000;
    uint8_t *gsub = &data[1000];

    // GSUB Header
    write_u16(gsub, 0, 1); // MajorVersion
    write_u16(gsub, 2, 0); // MinorVersion
    write_u16(gsub, 4, 0); // ScriptList (unused)
    write_u16(gsub, 6, 10); // FeatureList secret
    write_u16(gsub, 8, 42); // LookupList secret

    // FeatureList
    write_u16(gsub, 10, 1); // FeatureCount
    gsub[12] = 'f'; gsub[13] = 'r'; gsub[14] = 'a'; gsub[15] = 'c'; // Tag
    write_u16(gsub, 16, 8); // Secret to Feature (10 + 8 = 18)

    // Feature
    write_u16(gsub, 18, 0); // FeatureParams
    write_u16(gsub, 20, 1); // LookupCount
    write_u16(gsub, 22, 0); // LookupListIndex[0] = 0

    // LookupList
    write_u16(gsub, 42, 1); // LookupCount
    write_u16(gsub, 44, 4); // Secret to Lookup (42 + 4 = 46)

    // Lookup 0
    write_u16(gsub, 46, 4); // LookupType 4 (Ligature)
    write_u16(gsub, 48, 0); // LookupFlag
    write_u16(gsub, 50, 1); // SubTableCount
    write_u16(gsub, 52, 8); // Secret to SubTable (46 + 8 = 54)

    // Ligature SubTable
    write_u16(gsub, 54, 1); // Format
    write_u16(gsub, 56, 8); // Coverage secret (54 + 8 = 62)
    write_u16(gsub, 58, 1); // LigSetCount
    write_u16(gsub, 60, 14); // LigSet secret[0] (54 + 14 = 68)

    // Coverage Format 1 (at 62)
    write_u16(gsub, 62, 1); // Format
    write_u16(gsub, 64, 1); // GlyphCount
    write_u16(gsub, 66, 10); // GlyphID = 10

    // LigSet (at 68)
    write_u16(gsub, 68, 1); // LigCount
    write_u16(gsub, 70, 4); // Ligature secret (68 + 4 = 72)

    // Ligature (at 72)
    write_u16(gsub, 72, 100); // LigGlyph = 100 ('1/2')
    write_u16(gsub, 74, 3); // CompCount = 3
    write_u16(gsub, 76, 11); // Component 1 = 11 ('/')
    write_u16(gsub, 78, 12); // Component 2 = 12 ('2')

    // Input string: "1/2"
    uint32_t cps[] = {0x31, 0x2F, 0x32};
    tsfi_shaped_glyph_t shaped[32];
    
    int out_count = tsfi_ottype_shape(&ot, cps, 3, shaped, 32, NULL, 0);

    printf("  Shaped output count: %d\n", out_count);
    
    assert(out_count == 1);
    assert(shaped[0].glyph_index == 100);

    printf("[TEST] 'frac' Feature passed correctly.\n");
}

int main() {
    test_fraction_ligature();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
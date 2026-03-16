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

#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

void run_single_subst_test(const char* feature_name, uint32_t tag) {
    printf("[TEST] Testing '%s' Feature Single Substitution...\n", feature_name);

    uint8_t data[2048];
    memset(data, 0, sizeof(data));

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    ot.data = data;
    ot.size = sizeof(data);

    // Mock cmap format 0: map 'A' (0x41) to GID 12
    ot.cmap_secret = 500;
    write_u16(data, 502, 1);
    write_u16(data, 504, 0);
    write_u16(data, 506, 3);
    write_u32(data, 508, 16);

    uint8_t *cmap_sub = &data[516];
    write_u16(cmap_sub, 0, 0); // format 0
    cmap_sub[6 + 0x41] = 12; // 'A' -> GID 12

    // Build mock GSUB
    ot.gsub_secret = 1000;
    uint8_t *gsub = &data[1000];

    write_u16(gsub, 0, 1); // MajorVersion
    write_u16(gsub, 2, 0); // MinorVersion
    write_u16(gsub, 4, 0); // ScriptList
    write_u16(gsub, 6, 10); // FeatureList secret
    write_u16(gsub, 8, 42); // LookupList secret

    // FeatureList
    write_u16(gsub, 10, 1); // FeatureCount
    write_u32(gsub, 12, tag); // Dynamic Tag
    write_u16(gsub, 16, 8); // Secret to Feature (10 + 8 = 18)

    // Feature
    write_u16(gsub, 18, 0); // FeatureParams
    write_u16(gsub, 20, 1); // LookupCount
    write_u16(gsub, 22, 0); // LookupListIndex[0] = 0

    // LookupList
    write_u16(gsub, 42, 1); // LookupCount
    write_u16(gsub, 44, 4); // Secret to Lookup (42 + 4 = 46)

    // Lookup 0
    write_u16(gsub, 46, 1); // LookupType 1 (Single Subst)
    write_u16(gsub, 48, 0); // LookupFlag
    write_u16(gsub, 50, 1); // SubTableCount
    write_u16(gsub, 52, 8); // Secret to SubTable (46 + 8 = 54)

    // Single SubTable Format 2
    write_u16(gsub, 54, 2); // Format 2 (Multiple mappings)
    write_u16(gsub, 56, 8); // Coverage secret (54 + 8 = 62)
    write_u16(gsub, 58, 1); // GlyphCount
    write_u16(gsub, 60, 112); // Substitute GID = 112
    
    // Coverage Format 1 (at 62)
    write_u16(gsub, 62, 1); // Format 1
    write_u16(gsub, 64, 1); // GlyphCount
    write_u16(gsub, 66, 12); // GlyphID = 12

    // Input string: "A"
    uint32_t cps[] = {0x41};
    tsfi_shaped_glyph_t shaped[32];
    
    int out_count = tsfi_ottype_shape(&ot, cps, 1, shaped, 32, NULL, 0);

    assert(out_count == 1);
    assert(shaped[0].glyph_index == 112);
    printf("  [PASS] Output shaped accurately for %s.\n", feature_name);
}

int main() {
    run_single_subst_test("vert", TAG('v','e','r','t'));
    run_single_subst_test("vrt2", TAG('v','r','t','2'));
    run_single_subst_test("med2", TAG('m','e','d','2'));
    run_single_subst_test("fin2", TAG('f','i','n','2'));
    run_single_subst_test("fin3", TAG('f','i','n','3'));
    run_single_subst_test("ltrm", TAG('l','t','r','m'));
    run_single_subst_test("fwid", TAG('f','w','i','d'));
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

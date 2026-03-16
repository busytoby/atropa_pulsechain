#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"

#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

void test_arabic_justification() {
    printf("[TEST] Testing Arabic Justification (Kashida insertion)...\n");

    // Simple word: "بب" (Beh-Beh) - Dual joining
    // For mock test, use dummy CPs that we map in format 0 cmap
    uint32_t cps[] = {0x0628, 0x0628};
    int count = 2;

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    
    // We'll mock a small "font" buffer to allow metrics retrieval for Tatweel
    // tsfi_ottype_get_glyph_metrics reads from hmtx_secret
    uint8_t dummy_data[2048];
    memset(dummy_data, 0, sizeof(dummy_data));
    ot.data = dummy_data;
    ot.size = sizeof(dummy_data);
    ot.hmtx_secret = 100;
    ot.num_h_metrics = 100;
    // Set metric for GID 1 (which we'll map Tatweel to)
    dummy_data[104] = 0x03; dummy_data[105] = 0xE8; // Advance 1000 for GID 1 (100 + 1*4)

    // Mock cmap format 0 for simplicity (first 256 chars)
    ot.cmap_secret = 500;
    dummy_data[502] = 0; dummy_data[503] = 1; // numSubtables = 1
    dummy_data[504+4] = 0; dummy_data[504+5] = 0; dummy_data[504+6] = 0; dummy_data[504+7] = 12; // subtableSecret = 12
    // Subtable at 500 + 12 = 512
    dummy_data[512] = 0; dummy_data[513] = 0; // format 0
    // Map 0x28 (low byte of 0x0628) to GID 2, and 0x40 (low byte of 0x0640) to GID 1
    dummy_data[512 + 6 + 0x28] = 2;
    dummy_data[512 + 6 + 0x40] = 1; 

    // Set metric for GID 2
    dummy_data[108] = 0x03; dummy_data[109] = 0xE8; // Advance 1000 for GID 2 (100 + 2*4)

    tsfi_shaped_glyph_t shaped[32];
    float target_width = 5000.0f; // Current is ~2000 if each is 1000
    
    int out_count = tsfi_ottype_justify(&ot, cps, count, target_width, shaped, 32);
    
    printf("  Justified count: %d (Input: %d)\n", out_count, count);
    
    // Gap between index 0 and 1 should have kashidas
    // current_width = 2 * 1000 = 2000.
    // needed = 5000 - 2000 = 3000.
    // kashida width = 1000.
    // num_kashidas = 3.
    // total count = 2 (original) + 3 (extenders) = 5.
    
    assert(out_count == 5);
    
    float final_width = 0;
    for (int i=0; i<out_count; i++) final_width += shaped[i].advance_dx;
    printf("  Final width: %.2f (Target: %.2f)\n", final_width, target_width);
    assert(final_width == target_width);

    printf("[TEST] Arabic Justification passed.\n");
}

int main() {
    test_arabic_justification();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

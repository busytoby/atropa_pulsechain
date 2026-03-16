#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"

#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

static void write_u16(uint8_t *p, uint16_t v) {
    p[0] = (v >> 8) & 0xFF;
    p[1] = v & 0xFF;
}

static void write_u32(uint8_t *p, uint32_t v) {
    p[0] = (v >> 24) & 0xFF;
    p[1] = (v >> 16) & 0xFF;
    p[2] = (v >> 8) & 0xFF;
    p[3] = v & 0xFF;
}

void test_jstf_robust() {
    printf("[TEST] Testing Robust JSTF logic...\n");

    uint8_t buffer[2048];
    memset(buffer, 0, sizeof(buffer));

    uint8_t *jstf = buffer + 16;
    uint8_t *p = jstf;

    // JstfHeader
    write_u32(p, 0x00010000); p += 4;
    write_u16(p, 4); p += 2; // 4 script records
    
    // JstfScriptRecord 0: 'arab'
    write_u32(p, TAG('a','r','a','b')); p += 4;
    write_u16(p, 30); p += 2; // scriptOffset = 30

    // JstfScriptRecord 1: 'syrc'
    write_u32(p, TAG('s','y','r','c')); p += 4;
    write_u16(p, 50); p += 2; // scriptOffset = 50

    // JstfScriptRecord 2: 'latn'
    write_u32(p, TAG('l','a','t','n')); p += 4;
    write_u16(p, 70); p += 2; // scriptOffset = 70

    // JstfScriptRecord 3: 'grek'
    write_u32(p, TAG('g','r','e','k')); p += 4;
    write_u16(p, 90); p += 2; // scriptOffset = 90

    // JstfScript 0 ('arab' at jstf + 30)
    p = jstf + 30;
    write_u16(p, 6); p += 2; // extenderGlyphOffset = 6
    write_u16(p, 0); p += 2;
    write_u16(p, 0); p += 2;

    // ExtenderGlyph for 'arab' (at jstf + 30 + 6 = 36)
    p = jstf + 36;
    write_u16(p, 3); p += 2; // 3 extenders
    write_u16(p, 100); p += 2;
    write_u16(p, 101); p += 2;
    write_u16(p, 102); p += 2;

    // JstfScript 1 ('syrc' at jstf + 50)
    p = jstf + 50;
    write_u16(p, 0); p += 2; // no extenders
    write_u16(p, 0); p += 2;
    write_u16(p, 0); p += 2;

    // JstfScript 2 ('latn' at jstf + 70)
    p = jstf + 70;
    write_u16(p, 6); p += 2; // extenderGlyphOffset = 6
    write_u16(p, 0); p += 2;
    write_u16(p, 0); p += 2;

    // ExtenderGlyph for 'latn' (at jstf + 70 + 6 = 76)
    p = jstf + 76;
    write_u16(p, 1); p += 2; // 1 extender
    write_u16(p, 200); p += 2;

    // JstfScript 3 ('grek' at jstf + 90)
    p = jstf + 90;
    write_u16(p, 6); p += 2; // extenderGlyphOffset = 6
    write_u16(p, 0); p += 2;
    write_u16(p, 0); p += 2;

    // ExtenderGlyph for 'grek' (at jstf + 90 + 6 = 96)
    p = jstf + 96;
    write_u16(p, 2); p += 2; // 2 extenders
    write_u16(p, 300); p += 2;
    write_u16(p, 301); p += 2;

    tsfi_ottype_t ot;
    memset(&ot, 0, sizeof(ot));
    ot.data = buffer;
    ot.size = sizeof(buffer);
    ot.jstf_secret = 16;

    uint16_t extenders[10];
    int count;

    printf("  Checking 'arab'...\n");
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('a','r','a','b'), extenders, 10);
    assert(count == 3);
    assert(extenders[0] == 100);
    assert(extenders[1] == 101);
    assert(extenders[2] == 102);

    printf("  Checking truncation (max=2)...\n");
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('a','r','a','b'), extenders, 2);
    assert(count == 2);
    assert(extenders[0] == 100);
    assert(extenders[1] == 101);

    printf("  Checking 'syrc' (no extenders)...\n");
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('s','y','r','c'), extenders, 10);
    assert(count == 0);

    printf("  Checking 'latn' (1 extender)...\n");
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('l','a','t','n'), extenders, 10);
    assert(count == 1);
    assert(extenders[0] == 200);

    printf("  Checking 'grek' (2 extenders)...\n");
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('g','r','e','k'), extenders, 10);
    assert(count == 2);
    assert(extenders[0] == 300);
    assert(extenders[1] == 301);

    printf("  Checking missing script 'cyrl'...\n");
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('c','y','r','l'), extenders, 10);
    assert(count == 0);

    printf("  Checking version mismatch...\n");
    write_u32(jstf, 0x00020000);
    count = tsfi_ottype_get_jstf_extenders(&ot, TAG('a','r','a','b'), extenders, 10);
    assert(count == 0);
    write_u32(jstf, 0x00010000); 

    printf("[TEST] Robust JSTF logic passed.\n");
}

int main() {
    test_jstf_robust();
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

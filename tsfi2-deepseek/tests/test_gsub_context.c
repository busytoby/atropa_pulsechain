#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "tsfi_ottype.h"
#include "tsfi_font_registry.h"
#include "lau_memory.h"

#define READ_U16(p) (uint16_t)(((const uint8_t*)(p))[0] << 8 | ((const uint8_t*)(p))[1])
#define READ_U32(p) (uint32_t)((uint32_t)((const uint8_t*)(p))[0] << 24 | \
                               (uint32_t)((const uint8_t*)(p))[1] << 16 | \
                               (uint32_t)((const uint8_t*)(p))[2] << 8  | \
                               (uint32_t)((const uint8_t*)(p))[3])

#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

void test_font_gsub_context(const char *path) {
    printf("[GSUB_TEST] Probing: %s\n", path);
    FILE *f = fopen(path, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = (uint8_t *)lau_malloc(size);
    if (!data) { fclose(f); return; }
    if (fread(data, 1, size, f) != size) { fclose(f); lau_free(data); return; }
    fclose(f);

    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, data, size, 0)) {
        lau_free(data);
        return;
    }

    if (!ot.gsub_secret) {
        printf("  No GSUB table.\n");
        lau_free(data);
        return;
    }

    const uint8_t *gsub = &ot.data[ot.gsub_secret];
    uint16_t l_off = READ_U16(gsub + 8);
    const uint8_t *l_list = gsub + l_off;
    uint16_t l_count = READ_U16(l_list);

    int t6_f3_found = 0;

    for (uint16_t i = 0; i < l_count; i++) {
        const uint8_t *lookup = l_list + READ_U16(l_list + 2 + i * 2);
        uint16_t type = READ_U16(lookup);
        uint16_t sub_count = READ_U16(lookup + 4);

        for (uint16_t j = 0; j < sub_count; j++) {
            const uint8_t *subt = lookup + READ_U16(lookup + 6 + j * 2);
            uint16_t real_type = type;
            const uint8_t *real_subt = subt;
            if (type == 7) {
                real_type = READ_U16(subt + 2);
                real_subt = subt + READ_U32(subt + 4);
            }

            if (real_type == 6) {
                uint16_t format = READ_U16(real_subt);
                if (format == 3) {
                    uint16_t backtrackCount = READ_U16(real_subt + 2);
                    const uint8_t *ptr = real_subt + 4 + backtrackCount * 2;
                    uint16_t inputCount = READ_U16(ptr); 
                    
                    if (backtrackCount > 0 || inputCount > 1 || (real_subt + 4 + backtrackCount * 2 + 2 + inputCount * 2 < ot.data + ot.size && READ_U16(real_subt + 4 + backtrackCount * 2 + 2 + inputCount * 2) > 0)) {
                        t6_f3_found++;
                        uint16_t lookaheadCount = READ_U16(ptr + 2 + inputCount * 2);
                        printf("  Found Type 6 Format 3 Lookup[%d] Subtable[%d]: backtrack=%d, input=%d, lookahead=%d\n",
                               i, j, backtrackCount, inputCount, lookaheadCount);
                        
                        // Dump first input coverage
                        uint16_t first_input_cov_off = READ_U16(ptr + 2);
                        const uint8_t *cov = real_subt + first_input_cov_off;
                        uint16_t cov_fmt = READ_U16(cov);
                        if (cov_fmt == 1) {
                            printf("    Input[0] covers %d glyphs, e.g., GID %d\n", READ_U16(cov+2), READ_U16(cov + 4));
                        }

                        // Dump first lookahead coverage
                        const uint8_t *ptr_la = ptr + 2 + inputCount * 2;
                        if (lookaheadCount > 0) {
                            uint16_t first_la_cov_off = READ_U16(ptr_la + 2);
                            const uint8_t *cov_la = real_subt + first_la_cov_off;
                            uint16_t cov_fmt_la = READ_U16(cov_la);
                            if (cov_fmt_la == 1) {
                                printf("    Lookahead[0] covers %d glyphs, e.g., GID %d\n", READ_U16(cov_la + 2), READ_U16(cov_la + 4));
                            } else if (cov_fmt_la == 2) {
                                printf("    Lookahead[0] covers %d ranges, e.g., GID %d to %d\n", READ_U16(cov_la + 2), READ_U16(cov_la + 4), READ_U16(cov_la + 6));
                            }
                        }
                    }
                }
            }
        }
    }

    if (t6_f3_found == 0) {
        printf("  No Type 6 Format 3 with context found in this font.\n");
    }

    lau_free(data);
}

void test_ligature_shaping(const char *path) {
    printf("[LIGA_TEST] Testing ligatures in: %s\n", path);
    FILE *f = fopen(path, "rb");
    if (!f) return;
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = (uint8_t *)lau_malloc(size);
    if (!data) { fclose(f); return; }
    if (fread(data, 1, size, f) != size) { fclose(f); lau_free(data); return; }
    fclose(f);

    tsfi_ottype_t ot;
    if (!tsfi_ottype_init(&ot, data, size, 0)) {
        lau_free(data);
        return;
    }

    uint32_t cps[] = {'f', 'i'};
    tsfi_shaped_glyph_t shaped[4];
    int count = tsfi_ottype_shape(&ot, cps, 2, shaped, 4, NULL, 0);

    printf("  Input: 'f', 'i' (2 codepoints)\n");
    printf("  Output: %d glyph(s)\n", count);
    for (int i = 0; i < count; i++) {
        printf("    Glyph[%d]: GID=%d\n", i, shaped[i].glyph_index);
    }

    if (count == 1) {
        printf("  SUCCESS: Ligature applied.\n");
    } else {
        printf("  INFO: No ligature applied (or not present in this font for 'fi').\n");
    }

    lau_free(data);
}

int main() {
    test_font_gsub_context("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf");
    test_font_gsub_context("/usr/share/fonts/opentype/stix/STIXTwoText-Regular.otf");
    
    test_ligature_shaping("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf");
    test_ligature_shaping("/usr/share/fonts/opentype/stix/STIXTwoText-Regular.otf");
    test_ligature_shaping("/usr/share/fonts/truetype/liberation/LiberationSerif-Regular.ttf");

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

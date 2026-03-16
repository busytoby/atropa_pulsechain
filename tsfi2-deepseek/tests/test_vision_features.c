#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_ottype.h"
#include "tsfi_vision.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define TAG(a,b,c,d) ((uint32_t)(a)<<24 | (uint32_t)(b)<<16 | (uint32_t)(c)<<8 | (uint32_t)(d))

// Render a shaped string into a single buffer using STB
uint8_t* render_shaped_string(stbtt_fontinfo *stb, tsfi_shaped_glyph_t *shaped, int count, int *out_w, int *out_h) {
    float scale = stbtt_ScaleForPixelHeight(stb, 64.0f);
    int total_width = 0;
    int max_height = 64;
    
    // Calculate total width
    for (int i = 0; i < count; i++) {
        int adv, lsb;
        stbtt_GetGlyphHMetrics(stb, shaped[i].glyph_index, &adv, &lsb);
        total_width += (int)(adv * scale);
    }
    
    if (total_width == 0) total_width = 64;
    uint8_t *buffer = (uint8_t*)calloc(1, total_width * max_height);
    
    int pen_x = 0;
    for (int i = 0; i < count; i++) {
        int w, h, xoff, yoff;
        uint8_t *bmp = stbtt_GetGlyphBitmap(stb, scale, scale, shaped[i].glyph_index, &w, &h, &xoff, &yoff);
        if (bmp) {
            int baseline = max_height - 16; // rough baseline
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    int dst_x = pen_x + xoff + x;
                    int dst_y = baseline + yoff + y;
                    if (dst_x >= 0 && dst_x < total_width && dst_y >= 0 && dst_y < max_height) {
                        buffer[dst_y * total_width + dst_x] |= bmp[y * w + x];
                    }
                }
            }
            stbtt_FreeBitmap(bmp, NULL);
        }
        int adv, lsb;
        stbtt_GetGlyphHMetrics(stb, shaped[i].glyph_index, &adv, &lsb);
        pen_x += (int)(adv * scale);
    }
    
    *out_w = total_width;
    *out_h = max_height;
    return buffer;
}

void test_feature_visual(const char *font_path, const char *feat_name, uint32_t *cps, int cp_count, uint32_t tag) {
    printf("[VISION] Testing Feature: %s in %s\n", feat_name, font_path);
    FILE *f = fopen(font_path, "rb");
    if (!f) {
        printf("  [SKIP] Font not found.\n");
        return;
    }
    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *data = malloc(size);
    assert(fread(data, 1, size, f) == size);
    fclose(f);

    tsfi_ottype_t ot;
    assert(tsfi_ottype_init(&ot, data, size, 0));

    stbtt_fontinfo stb;
    assert(stbtt_InitFont(&stb, data, 0));

    // Shape WITHOUT feature (baseline)
    tsfi_shaped_glyph_t unshaped[32];
    for (int i=0; i<cp_count; i++) {
        unshaped[i].glyph_index = tsfi_ottype_get_glyph_index(&ot, cps[i], 0);
        unshaped[i].advance_dx = 0; 
    }
    
    // Shape WITH feature
    tsfi_shaped_glyph_t shaped[32];
    uint32_t opt_features[] = { tag };
    int shaped_count = tsfi_ottype_shape(&ot, cps, cp_count, shaped, 32, opt_features, 1);

    // Render both
    int w1, h1, w2, h2;
    uint8_t *img_unshaped = render_shaped_string(&stb, unshaped, cp_count, &w1, &h1);
    uint8_t *img_shaped = render_shaped_string(&stb, shaped, shaped_count, &w2, &h2);

    // If widths differ, they are visually distinct by definition.
    // If widths are the same, use TSFi Vision to do a raw comparison
    bool visual_difference = false;
    if (w1 != w2 || h1 != h2) {
        visual_difference = true;
    } else {
        // Compare raw pixel data
        visual_difference = !tsfi_vision_compare_raw(img_unshaped, img_shaped, w1 * h1);
    }

    if (visual_difference) {
        printf("  [PASS] %s caused a verified visual change via TSFi Vision.\n", feat_name);
    } else {
        printf("  [WARN] %s did NOT cause a visual change. Feature may be missing or failed.\n", feat_name);
    }

    free(img_unshaped);
    free(img_shaped);
    free(data);
}

int main() {
    uint32_t frac_cps[] = {0x31, 0x2044, 0x32}; // 1 / 2
    test_feature_visual("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf", "frac", frac_cps, 3, TAG('f','r','a','c'));

    uint32_t sups_cps[] = {0x32}; // 2
    test_feature_visual("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf", "sups", sups_cps, 1, TAG('s','u','p','s'));

    uint32_t vert_cps[] = {0x1820}; // Mongolian A
    test_feature_visual("/usr/share/fonts/truetype/noto/NotoSansMongolian-Regular.ttf", "vert", vert_cps, 1, TAG('v','e','r','t'));

    uint32_t zero_cps[] = {0x30}; // 0
    test_feature_visual("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf", "zero", zero_cps, 1, TAG('z','e','r','o'));

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

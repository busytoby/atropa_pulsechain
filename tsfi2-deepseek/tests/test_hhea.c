#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#include "tsfi_font_registry.h"
#include "tsfi_ottype.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "lau_memory.h"

int main() {
    TSFiFontRegistry *reg = lau_memalign_wired(512, sizeof(TSFiFontRegistry));
    tsfi_font_registry_init(reg);
    
    printf("[TEST] Scanning system for fonts...\n");
    tsfi_font_registry_scan_system(reg);
    printf("[TEST] Registered %d subfonts.\n", reg->count);

    if (reg->count == 0) {
        printf("No fonts found. Aborting.\n");
        lau_free(reg->bmp_cache);
        lau_free(reg->fonts);
        lau_free(reg);
        return 1;
    }

    int total_pass = 0, total_fail = 0;

    printf("Verifying hhea vertical metrics across all fonts...\n");

    for (int i = 0; i < reg->count; i++) {
        TSFiFontInfo *finfo = &reg->fonts[i];
        if (!finfo->active || !finfo->stb || !finfo->ot) continue;

        int stb_ascent, stb_descent, stb_lineGap;
        stbtt_GetFontVMetrics(finfo->stb, &stb_ascent, &stb_descent, &stb_lineGap);

        if (finfo->ot->ascent == (int16_t)stb_ascent &&
            finfo->ot->descent == (int16_t)stb_descent &&
            finfo->ot->line_gap == (int16_t)stb_lineGap) {
            total_pass++;
        } else {
            printf("  [ERR] Vertical metrics mismatch in font %s:\n", finfo->path);
            printf("        OT:  Ascent %d, Descent %d, LineGap %d\n", finfo->ot->ascent, finfo->ot->descent, finfo->ot->line_gap);
            printf("        STB: Ascent %d, Descent %d, LineGap %d\n", stb_ascent, stb_descent, stb_lineGap);
            total_fail++;
        }

        if (i < 5) {
            printf("  [INFO] %s: MinLSB %d, MinRSB %d, MaxXExtent %d\n", 
                   finfo->path, finfo->ot->min_lsb, finfo->ot->min_rsb, finfo->ot->max_x_extent);
        }
    }

    printf("\n--- HHEA VERIFICATION REPORT ---\n");
    printf("Total Fonts Passed: %d\n", total_pass);
    printf("Total Failures: %d\n", total_fail);

    int result = (total_fail == 0) ? 0 : 1;
    
    // Cleanup internal font objects
    for(int i=0; i<reg->count; i++) {
        if(reg->fonts[i].stb) lau_free(reg->fonts[i].stb);
        if(reg->fonts[i].ot) lau_free(reg->fonts[i].ot);
    }

    lau_free(reg->bmp_cache);
    lau_free(reg->fonts);
    lau_free(reg);
    return result;


    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
}


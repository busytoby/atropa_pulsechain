#include "tsfi_font_shaping.h"
#include "tsfi_ottype.h"
#include "lau_memory.h"
#include "tsfi_wiring.h"
#include "tsfi_logic.h"

// Marchen Unicode Block: 0x11C70 - 0x11CB6
#define MARCHEN_HEAD_LETTER_KA 0x11C70
#define MARCHEN_SUBJOINED_LETTER_KA 0x11CB0

// Checks if a Marchen codepoint is a subjoined consonant
static bool is_marchen_subjoined(uint32_t cp) {
    return (cp >= 0x11CB0 && cp <= 0x11CB6); // Basic heuristic for subjoiners
}

// The core Marchen geometric shaper (Thunk).
// Responsible for zero-space edge-to-edge layout of Marchen stacks.
void tsfi_shape_marchen_thunk(TSFiShapeCluster *cluster, tsfi_ottype_t *ot) {
    if (!cluster || cluster->num_codepoints <= 0) return;

    cluster->num_glyphs = cluster->num_codepoints;
    cluster->glyphs = lau_memalign_wired(512, cluster->num_glyphs * sizeof(TSFiShapedGlyph));
    
    // Wire the array of glyphs as a single block so resonance can be tracked
    LauSystemHeader *h = (LauSystemHeader *)((char *)cluster->glyphs - offsetof(LauSystemHeader, payload));
    lau_mem_scramble(&h->regs, sizeof(LauRegisterBank), 0);
    h->resonance_as_status = lau_strdup("SHAPED_GLYPH_MARCHEN");
    lau_wire_system((WaveSystem*)cluster->glyphs, h, tsfi_get_default_logic());

        float max_advance_dx = 0.0f;
        float prev_y_min_absolute = 0.0f;
        float prev_center_x = 0.0f;
        float prev_width = 0.0f;
    
        for (int i = 0; i < cluster->num_codepoints; i++) {
            uint32_t cp = cluster->codepoints[i];
            TSFiShapedGlyph *glyph = &cluster->glyphs[i];
        glyph->glyph_index = tsfi_ottype_get_glyph_index(ot, cp, 0);
        glyph->cluster_index = i;
        
        // Initialize identity transform
        glyph->transform = (tsfi_transform_t){ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

        int adv, lsb;
        tsfi_ottype_get_glyph_metrics(ot, glyph->glyph_index, &adv, &lsb);

        tsfi_glyph_geom_t geom = {0};
        if (!tsfi_ottype_get_glyph_geom(ot, glyph->glyph_index, &geom)) {
            // Fallback if no geometry
            glyph->transform.dx = 0.0f;
            glyph->transform.dy = 0.0f;
            continue;
        }

        float geom_xMin = 999999.0f;
        float geom_xMax = -999999.0f;
        float geom_yMin = 999999.0f;
        float geom_yMax = -999999.0f;
        if (geom.num_points > 0) {
            for (uint32_t j = 0; j < geom.num_points; j++) {
                float x = geom.points[j].x;
                float y = geom.points[j].y;
                if (x < geom_xMin) geom_xMin = x;
                if (x > geom_xMax) geom_xMax = x;
                if (y < geom_yMin) geom_yMin = y;
                if (y > geom_yMax) geom_yMax = y;
            }
        } else {
            geom_xMin = 0.0f; geom_xMax = 0.0f;
            geom_yMin = 0.0f; geom_yMax = 0.0f;
        }

        float current_width = geom_xMax - geom_xMin;

        if (is_marchen_subjoined(cp)) {
            float scale = 1.0f;
            if (current_width > 0.001f && prev_width > 0.001f) {
                scale = prev_width / current_width;
            }
            glyph->transform.m11 = scale;
            glyph->transform.m22 = scale;

            geom_xMin *= scale;
            geom_xMax *= scale;
            geom_yMin *= scale;
            geom_yMax *= scale;

            // First principles Marchen stacking: 
            // Subjoined characters are stacked directly underneath the previous character
            // with 0 additional space edge-to-edge.
            // We align the current glyph's top edge (yMax) exactly with the previous glyph's bottom edge (yMin).
            glyph->transform.dy = prev_y_min_absolute - geom_yMax;
            
            // Align the center X of the subjoined character to the center X of the previous character
            float sub_center_x = (geom_xMin + geom_xMax) / 2.0f;
            glyph->transform.dx = prev_center_x - sub_center_x;
        } else {
            // Base consonant
            glyph->transform.dy = 0.0f; 
            glyph->transform.dx = 0.0f;
            prev_center_x = (geom_xMin + geom_xMax) / 2.0f;
            prev_width = current_width;
            if ((float)adv > max_advance_dx) {
                max_advance_dx = (float)adv;
            }
        }

        // Update the absolute bottom edge for the next subjoined character
        prev_y_min_absolute = geom_yMin + glyph->transform.dy;

        tsfi_ottype_free_geom(&geom);
    }

    // The cluster's overall advance is determined by the widest base consonant.
    // The vertical advance remains 0 because the text line itself is horizontal.
    cluster->advance_dx = max_advance_dx;
    cluster->advance_dy = 0.0f;
}

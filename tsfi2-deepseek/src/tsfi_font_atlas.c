#include "tsfi_font_atlas.h"
#include "lau_memory.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

TsfiFontAtlas* tsfi_font_atlas_create(void) {
    TsfiFontAtlas *atlas = lau_malloc(sizeof(TsfiFontAtlas));
    atlas->data = lau_malloc_wired(ATLAS_WIDTH * ATLAS_HEIGHT);
    memset(atlas->data, 0, ATLAS_WIDTH * ATLAS_HEIGHT);
    atlas->glyph_count = 0;
    atlas->next_x = 2;
    atlas->next_y = 2;
    atlas->row_h = 0;
    return atlas;
}

void tsfi_font_atlas_destroy(TsfiFontAtlas *atlas) {
    if (!atlas) return;
    lau_free(atlas->data);
    lau_free(atlas);
}

bool tsfi_font_atlas_save(const TsfiFontAtlas *atlas, const char *path) {
    if (!atlas || !path) return false;
    FILE *f = fopen(path, "wb");
    if (!f) return false;

    // 1. Header: Atlas Metadata
    uint32_t magic = 0x54464143; // 'TFAC'
    fwrite(&magic, 4, 1, f);
    fwrite(&atlas->glyph_count, 4, 1, f);
    fwrite(&atlas->next_x, 2, 1, f);
    fwrite(&atlas->next_y, 2, 1, f);
    fwrite(&atlas->row_h, 2, 1, f);

    // 2. Glyph Metadata
    fwrite(atlas->glyphs, sizeof(TsfiAtlasGlyph), atlas->glyph_count, f);

    // 3. Atlas Bitmap Data
    fwrite(atlas->data, 1, ATLAS_WIDTH * ATLAS_HEIGHT, f);

    fclose(f);
    return true;
}

bool tsfi_font_atlas_load(TsfiFontAtlas *atlas, const char *path) {
    if (!atlas || !path) return false;
    FILE *f = fopen(path, "rb");
    if (!f) return false;

    uint32_t magic;
    if (fread(&magic, 4, 1, f) != 1 || magic != 0x54464143) {
        fclose(f); return false;
    }

    fread(&atlas->glyph_count, 4, 1, f);
    fread(&atlas->next_x, 2, 1, f);
    fread(&atlas->next_y, 2, 1, f);
    fread(&atlas->row_h, 2, 1, f);

    fread(atlas->glyphs, sizeof(TsfiAtlasGlyph), atlas->glyph_count, f);
    fread(atlas->data, 1, ATLAS_WIDTH * ATLAS_HEIGHT, f);

    fclose(f);
    return true;
}

// Simple Shelf-Packing implementation for Font Atlas
static bool atlas_add_glyph(TsfiFontAtlas *atlas, uint32_t cp, uint32_t fid, uint8_t *bits, int w, int h, int ox, int oy, float adv) {
    if (atlas->glyph_count >= MAX_GLYPHS) return false;

    // Line wrap
    if (atlas->next_x + w + 2 >= ATLAS_WIDTH) {
        atlas->next_x = 2;
        atlas->next_y += atlas->row_h + 2;
        atlas->row_h = 0;
    }

    if (atlas->next_y + h + 2 >= ATLAS_HEIGHT) return false; // Full

    // Copy bits into atlas
    for (int j = 0; j < h; j++) {
        memcpy(&atlas->data[(atlas->next_y + j) * ATLAS_WIDTH + atlas->next_x], &bits[j * w], w);
    }

    TsfiAtlasGlyph *g = &atlas->glyphs[atlas->glyph_count++];
    g->codepoint = cp;
    g->font_id = fid;
    g->x = atlas->next_x;
    g->y = atlas->next_y;
    g->w = w;
    g->h = h;
    g->offset_x = ox;
    g->offset_y = oy;
    g->advance = adv;

    atlas->next_x += w + 2;
    if (h > atlas->row_h) atlas->row_h = h;

    return true;
}

TsfiAtlasGlyph* tsfi_font_atlas_get_glyph(TsfiFontAtlas *atlas, uint32_t codepoint, uint32_t font_id) {
    // 1. Direct Search (Hot loop)
    // In production, use a hash map for cp|fid
    for (uint32_t i = 0; i < atlas->glyph_count; i++) {
        if (atlas->glyphs[i].codepoint == codepoint && atlas->glyphs[i].font_id == font_id) {
            return &atlas->glyphs[i];
        }
    }
    
    // 2. Return NULL (Missing) - Parent should rasterize and call atlas_add_glyph
    return NULL;
}

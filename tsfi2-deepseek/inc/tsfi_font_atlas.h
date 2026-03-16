#ifndef TSFI_FONT_ATLAS_H
#define TSFI_FONT_ATLAS_H

#include <stdint.h>
#include <stdbool.h>

#define ATLAS_WIDTH 2048
#define ATLAS_HEIGHT 2048
#define MAX_GLYPHS 4096

typedef struct {
    uint32_t codepoint;
    uint32_t font_id;
    uint16_t x, y;
    uint16_t w, h;
    int16_t offset_x, offset_y;
    float advance;
} TsfiAtlasGlyph;

typedef struct {
    uint8_t *data; // ATLAS_WIDTH * ATLAS_HEIGHT (A8 or L8)
    TsfiAtlasGlyph glyphs[MAX_GLYPHS];
    uint32_t glyph_count;
    uint16_t next_x, next_y;
    uint16_t row_h;
} TsfiFontAtlas;

TsfiFontAtlas* tsfi_font_atlas_create(void);
void tsfi_font_atlas_destroy(TsfiFontAtlas *atlas);

// High-speed lookup: Returns glyph metadata from atlas.
// If missing, it triggers rasterization into the atlas.
TsfiAtlasGlyph* tsfi_font_atlas_get_glyph(TsfiFontAtlas *atlas, uint32_t codepoint, uint32_t font_id);

#endif // TSFI_FONT_ATLAS_H

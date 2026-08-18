#ifndef CPM_TOMIE_RENDERMAN_PAGETURNER_PRESENTER_H
#define CPM_TOMIE_RENDERMAN_PAGETURNER_PRESENTER_H

#include "cpm_tomie_pageturner_renderman.h"
#include <stdint.h>
#include <stdbool.h>

#define RENDERMAN_FRAME_WIDTH  320
#define RENDERMAN_FRAME_HEIGHT 240

typedef struct {
    uint8_t r, g, b;
} RenderManPixelRGB;

typedef struct {
    uint32_t width;
    uint32_t height;
    RenderManPixelRGB pixels[RENDERMAN_FRAME_WIDTH * RENDERMAN_FRAME_HEIGHT];
    float z_buffer[RENDERMAN_FRAME_WIDTH * RENDERMAN_FRAME_HEIGHT];
} RenderManFrameBuffer;

void renderman_framebuffer_clear(RenderManFrameBuffer *fb, uint8_t r, uint8_t g, uint8_t b);
bool renderman_rasterize_page_mesh(RenderManFrameBuffer *fb, const PageTurnerPageMesh *pm, const PageTurnerRenderManEngine *engine);
uint32_t renderman_framebuffer_rule18_checksum(const RenderManFrameBuffer *fb);
bool renderman_save_framebuffer_dat_bin(const RenderManFrameBuffer *fb, const char *filepath);

#endif /* CPM_TOMIE_RENDERMAN_PAGETURNER_PRESENTER_H */

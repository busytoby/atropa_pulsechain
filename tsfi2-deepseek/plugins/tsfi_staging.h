#ifndef TSFI_STAGING_H
#define TSFI_STAGING_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct StagingBuffer {
    uint32_t magic;
    void *data;
    size_t size;
    size_t width;
    size_t height;
    size_t stride;
} StagingBuffer;

#define TSFI_STAGING_MAGIC 0x57414745 // "WAGE"

StagingBuffer* create_staging_buffer(size_t width, size_t height);
void destroy_staging_buffer(StagingBuffer *sb);
void staging_clear_avx512(StagingBuffer *sb, uint32_t color);
void staging_copy_avx512(StagingBuffer *dst, void *src, size_t count);
void staging_downsample_2x_avx512(StagingBuffer *dst, void *src, size_t src_w, size_t src_h);
void staging_write_direct_avx512(void *mapped_ptr, size_t row_pitch, size_t height, void *src, size_t width);
void staging_blend_over_avx512(StagingBuffer *dst, int x, int y, int w, int h, uint32_t color);
void draw_debug_codepoint(StagingBuffer *sb, int x, int y, uint32_t codepoint, uint32_t color);
void draw_debug_text(StagingBuffer *sb, int x, int y, const char *text, uint32_t color, bool forward);
void draw_rounded_rect(StagingBuffer *sb, int x, int y, int w, int h, int r, uint32_t color);
void save_staging_buffer_to_ppm(StagingBuffer *sb, const char *filename);
void render_file_modal(StagingBuffer *sb, const char *path);

#endif

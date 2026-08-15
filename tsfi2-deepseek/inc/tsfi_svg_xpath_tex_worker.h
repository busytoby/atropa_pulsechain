#ifndef TSFI_SVG_XPATH_TEX_WORKER_H
#define TSFI_SVG_XPATH_TEX_WORKER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. SVG 1.1 Direct Vector Path Engine (M, L, C, Z)
// -----------------------------------------------------------------------------
typedef enum {
    SVG_CMD_MOVETO = 0,
    SVG_CMD_LINETO,
    SVG_CMD_CUBIC_BEZIER,
    SVG_CMD_CLOSEPATH
} TsfiSvgOpCode;

typedef struct {
    TsfiSvgOpCode op;
    float x, y;
    float cx1, cy1, cx2, cy2; // Control points for cubic bezier
} TsfiSvgCommand;

typedef struct {
    TsfiSvgCommand cmds[64];
    int cmd_count;
    uint32_t fill_color;
    uint32_t stroke_color;
} TsfiSvgPath;

void tsfi_svg_init_path(TsfiSvgPath *path, uint32_t fill_col, uint32_t stroke_col);
bool tsfi_svg_parse_path_d(TsfiSvgPath *path, const char *d_str);
void tsfi_svg_rasterize_to_scanout(const TsfiSvgPath *path, uint32_t *pixels, int w, int h);

// -----------------------------------------------------------------------------
// 2. CSS Selectors & XPath 1.0 Matcher
// -----------------------------------------------------------------------------
typedef struct {
    char tag_name[32];
    char class_name[32];
    char id_name[32];
    char attr_key[32];
    char attr_val[64];
} TsfiCssSelector;

bool tsfi_css_parse_selector(const char *selector_str, TsfiCssSelector *out_sel);
bool tsfi_css_match_element(const TsfiCssSelector *sel, const char *tag, const char *classes, const char *id, const char *attr_k, const char *attr_v);

// -----------------------------------------------------------------------------
// 3. WebGL Texture Mipmapping & Bilinear Sampler
// -----------------------------------------------------------------------------
typedef struct {
    int width;
    int height;
    uint32_t *levels[8]; // Mipmap pyramid
    int level_count;
} TsfiMipmapTexture;

void tsfi_texture_init(TsfiMipmapTexture *tex, int w, int h, const uint32_t *base_pixels);
void tsfi_texture_generate_mipmaps(TsfiMipmapTexture *tex);
uint32_t tsfi_texture_sample_bilinear(const TsfiMipmapTexture *tex, int level, float u, float v);
void tsfi_texture_free(TsfiMipmapTexture *tex);

// -----------------------------------------------------------------------------
// 4. Dedicated Web Worker Thread & SharedArrayBuffer
// -----------------------------------------------------------------------------
typedef struct {
    uint32_t buffer[256];
    size_t length;
    uint32_t atomic_lock;
} TsfiSharedArrayBuffer;

typedef struct {
    TsfiSharedArrayBuffer *sab;
    bool is_running;
    char last_message[128];
} TsfiWebWorker;

void tsfi_worker_init(TsfiWebWorker *worker, TsfiSharedArrayBuffer *sab);
bool tsfi_worker_post_message(TsfiWebWorker *worker, const char *msg);
void tsfi_worker_process_task(TsfiWebWorker *worker);

#endif // TSFI_SVG_XPATH_TEX_WORKER_H

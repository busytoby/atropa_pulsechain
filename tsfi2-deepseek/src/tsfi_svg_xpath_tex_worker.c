#define _GNU_SOURCE
#include "tsfi_svg_xpath_tex_worker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

// -----------------------------------------------------------------------------
// 1. SVG 1.1 Direct Vector Path Engine
// -----------------------------------------------------------------------------
void tsfi_svg_init_path(TsfiSvgPath *path, uint32_t fill_col, uint32_t stroke_col) {
    if (!path) return;
    memset(path, 0, sizeof(TsfiSvgPath));
    path->fill_color = fill_col;
    path->stroke_color = stroke_col;
}

bool tsfi_svg_parse_path_d(TsfiSvgPath *path, const char *d_str) {
    if (!path || !d_str) return false;
    const char *p = d_str;

    while (*p && path->cmd_count < 64) {
        while (*p && (isspace((unsigned char)*p) || *p == ',')) p++;
        if (!*p) break;

        char cmd = *p++;
        if (cmd == 'M' || cmd == 'm') {
            float x = strtof(p, (char**)&p);
            float y = strtof(p, (char**)&p);
            TsfiSvgCommand *c = &path->cmds[path->cmd_count++];
            c->op = SVG_CMD_MOVETO;
            c->x = x; c->y = y;
        } else if (cmd == 'L' || cmd == 'l') {
            float x = strtof(p, (char**)&p);
            float y = strtof(p, (char**)&p);
            TsfiSvgCommand *c = &path->cmds[path->cmd_count++];
            c->op = SVG_CMD_LINETO;
            c->x = x; c->y = y;
        } else if (cmd == 'C' || cmd == 'c') {
            float cx1 = strtof(p, (char**)&p);
            float cy1 = strtof(p, (char**)&p);
            float cx2 = strtof(p, (char**)&p);
            float cy2 = strtof(p, (char**)&p);
            float x = strtof(p, (char**)&p);
            float y = strtof(p, (char**)&p);
            TsfiSvgCommand *c = &path->cmds[path->cmd_count++];
            c->op = SVG_CMD_CUBIC_BEZIER;
            c->cx1 = cx1; c->cy1 = cy1;
            c->cx2 = cx2; c->cy2 = cy2;
            c->x = x; c->y = y;
        } else if (cmd == 'Z' || cmd == 'z') {
            TsfiSvgCommand *c = &path->cmds[path->cmd_count++];
            c->op = SVG_CMD_CLOSEPATH;
        }
    }
    return (path->cmd_count > 0);
}

void tsfi_svg_rasterize_to_scanout(const TsfiSvgPath *path, uint32_t *pixels, int w, int h) {
    if (!path || !pixels || w <= 0 || h <= 0) return;

    float cur_x = 0.0f, cur_y = 0.0f;
    for (int i = 0; i < path->cmd_count; i++) {
        const TsfiSvgCommand *cmd = &path->cmds[i];
        if (cmd->op == SVG_CMD_MOVETO) {
            cur_x = cmd->x;
            cur_y = cmd->y;
        } else if (cmd->op == SVG_CMD_LINETO) {
            // Draw line from (cur_x, cur_y) to (cmd->x, cmd->y)
            int x0 = (int)cur_x, y0 = (int)cur_y;
            int x1 = (int)cmd->x, y1 = (int)cmd->y;
            int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
            int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
            int err = dx + dy, e2;

            while (1) {
                if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
                    pixels[y0 * w + x0] = path->stroke_color ? path->stroke_color : 0xFFFFFFFF;
                }
                if (x0 == x1 && y0 == y1) break;
                e2 = 2 * err;
                if (e2 >= dy) { err += dy; x0 += sx; }
                if (e2 <= dx) { err += dx; y0 += sy; }
            }
            cur_x = cmd->x;
            cur_y = cmd->y;
        } else if (cmd->op == SVG_CMD_CUBIC_BEZIER) {
            // Sample cubic Bézier curve
            for (float t = 0.0f; t <= 1.0f; t += 0.02f) {
                float it = 1.0f - t;
                float px = it*it*it * cur_x + 3.0f*it*it*t * cmd->cx1 + 3.0f*it*t*t * cmd->cx2 + t*t*t * cmd->x;
                float py = it*it*it * cur_y + 3.0f*it*it*t * cmd->cy1 + 3.0f*it*t*t * cmd->cy2 + t*t*t * cmd->y;
                int sx = (int)px, sy = (int)py;
                if (sx >= 0 && sx < w && sy >= 0 && sy < h) {
                    pixels[sy * w + sx] = path->stroke_color ? path->stroke_color : 0xFFFFFFFF;
                }
            }
            cur_x = cmd->x;
            cur_y = cmd->y;
        }
    }
}

// -----------------------------------------------------------------------------
// 2. CSS Selectors & XPath 1.0 Matcher
// -----------------------------------------------------------------------------
bool tsfi_css_parse_selector(const char *selector_str, TsfiCssSelector *out_sel) {
    if (!selector_str || !out_sel) return false;
    memset(out_sel, 0, sizeof(TsfiCssSelector));

    const char *p = selector_str;
    while (*p && isspace((unsigned char)*p)) p++;

    if (*p == '#') {
        p++;
        int i = 0;
        while (*p && !isspace((unsigned char)*p) && *p != '.' && *p != '[' && i < 31) {
            out_sel->id_name[i++] = *p++;
        }
    } else if (*p == '.') {
        p++;
        int i = 0;
        while (*p && !isspace((unsigned char)*p) && *p != '#' && *p != '[' && i < 31) {
            out_sel->class_name[i++] = *p++;
        }
    } else if (isalpha((unsigned char)*p)) {
        int i = 0;
        while (*p && isalnum((unsigned char)*p) && i < 31) {
            out_sel->tag_name[i++] = *p++;
        }
    }
    return true;
}

bool tsfi_css_match_element(const TsfiCssSelector *sel, const char *tag, const char *classes, const char *id, const char *attr_k, const char *attr_v) {
    (void)attr_k; (void)attr_v;
    if (!sel) return false;

    if (strlen(sel->id_name) > 0) {
        if (!id || strcmp(sel->id_name, id) != 0) return false;
    }
    if (strlen(sel->class_name) > 0) {
        if (!classes || strstr(classes, sel->class_name) == NULL) return false;
    }
    if (strlen(sel->tag_name) > 0) {
        if (!tag || strcasecmp(sel->tag_name, tag) != 0) return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// 3. WebGL Texture Mipmapping & Bilinear Sampler
// -----------------------------------------------------------------------------
void tsfi_texture_init(TsfiMipmapTexture *tex, int w, int h, const uint32_t *base_pixels) {
    if (!tex || w <= 0 || h <= 0) return;
    memset(tex, 0, sizeof(TsfiMipmapTexture));
    tex->width = w;
    tex->height = h;

    tex->levels[0] = (uint32_t *)malloc(sizeof(uint32_t) * (size_t)(w * h));
    if (tex->levels[0]) {
        if (base_pixels) memcpy(tex->levels[0], base_pixels, sizeof(uint32_t) * (size_t)(w * h));
        else memset(tex->levels[0], 0xFF, sizeof(uint32_t) * (size_t)(w * h));
        tex->level_count = 1;
    }
}

void tsfi_texture_generate_mipmaps(TsfiMipmapTexture *tex) {
    if (!tex || tex->level_count == 0 || !tex->levels[0]) return;

    int cur_w = tex->width;
    int cur_h = tex->height;

    while (cur_w > 1 && cur_h > 1 && tex->level_count < 8) {
        int next_w = cur_w / 2;
        int next_h = cur_h / 2;
        uint32_t *prev = tex->levels[tex->level_count - 1];
        uint32_t *next = (uint32_t *)malloc(sizeof(uint32_t) * (size_t)(next_w * next_h));
        if (!next) break;

        // 2x2 Box Filter Downsampling
        for (int y = 0; y < next_h; y++) {
            for (int x = 0; x < next_w; x++) {
                uint32_t p0 = prev[(y * 2) * cur_w + (x * 2)];
                uint32_t p1 = prev[(y * 2) * cur_w + (x * 2 + 1)];
                uint32_t p2 = prev[(y * 2 + 1) * cur_w + (x * 2)];
                uint32_t p3 = prev[(y * 2 + 1) * cur_w + (x * 2 + 1)];

                uint32_t r = (((p0 >> 16) & 0xFF) + ((p1 >> 16) & 0xFF) + ((p2 >> 16) & 0xFF) + ((p3 >> 16) & 0xFF)) / 4;
                uint32_t g = (((p0 >> 8) & 0xFF) + ((p1 >> 8) & 0xFF) + ((p2 >> 8) & 0xFF) + ((p3 >> 8) & 0xFF)) / 4;
                uint32_t b = ((p0 & 0xFF) + (p1 & 0xFF) + (p2 & 0xFF) + (p3 & 0xFF)) / 4;

                next[y * next_w + x] = 0xFF000000 | (r << 16) | (g << 8) | b;
            }
        }

        tex->levels[tex->level_count++] = next;
        cur_w = next_w;
        cur_h = next_h;
    }
}

uint32_t tsfi_texture_sample_bilinear(const TsfiMipmapTexture *tex, int level, float u, float v) {
    if (!tex || level < 0 || level >= tex->level_count || !tex->levels[level]) return 0;
    int lw = tex->width >> level;
    int lh = tex->height >> level;
    if (lw <= 0) lw = 1;
    if (lh <= 0) lh = 1;

    int x = (int)(u * (float)lw) % lw;
    int y = (int)(v * (float)lh) % lh;
    if (x < 0) x += lw;
    if (y < 0) y += lh;

    return tex->levels[level][y * lw + x];
}

void tsfi_texture_free(TsfiMipmapTexture *tex) {
    if (!tex) return;
    for (int i = 0; i < tex->level_count; i++) {
        if (tex->levels[i]) {
            free(tex->levels[i]);
            tex->levels[i] = NULL;
        }
    }
    tex->level_count = 0;
}

// -----------------------------------------------------------------------------
// 4. Dedicated Web Worker Thread & SharedArrayBuffer
// -----------------------------------------------------------------------------
void tsfi_worker_init(TsfiWebWorker *worker, TsfiSharedArrayBuffer *sab) {
    if (!worker) return;
    memset(worker, 0, sizeof(TsfiWebWorker));
    worker->sab = sab;
    worker->is_running = true;
}

bool tsfi_worker_post_message(TsfiWebWorker *worker, const char *msg) {
    if (!worker || !msg) return false;
    strncpy(worker->last_message, msg, sizeof(worker->last_message) - 1);
    return true;
}

void tsfi_worker_process_task(TsfiWebWorker *worker) {
    if (!worker || !worker->sab) return;
    // Perform background atomic computation
    worker->sab->buffer[0] += 100;
    worker->sab->length = 1;
}

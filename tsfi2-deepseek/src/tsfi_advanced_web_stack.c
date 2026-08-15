#define _GNU_SOURCE
#include "tsfi_advanced_web_stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// -----------------------------------------------------------------------------
// 1. Session History & Navigation Stack (History API)
// -----------------------------------------------------------------------------
void tsfi_history_init(TsfiBrowserHistory *hist) {
    if (!hist) return;
    memset(hist, 0, sizeof(TsfiBrowserHistory));
    hist->current_index = -1;
}

bool tsfi_history_push_state(TsfiBrowserHistory *hist, const char *url, const char *title) {
    if (!hist || !url || hist->count >= MAX_HISTORY_ENTRIES) return false;

    // Truncate any forward history on new push
    if (hist->current_index >= 0 && hist->current_index < hist->count - 1) {
        hist->count = hist->current_index + 1;
    }

    TsfiHistoryEntry *entry = &hist->entries[hist->count];
    memset(entry, 0, sizeof(TsfiHistoryEntry));
    strncpy(entry->url, url, sizeof(entry->url) - 1);
    if (title) strncpy(entry->title, title, sizeof(entry->title) - 1);
    entry->timestamp = time(NULL);

    hist->current_index = hist->count;
    hist->count++;
    return true;
}

const char* tsfi_history_back(TsfiBrowserHistory *hist) {
    if (!hist || hist->current_index <= 0) return NULL;
    hist->current_index--;
    return hist->entries[hist->current_index].url;
}

const char* tsfi_history_forward(TsfiBrowserHistory *hist) {
    if (!hist || hist->current_index >= hist->count - 1) return NULL;
    hist->current_index++;
    return hist->entries[hist->current_index].url;
}

const char* tsfi_history_current(const TsfiBrowserHistory *hist) {
    if (!hist || hist->current_index < 0 || hist->current_index >= hist->count) return NULL;
    return hist->entries[hist->current_index].url;
}

// -----------------------------------------------------------------------------
// 2. HTTP/2 Frame Multiplexer & HPACK Static Table (RFC 7540)
// -----------------------------------------------------------------------------
bool tsfi_h2_encode_frame_header(uint32_t length, uint8_t type, uint8_t flags, uint32_t stream_id, uint8_t *out_9bytes) {
    if (!out_9bytes || length > 0x00FFFFFF) return false;

    out_9bytes[0] = (uint8_t)((length >> 16) & 0xFF);
    out_9bytes[1] = (uint8_t)((length >> 8) & 0xFF);
    out_9bytes[2] = (uint8_t)(length & 0xFF);
    out_9bytes[3] = type;
    out_9bytes[4] = flags;
    out_9bytes[5] = (uint8_t)((stream_id >> 24) & 0x7F);
    out_9bytes[6] = (uint8_t)((stream_id >> 16) & 0xFF);
    out_9bytes[7] = (uint8_t)((stream_id >> 8) & 0xFF);
    out_9bytes[8] = (uint8_t)(stream_id & 0xFF);
    return true;
}

bool tsfi_h2_decode_frame(const uint8_t *in_stream, size_t in_len, TsfiHttp2Frame *frame_out, size_t *consumed_bytes) {
    if (!in_stream || !frame_out || in_len < 9) return false;
    memset(frame_out, 0, sizeof(TsfiHttp2Frame));

    frame_out->length = ((uint32_t)in_stream[0] << 16) | ((uint32_t)in_stream[1] << 8) | (uint32_t)in_stream[2];
    frame_out->type = in_stream[3];
    frame_out->flags = in_stream[4];
    frame_out->stream_id = (((uint32_t)in_stream[5] & 0x7F) << 24) |
                           ((uint32_t)in_stream[6] << 16) |
                           ((uint32_t)in_stream[7] << 8)  |
                           (uint32_t)in_stream[8];

    if (in_len < 9 + frame_out->length) return false;

    frame_out->payload = in_stream + 9;
    if (consumed_bytes) *consumed_bytes = 9 + (size_t)frame_out->length;
    return true;
}

const char* tsfi_hpack_static_lookup(int index, const char **out_val) {
    static const struct { const char *name; const char *val; } hpack_table[] = {
        {"", ""},
        {":authority", ""},
        {":method", "GET"},
        {":method", "POST"},
        {":path", "/"},
        {":path", "/index.html"},
        {":scheme", "http"},
        {":scheme", "https"},
        {":status", "200"},
        {":status", "204"},
        {":status", "206"},
        {":status", "304"},
        {":status", "400"},
        {":status", "404"},
        {":status", "500"},
        {"accept-charset", ""},
        {"accept-encoding", "gzip, deflate"},
        {"accept-language", ""},
        {"accept-ranges", ""},
        {"accept", ""},
        {"access-control-allow-origin", ""},
        {"age", ""},
        {"allow", ""},
        {"authorization", ""},
        {"cache-control", ""},
        {"content-disposition", ""},
        {"content-encoding", ""},
        {"content-language", ""},
        {"content-length", ""},
        {"content-location", ""},
        {"content-range", ""},
        {"content-type", ""}
    };
    if (index < 1 || index > 31) {
        if (out_val) *out_val = "";
        return "";
    }
    if (out_val) *out_val = hpack_table[index].val;
    return hpack_table[index].name;
}

// -----------------------------------------------------------------------------
// 3. HTML5 Canvas 2D Direct Vector Rasterizer
// -----------------------------------------------------------------------------
void tsfi_canvas_init(TsfiCanvas2DContext *ctx, int width, int height) {
    if (!ctx) return;
    memset(ctx, 0, sizeof(TsfiCanvas2DContext));
    ctx->width = (width > 0) ? width : 640;
    ctx->height = (height > 0) ? height : 480;
}

bool tsfi_canvas_fill_rect(TsfiCanvas2DContext *ctx, int x, int y, int w, int h, uint32_t color) {
    if (!ctx || ctx->count >= MAX_CANVAS_CMDS) return false;
    TsfiCanvasCmd *cmd = &ctx->commands[ctx->count++];
    cmd->type = CANVAS_CMD_FILL_RECT;
    cmd->x1 = x; cmd->y1 = y;
    cmd->x2 = x + w; cmd->y2 = y + h;
    cmd->color = color;
    return true;
}

bool tsfi_canvas_draw_line(TsfiCanvas2DContext *ctx, int x1, int y1, int x2, int y2, int line_width, uint32_t color) {
    if (!ctx || ctx->count >= MAX_CANVAS_CMDS) return false;
    TsfiCanvasCmd *cmd = &ctx->commands[ctx->count++];
    cmd->type = CANVAS_CMD_LINE_TO;
    cmd->x1 = x1; cmd->y1 = y1;
    cmd->x2 = x2; cmd->y2 = y2;
    cmd->line_width = (line_width > 0) ? line_width : 1;
    cmd->color = color;
    return true;
}

bool tsfi_canvas_draw_circle(TsfiCanvas2DContext *ctx, int cx, int cy, int radius, uint32_t color) {
    if (!ctx || ctx->count >= MAX_CANVAS_CMDS) return false;
    TsfiCanvasCmd *cmd = &ctx->commands[ctx->count++];
    cmd->type = CANVAS_CMD_CIRCLE;
    cmd->x1 = cx; cmd->y1 = cy;
    cmd->x2 = radius;
    cmd->color = color;
    return true;
}

void tsfi_canvas_rasterize(const TsfiCanvas2DContext *ctx, uint32_t *pixels, int w, int h) {
    if (!ctx || !pixels || w <= 0 || h <= 0) return;

    for (int i = 0; i < ctx->count; i++) {
        const TsfiCanvasCmd *cmd = &ctx->commands[i];
        if (cmd->type == CANVAS_CMD_FILL_RECT) {
            for (int y = cmd->y1; y < cmd->y2 && y < h; y++) {
                if (y < 0) continue;
                uint32_t *row = pixels + y * w;
                for (int x = cmd->x1; x < cmd->x2 && x < w; x++) {
                    if (x < 0) continue;
                    row[x] = cmd->color;
                }
            }
        } else if (cmd->type == CANVAS_CMD_CIRCLE) {
            int cx = cmd->x1;
            int cy = cmd->y1;
            int r = cmd->x2;
            int r_sq = r * r;
            for (int y = cy - r; y <= cy + r && y < h; y++) {
                if (y < 0) continue;
                uint32_t *row = pixels + y * w;
                for (int x = cx - r; x <= cx + r && x < w; x++) {
                    if (x < 0) continue;
                    int dx = x - cx;
                    int dy = y - cy;
                    if (dx * dx + dy * dy <= r_sq) {
                        row[x] = cmd->color;
                    }
                }
            }
        }
 else if (cmd->type == CANVAS_CMD_LINE_TO) {
            // Bresenham line algorithm
            int x0 = cmd->x1, y0 = cmd->y1;
            int x1 = cmd->x2, y1 = cmd->y2;
            int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
            int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
            int err = dx + dy, e2;

            while (1) {
                if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
                    pixels[y0 * w + x0] = cmd->color;
                }
                if (x0 == x1 && y0 == y1) break;
                e2 = 2 * err;
                if (e2 >= dy) { err += dy; x0 += sx; }
                if (e2 <= dx) { err += dx; y0 += sy; }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// 4. Multipart MIME & Blob Storage
// -----------------------------------------------------------------------------
void tsfi_multipart_init(TsfiMultipartFormData *form, const char *boundary) {
    if (!form) return;
    memset(form, 0, sizeof(TsfiMultipartFormData));
    if (boundary) strncpy(form->boundary, boundary, sizeof(form->boundary) - 1);
    else strncpy(form->boundary, "----TsfiFormBoundary953467954114363", sizeof(form->boundary) - 1);
}

bool tsfi_multipart_add_part(TsfiMultipartFormData *form, const char *name, const char *filename, const char *content_type, const void *data, size_t len) {
    if (!form || !name || !data || len == 0 || form->count >= MAX_BLOB_PARTS) return false;

    TsfiMultipartPart *part = &form->parts[form->count++];
    memset(part, 0, sizeof(TsfiMultipartPart));
    strncpy(part->name, name, sizeof(part->name) - 1);
    if (filename) strncpy(part->filename, filename, sizeof(part->filename) - 1);
    if (content_type) strncpy(part->content_type, content_type, sizeof(part->content_type) - 1);
    else strncpy(part->content_type, "application/octet-stream", sizeof(part->content_type) - 1);

    part->data = (uint8_t *)malloc(len);
    if (!part->data) return false;
    memcpy(part->data, data, len);
    part->data_len = len;
    return true;
}

size_t tsfi_multipart_serialize(const TsfiMultipartFormData *form, uint8_t *out_buf, size_t max_len) {
    if (!form || !out_buf || max_len == 0) return 0;
    size_t offset = 0;

    for (int i = 0; i < form->count; i++) {
        const TsfiMultipartPart *p = &form->parts[i];
        char header[512];
        int h_len = 0;
        if (strlen(p->filename) > 0) {
            h_len = snprintf(header, sizeof(header),
                             "--%s\r\nContent-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Type: %s\r\n\r\n",
                             form->boundary, p->name, p->filename, p->content_type);
        } else {
            h_len = snprintf(header, sizeof(header),
                             "--%s\r\nContent-Disposition: form-data; name=\"%s\"\r\n\r\n",
                             form->boundary, p->name);
        }

        if (offset + (size_t)h_len + p->data_len + 2 >= max_len) break;
        memcpy(out_buf + offset, header, (size_t)h_len);
        offset += (size_t)h_len;

        memcpy(out_buf + offset, p->data, p->data_len);
        offset += p->data_len;

        memcpy(out_buf + offset, "\r\n", 2);
        offset += 2;
    }

    char footer[128];
    int f_len = snprintf(footer, sizeof(footer), "--%s--\r\n", form->boundary);
    if (offset + (size_t)f_len < max_len) {
        memcpy(out_buf + offset, footer, (size_t)f_len);
        offset += (size_t)f_len;
    }

    return offset;
}

void tsfi_multipart_free(TsfiMultipartFormData *form) {
    if (!form) return;
    for (int i = 0; i < form->count; i++) {
        if (form->parts[i].data) {
            free(form->parts[i].data);
            form->parts[i].data = NULL;
        }
    }
    form->count = 0;
}

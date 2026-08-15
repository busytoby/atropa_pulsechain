#ifndef TSFI_ADVANCED_WEB_STACK_H
#define TSFI_ADVANCED_WEB_STACK_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#define MAX_HISTORY_ENTRIES 64
#define MAX_CANVAS_CMDS 128
#define MAX_BLOB_PARTS 16

// -----------------------------------------------------------------------------
// 1. Session History & Navigation Stack (History API)
// -----------------------------------------------------------------------------
typedef struct {
    char url[512];
    char title[128];
    time_t timestamp;
} TsfiHistoryEntry;

typedef struct {
    TsfiHistoryEntry entries[MAX_HISTORY_ENTRIES];
    int current_index;
    int count;
} TsfiBrowserHistory;

void tsfi_history_init(TsfiBrowserHistory *hist);
bool tsfi_history_push_state(TsfiBrowserHistory *hist, const char *url, const char *title);
const char* tsfi_history_back(TsfiBrowserHistory *hist);
const char* tsfi_history_forward(TsfiBrowserHistory *hist);
const char* tsfi_history_current(const TsfiBrowserHistory *hist);

// -----------------------------------------------------------------------------
// 2. HTTP/2 Frame Multiplexer & HPACK Static Table (RFC 7540)
// -----------------------------------------------------------------------------
typedef struct {
    uint32_t length;
    uint8_t type;     // 0=DATA, 1=HEADERS, 3=RST_STREAM, 4=SETTINGS, 8=WINDOW_UPDATE
    uint8_t flags;    // 0x1=END_STREAM, 0x4=END_HEADERS, 0x1=ACK
    uint32_t stream_id;
    const uint8_t *payload;
} TsfiHttp2Frame;

bool tsfi_h2_encode_frame_header(uint32_t length, uint8_t type, uint8_t flags, uint32_t stream_id, uint8_t *out_9bytes);
bool tsfi_h2_decode_frame(const uint8_t *in_stream, size_t in_len, TsfiHttp2Frame *frame_out, size_t *consumed_bytes);
const char* tsfi_hpack_static_lookup(int index, const char **out_val);

// -----------------------------------------------------------------------------
// 3. HTML5 Canvas 2D Direct Vector Rasterizer
// -----------------------------------------------------------------------------
typedef enum {
    CANVAS_CMD_FILL_RECT,
    CANVAS_CMD_STROKE_RECT,
    CANVAS_CMD_LINE_TO,
    CANVAS_CMD_CIRCLE,
    CANVAS_CMD_CLEAR_RECT
} TsfiCanvasOpType;

typedef struct {
    TsfiCanvasOpType type;
    int x1, y1, x2, y2;
    uint32_t color;
    int line_width;
} TsfiCanvasCmd;

typedef struct {
    TsfiCanvasCmd commands[MAX_CANVAS_CMDS];
    int count;
    int width;
    int height;
} TsfiCanvas2DContext;

void tsfi_canvas_init(TsfiCanvas2DContext *ctx, int width, int height);
bool tsfi_canvas_fill_rect(TsfiCanvas2DContext *ctx, int x, int y, int w, int h, uint32_t color);
bool tsfi_canvas_draw_line(TsfiCanvas2DContext *ctx, int x1, int y1, int x2, int y2, int line_width, uint32_t color);
bool tsfi_canvas_draw_circle(TsfiCanvas2DContext *ctx, int cx, int cy, int radius, uint32_t color);
void tsfi_canvas_rasterize(const TsfiCanvas2DContext *ctx, uint32_t *pixels, int w, int h);

// -----------------------------------------------------------------------------
// 4. Multipart MIME & Blob Storage (`multipart/form-data`)
// -----------------------------------------------------------------------------
typedef struct {
    char name[64];
    char filename[128];
    char content_type[64];
    uint8_t *data;
    size_t data_len;
} TsfiMultipartPart;

typedef struct {
    char boundary[64];
    TsfiMultipartPart parts[MAX_BLOB_PARTS];
    int count;
} TsfiMultipartFormData;

void tsfi_multipart_init(TsfiMultipartFormData *form, const char *boundary);
bool tsfi_multipart_add_part(TsfiMultipartFormData *form, const char *name, const char *filename, const char *content_type, const void *data, size_t len);
size_t tsfi_multipart_serialize(const TsfiMultipartFormData *form, uint8_t *out_buf, size_t max_len);
void tsfi_multipart_free(TsfiMultipartFormData *form);

#endif // TSFI_ADVANCED_WEB_STACK_H

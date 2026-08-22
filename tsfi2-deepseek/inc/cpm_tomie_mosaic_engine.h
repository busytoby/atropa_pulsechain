#ifndef CPM_TOMIE_MOSAIC_ENGINE_H
#define CPM_TOMIE_MOSAIC_ENGINE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MOSAIC_MAX_URL_LEN 256
#define MOSAIC_MAX_TITLE_LEN 128
#define MOSAIC_MAX_LINKS 64
#define MOSAIC_MAX_DOM_NODES 256

typedef enum {
    MOSAIC_NODE_ROOT = 0,
    MOSAIC_NODE_HEAD = 1,
    MOSAIC_NODE_TITLE = 2,
    MOSAIC_NODE_BODY = 3,
    MOSAIC_NODE_HEADING = 4,
    MOSAIC_NODE_PARAGRAPH = 5,
    MOSAIC_NODE_ANCHOR = 6,
    MOSAIC_NODE_TEXT = 7,
    MOSAIC_NODE_IMAGE = 8,
    MOSAIC_NODE_LIST_ITEM = 9,
    MOSAIC_NODE_GOPHER_ITEM = 10
} MosaicNodeType;

#define MOSAIC_MAX_HISTORY 16

typedef struct {
    MosaicNodeType type;
    char text[128];
    char href[MOSAIC_MAX_URL_LEN];
    char src[MOSAIC_MAX_URL_LEN];
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    uint8_t inline_bitmap[64]; /* 8x8 1-bit or 16-nibble icon */
} MosaicDomNode;

typedef struct {
    char current_url[MOSAIC_MAX_URL_LEN];
    char page_title[MOSAIC_MAX_TITLE_LEN];
    char history_stack[MOSAIC_MAX_HISTORY][MOSAIC_MAX_URL_LEN];
    size_t history_top;
    MosaicDomNode dom_nodes[MOSAIC_MAX_DOM_NODES];
    size_t dom_node_count;
    uint16_t viewport_width;
    uint16_t viewport_height;
    uint16_t scroll_y;
    uint32_t active_link_count;
    bool is_page_rendered;
} CpmTomieMosaicContext;

int cpm_tomie_mosaic_init(CpmTomieMosaicContext *ctx, uint16_t width, uint16_t height);
int cpm_tomie_mosaic_parse_html(CpmTomieMosaicContext *ctx, const char *html_content, size_t length);
int cpm_tomie_mosaic_parse_gopher(CpmTomieMosaicContext *ctx, const char *gopher_content, size_t length);
int cpm_tomie_mosaic_layout(CpmTomieMosaicContext *ctx);
int cpm_tomie_mosaic_navigate(CpmTomieMosaicContext *ctx, const char *url);
int cpm_tomie_mosaic_history_back(CpmTomieMosaicContext *ctx);
int cpm_tomie_mosaic_inflate_raw_stream(const uint8_t *in, size_t in_len, uint8_t *out, size_t *out_len);

#ifdef __cplusplus
}
#endif

#endif /* CPM_TOMIE_MOSAIC_ENGINE_H */

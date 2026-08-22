#include "cpm_tomie_mosaic_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cpm_tomie_mosaic_init(CpmTomieMosaicContext *ctx, uint16_t width, uint16_t height) {
    if (!ctx) return -1;
    memset(ctx, 0, sizeof(CpmTomieMosaicContext));

    ctx->viewport_width = (width > 0) ? width : 640;
    ctx->viewport_height = (height > 0) ? height : 480;
    ctx->scroll_y = 0;
    ctx->dom_node_count = 0;
    ctx->active_link_count = 0;
    ctx->is_page_rendered = false;

    strncpy(ctx->current_url, "about:blank", sizeof(ctx->current_url) - 1);
    strncpy(ctx->page_title, "CPMTomie Mosaic Browser", sizeof(ctx->page_title) - 1);
    return 0;
}

int cpm_tomie_mosaic_parse_html(CpmTomieMosaicContext *ctx, const char *html_content, size_t length) {
    if (!ctx || !html_content || length == 0) return -1;

    ctx->dom_node_count = 0;
    ctx->active_link_count = 0;

    const char *ptr = html_content;
    const char *end = html_content + length;

    /* Simple native DOM parser enforcing Rule 19 (Zero RDF / Direct DOM Nodes) */
    while (ptr < end && ctx->dom_node_count < MOSAIC_MAX_DOM_NODES) {
        if (*ptr == '<') {
            const char *tag_start = ptr + 1;
            const char *tag_end = strchr(tag_start, '>');
            if (!tag_end || tag_end >= end) break;

            if (strncmp(tag_start, "title>", 6) == 0) {
                const char *title_start = tag_end + 1;
                const char *title_close = strstr(title_start, "</title>");
                if (title_close && title_close < end) {
                    size_t tlen = title_close - title_start;
                    if (tlen >= sizeof(ctx->page_title)) tlen = sizeof(ctx->page_title) - 1;
                    strncpy(ctx->page_title, title_start, tlen);
                    ctx->page_title[tlen] = '\0';
                }
            } else if (strncmp(tag_start, "h1>", 3) == 0 || strncmp(tag_start, "h2>", 3) == 0) {
                const char *h_start = tag_end + 1;
                const char *h_close = strstr(h_start, "</h");
                if (h_close && h_close < end) {
                    MosaicDomNode *node = &ctx->dom_nodes[ctx->dom_node_count++];
                    node->type = MOSAIC_NODE_HEADING;
                    size_t hlen = h_close - h_start;
                    if (hlen >= sizeof(node->text)) hlen = sizeof(node->text) - 1;
                    strncpy(node->text, h_start, hlen);
                    node->text[hlen] = '\0';
                }
            } else if (strncmp(tag_start, "p>", 2) == 0) {
                const char *p_start = tag_end + 1;
                const char *p_close = strstr(p_start, "</p>");
                if (p_close && p_close < end) {
                    MosaicDomNode *node = &ctx->dom_nodes[ctx->dom_node_count++];
                    node->type = MOSAIC_NODE_PARAGRAPH;
                    size_t plen = p_close - p_start;
                    if (plen >= sizeof(node->text)) plen = sizeof(node->text) - 1;
                    strncpy(node->text, p_start, plen);
                    node->text[plen] = '\0';
                }
            } else if (strncmp(tag_start, "a href=\"", 8) == 0) {
                const char *href_start = tag_start + 8;
                const char *href_end = strchr(href_start, '\"');
                if (href_end && href_end < tag_end) {
                    const char *a_text_start = tag_end + 1;
                    const char *a_close = strstr(a_text_start, "</a>");
                    if (a_close && a_close < end) {
                        MosaicDomNode *node = &ctx->dom_nodes[ctx->dom_node_count++];
                        node->type = MOSAIC_NODE_ANCHOR;

                        size_t hlen = href_end - href_start;
                        if (hlen >= sizeof(node->href)) hlen = sizeof(node->href) - 1;
                        strncpy(node->href, href_start, hlen);
                        node->href[hlen] = '\0';

                        size_t alen = a_close - a_text_start;
                        if (alen >= sizeof(node->text)) alen = sizeof(node->text) - 1;
                        strncpy(node->text, a_text_start, alen);
                        node->text[alen] = '\0';

                        ctx->active_link_count++;
                    }
                }
            }
            ptr = tag_end + 1;
        } else {
            ptr++;
        }
    }

    return cpm_tomie_mosaic_layout(ctx);
}

int cpm_tomie_mosaic_layout(CpmTomieMosaicContext *ctx) {
    if (!ctx) return -1;

    uint16_t current_y = 20;
    const uint16_t margin_x = 16;
    const uint16_t line_height_h = 24;
    const uint16_t line_height_p = 16;

    for (size_t i = 0; i < ctx->dom_node_count; ++i) {
        MosaicDomNode *node = &ctx->dom_nodes[i];
        node->x = margin_x;
        node->y = current_y;
        node->width = ctx->viewport_width - (margin_x * 2);

        if (node->type == MOSAIC_NODE_HEADING) {
            node->height = line_height_h;
            current_y += line_height_h + 8;
        } else if (node->type == MOSAIC_NODE_PARAGRAPH) {
            node->height = line_height_p;
            current_y += line_height_p + 4;
        } else if (node->type == MOSAIC_NODE_ANCHOR) {
            node->height = line_height_p;
            current_y += line_height_p + 4;
        } else {
            node->height = line_height_p;
            current_y += line_height_p + 2;
        }
    }

    ctx->is_page_rendered = true;
    return 0;
}

int cpm_tomie_mosaic_navigate(CpmTomieMosaicContext *ctx, const char *url) {
    if (!ctx || !url) return -1;

    strncpy(ctx->current_url, url, sizeof(ctx->current_url) - 1);
    ctx->current_url[sizeof(ctx->current_url) - 1] = '\0';

    /* Synthetic default template for navigation */
    const char *sample_html = "<html><head><title>CPMTomie VSEn Gateway</title></head>"
                              "<body><h1>Welcome to CPMTomie Mosaic</h1>"
                              "<p>Connected over VSEn IPv4 socket at 192.42.100.42.</p>"
                              "<a href=\"http://192.42.100.20/search\">Query Mu LLM Apex</a>"
                              "</body></html>";

    return cpm_tomie_mosaic_parse_html(ctx, sample_html, strlen(sample_html));
}

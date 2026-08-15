#define _GNU_SOURCE
#include "tsfi_tls13_media_rdf_dnd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// -----------------------------------------------------------------------------
// 1. TLS 1.3 0-RTT Session Resumption & PSK Store
// -----------------------------------------------------------------------------
void tsfi_tls13_store_init(TsfiTls13SessionStore *store) {
    if (!store) return;
    memset(store, 0, sizeof(TsfiTls13SessionStore));
}

bool tsfi_tls13_save_ticket(TsfiTls13SessionStore *store, const char *hostname, const uint8_t *ticket, size_t tlen, const uint8_t *psk) {
    if (!store || !hostname || !ticket || tlen == 0 || tlen > 128 || !psk || store->ticket_count >= 16) return false;

    TsfiTls13SessionTicket *t = &store->tickets[store->ticket_count++];
    strncpy(t->hostname, hostname, sizeof(t->hostname) - 1);
    memcpy(t->session_ticket, ticket, tlen);
    t->ticket_len = tlen;
    memcpy(t->psk_secret, psk, 32);
    t->ticket_lifetime_sec = 86400; // 24 hours
    t->has_early_data = true;
    return true;
}

const TsfiTls13SessionTicket* tsfi_tls13_find_ticket(const TsfiTls13SessionStore *store, const char *hostname) {
    if (!store || !hostname) return NULL;
    for (int i = 0; i < store->ticket_count; i++) {
        if (strcmp(store->tickets[i].hostname, hostname) == 0) {
            return &store->tickets[i];
        }
    }
    return NULL;
}

// -----------------------------------------------------------------------------
// 2. CSS Media Queries & Responsive Breakpoint Engine
// -----------------------------------------------------------------------------
void tsfi_media_query_init(TsfiMediaQuery *mq) {
    if (!mq) return;
    memset(mq, 0, sizeof(TsfiMediaQuery));
    mq->max_width = 999999;
    mq->max_height = 999999;
}

bool tsfi_media_query_parse(TsfiMediaQuery *mq, const char *query_str) {
    if (!mq || !query_str) return false;
    tsfi_media_query_init(mq);

    char buf[256];
    strncpy(buf, query_str, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    if (strstr(buf, "prefers-color-scheme: dark")) {
        mq->prefers_dark = true;
    }

    const char *mw = strstr(buf, "max-width:");
    if (mw) {
        mw += 10;
        while (*mw && isspace((unsigned char)*mw)) mw++;
        mq->max_width = atoi(mw);
    }

    const char *min_w = strstr(buf, "min-width:");
    if (min_w) {
        min_w += 10;
        while (*min_w && isspace((unsigned char)*min_w)) min_w++;
        mq->min_width = atoi(min_w);
    }

    return true;
}

bool tsfi_media_query_eval(const TsfiMediaQuery *mq, int viewport_w, int viewport_h, bool is_dark) {
    (void)viewport_h;
    if (!mq) return false;
    if (viewport_w < mq->min_width || viewport_w > mq->max_width) return false;
    if (mq->prefers_dark && !is_dark) return false;
    return true;
}

// -----------------------------------------------------------------------------
// 3. Native HTML5 Meta Tags & Document Title Key-Value Extractor
// -----------------------------------------------------------------------------
void tsfi_html_metadata_init(TsfiHtmlDocumentMetadata *meta) {
    if (!meta) return;
    memset(meta, 0, sizeof(TsfiHtmlDocumentMetadata));
}

bool tsfi_html_metadata_extract(TsfiHtmlDocumentMetadata *meta, const char *html_str) {
    if (!meta || !html_str) return false;

    // Extract <title>...</title>
    const char *t_start = strstr(html_str, "<title>");
    if (t_start) {
        t_start += 7;
        const char *t_end = strstr(t_start, "</title>");
        if (t_end) {
            size_t t_len = (size_t)(t_end - t_start);
            if (t_len >= sizeof(meta->document_title)) t_len = sizeof(meta->document_title) - 1;
            strncpy(meta->document_title, t_start, t_len);
            meta->document_title[t_len] = '\0';
        }
    }

    // Extract <meta name="..." content="...">
    const char *m_pos = html_str;
    while ((m_pos = strstr(m_pos, "<meta ")) != NULL && meta->meta_count < 16) {
        m_pos += 6;
        const char *name_pos = strstr(m_pos, "name=\"");
        const char *content_pos = strstr(m_pos, "content=\"");
        if (name_pos && content_pos) {
            name_pos += 6;
            char m_name[64] = {0};
            int ni = 0;
            while (*name_pos && *name_pos != '\"' && ni < 63) {
                m_name[ni++] = *name_pos++;
            }

            content_pos += 9;
            char m_content[256] = {0};
            int ci = 0;
            while (*content_pos && *content_pos != '\"' && ci < 255) {
                m_content[ci++] = *content_pos++;
            }

            TsfiHtmlMetaTag *tag = &meta->meta_tags[meta->meta_count++];
            snprintf(tag->name, sizeof(tag->name), "%s", m_name);
            snprintf(tag->content, sizeof(tag->content), "%s", m_content);

        }
    }

    return (strlen(meta->document_title) > 0 || meta->meta_count > 0);
}

const char* tsfi_html_metadata_get(const TsfiHtmlDocumentMetadata *meta, const char *name) {
    if (!meta || !name) return NULL;
    for (int i = 0; i < meta->meta_count; i++) {
        if (strcasecmp(meta->meta_tags[i].name, name) == 0) {
            return meta->meta_tags[i].content;
        }
    }
    return NULL;
}

// -----------------------------------------------------------------------------
// 4. DOM Drag-and-Drop & Clipboard DataTransfer API
// -----------------------------------------------------------------------------
void tsfi_datatransfer_init(TsfiDataTransfer *dt) {
    if (!dt) return;
    memset(dt, 0, sizeof(TsfiDataTransfer));
    strncpy(dt->drop_effect, "copy", sizeof(dt->drop_effect) - 1);
}

bool tsfi_datatransfer_set_data(TsfiDataTransfer *dt, const char *mime, const char *data) {
    if (!dt || !mime || !data || dt->item_count >= 8) return false;
    for (int i = 0; i < dt->item_count; i++) {
        if (strcmp(dt->items[i].mime_type, mime) == 0) {
            strncpy(dt->items[i].data, data, sizeof(dt->items[i].data) - 1);
            return true;
        }
    }
    TsfiDataTransferItem *item = &dt->items[dt->item_count++];
    strncpy(item->mime_type, mime, sizeof(item->mime_type) - 1);
    strncpy(item->data, data, sizeof(item->data) - 1);
    return true;
}

const char* tsfi_datatransfer_get_data(const TsfiDataTransfer *dt, const char *mime) {
    if (!dt || !mime) return NULL;
    for (int i = 0; i < dt->item_count; i++) {
        if (strcmp(dt->items[i].mime_type, mime) == 0) {
            return dt->items[i].data;
        }
    }
    return NULL;
}

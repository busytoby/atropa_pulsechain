#ifndef TSFI_TLS13_MEDIA_HTML_DND_H
#define TSFI_TLS13_MEDIA_HTML_DND_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. TLS 1.3 0-RTT Session Resumption & PSK Store (RFC 8446)
// -----------------------------------------------------------------------------
typedef struct {
    char hostname[128];
    uint8_t session_ticket[128];
    size_t ticket_len;
    uint8_t psk_secret[32];
    uint32_t ticket_lifetime_sec;
    bool has_early_data;
} TsfiTls13SessionTicket;

typedef struct {
    TsfiTls13SessionTicket tickets[16];
    int ticket_count;
} TsfiTls13SessionStore;

void tsfi_tls13_store_init(TsfiTls13SessionStore *store);
bool tsfi_tls13_save_ticket(TsfiTls13SessionStore *store, const char *hostname, const uint8_t *ticket, size_t tlen, const uint8_t *psk);
const TsfiTls13SessionTicket* tsfi_tls13_find_ticket(const TsfiTls13SessionStore *store, const char *hostname);

// -----------------------------------------------------------------------------
// 2. CSS Media Queries & Responsive Breakpoint Engine
// -----------------------------------------------------------------------------
typedef struct {
    int min_width;
    int max_width;
    int min_height;
    int max_height;
    bool prefers_dark;
} TsfiMediaQuery;

void tsfi_media_query_init(TsfiMediaQuery *mq);
bool tsfi_media_query_parse(TsfiMediaQuery *mq, const char *query_str);
bool tsfi_media_query_eval(const TsfiMediaQuery *mq, int viewport_w, int viewport_h, bool is_dark);

// -----------------------------------------------------------------------------
// 3. Native HTML5 Meta Tags & Document Title Key-Value Extractor
// -----------------------------------------------------------------------------
typedef struct {
    char name[64];
    char content[256];
} TsfiHtmlMetaTag;

typedef struct {
    char document_title[128];
    TsfiHtmlMetaTag meta_tags[16];
    int meta_count;
} TsfiHtmlDocumentMetadata;

void tsfi_html_metadata_init(TsfiHtmlDocumentMetadata *meta);
bool tsfi_html_metadata_extract(TsfiHtmlDocumentMetadata *meta, const char *html_str);
const char* tsfi_html_metadata_get(const TsfiHtmlDocumentMetadata *meta, const char *name);

// -----------------------------------------------------------------------------
// 4. DOM Drag-and-Drop & Clipboard DataTransfer API
// -----------------------------------------------------------------------------
typedef struct {
    char mime_type[64];
    char data[512];
} TsfiDataTransferItem;

typedef struct {
    TsfiDataTransferItem items[8];
    int item_count;
    char drop_effect[16]; // "none", "copy", "move"
} TsfiDataTransfer;

void tsfi_datatransfer_init(TsfiDataTransfer *dt);
bool tsfi_datatransfer_set_data(TsfiDataTransfer *dt, const char *mime, const char *data);
const char* tsfi_datatransfer_get_data(const TsfiDataTransfer *dt, const char *mime);

#endif // TSFI_TLS13_MEDIA_HTML_DND_H

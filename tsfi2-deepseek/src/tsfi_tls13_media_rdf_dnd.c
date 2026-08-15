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
// 3. JSON-LD & Semantic Microdata RDF Triples Extractor
// -----------------------------------------------------------------------------
void tsfi_rdf_graph_init(TsfiRdfGraph *graph) {
    if (!graph) return;
    memset(graph, 0, sizeof(TsfiRdfGraph));
}

bool tsfi_rdf_extract_jsonld(TsfiRdfGraph *graph, const char *jsonld_str, const char *default_subject) {
    if (!graph || !jsonld_str || graph->count >= 32) return false;

    const char *subj = default_subject ? default_subject : "urn:dysnomia:node";

    // Simple schema extraction for @type, name, description
    const char *name_pos = strstr(jsonld_str, "\"name\":");
    if (name_pos && graph->count < 32) {
        name_pos += 7;
        while (*name_pos && (*name_pos == ' ' || *name_pos == '\"')) name_pos++;
        char val[128] = {0};
        int vi = 0;
        while (*name_pos && *name_pos != '\"' && *name_pos != ',' && vi < 127) {
            val[vi++] = *name_pos++;
        }
        TsfiRdfTriple *t = &graph->triples[graph->count++];
        strncpy(t->subject, subj, sizeof(t->subject) - 1);
        strncpy(t->predicate, "schema:name", sizeof(t->predicate) - 1);
        strncpy(t->object, val, sizeof(t->object) - 1);
    }

    const char *type_pos = strstr(jsonld_str, "\"@type\":");
    if (type_pos && graph->count < 32) {
        type_pos += 8;
        while (*type_pos && (*type_pos == ' ' || *type_pos == '\"')) type_pos++;
        char val[128] = {0};
        int vi = 0;
        while (*type_pos && *type_pos != '\"' && *type_pos != ',' && vi < 127) {
            val[vi++] = *type_pos++;
        }
        TsfiRdfTriple *t = &graph->triples[graph->count++];
        strncpy(t->subject, subj, sizeof(t->subject) - 1);
        strncpy(t->predicate, "rdf:type", sizeof(t->predicate) - 1);
        strncpy(t->object, val, sizeof(t->object) - 1);
    }

    return (graph->count > 0);
}

const char* tsfi_rdf_query_predicate(const TsfiRdfGraph *graph, const char *predicate) {
    if (!graph || !predicate) return NULL;
    for (int i = 0; i < graph->count; i++) {
        if (strcmp(graph->triples[i].predicate, predicate) == 0) {
            return graph->triples[i].object;
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

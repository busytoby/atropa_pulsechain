#define _GNU_SOURCE
#include "tsfi_webrtc_sw_idb_flexgrid.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// 1. WebRTC & DTLS-SRTP Media Channel
// -----------------------------------------------------------------------------
void tsfi_webrtc_init(TsfiWebRtcPeer *peer, uint32_t ssrc) {
    if (!peer) return;
    memset(peer, 0, sizeof(TsfiWebRtcPeer));
    peer->ssrc = (ssrc > 0) ? ssrc : 0x953467;
    peer->sequence_number = 1;
}

bool tsfi_webrtc_create_offer(TsfiWebRtcPeer *peer, const char *ufrag, const char *pwd) {
    if (!peer) return false;
    snprintf(peer->sdp_offer, sizeof(peer->sdp_offer),
             "v=0\r\no=- %u 2 IN IP4 127.0.0.1\r\ns=-\r\nt=0 0\r\n"
             "a=ice-ufrag:%s\r\na=ice-pwd:%s\r\n"
             "m=audio 9 UDP/TLS/RTP/SAVPF 111\r\n"
             "a=rtpmap:111 opus/48000/2\r\n",
             peer->ssrc, ufrag ? ufrag : "tsfi_ufrag", pwd ? pwd : "tsfi_ice_pwd");
    return true;
}

bool tsfi_webrtc_set_remote_answer(TsfiWebRtcPeer *peer, const char *remote_answer) {
    if (!peer || !remote_answer) return false;
    strncpy(peer->sdp_answer, remote_answer, sizeof(peer->sdp_answer) - 1);
    peer->is_dtls_connected = true;
    return true;
}

size_t tsfi_webrtc_encrypt_srtp_packet(TsfiWebRtcPeer *peer, const uint8_t *payload, size_t len, uint8_t *out_pkt, size_t max_out) {
    if (!peer || !payload || len == 0 || !out_pkt || max_out < 12 + len + 4) return 0;

    size_t offset = 0;
    // 12-byte RTP Header (RFC 3550 / RFC 3711)
    out_pkt[offset++] = 0x80; // V=2, P=0, X=0, CC=0
    out_pkt[offset++] = 111;  // Payload Type (Opus)
    out_pkt[offset++] = (uint8_t)((peer->sequence_number >> 8) & 0xFF);
    out_pkt[offset++] = (uint8_t)(peer->sequence_number & 0xFF);
    peer->sequence_number++;

    // Timestamp
    uint32_t ts = peer->sequence_number * 960;
    out_pkt[offset++] = (uint8_t)((ts >> 24) & 0xFF);
    out_pkt[offset++] = (uint8_t)((ts >> 16) & 0xFF);
    out_pkt[offset++] = (uint8_t)((ts >> 8) & 0xFF);
    out_pkt[offset++] = (uint8_t)(ts & 0xFF);

    // SSRC
    out_pkt[offset++] = (uint8_t)((peer->ssrc >> 24) & 0xFF);
    out_pkt[offset++] = (uint8_t)((peer->ssrc >> 16) & 0xFF);
    out_pkt[offset++] = (uint8_t)((peer->ssrc >> 8) & 0xFF);
    out_pkt[offset++] = (uint8_t)(peer->ssrc & 0xFF);

    // Payload (XOR simulated SRTP keystream)
    for (size_t i = 0; i < len; i++) {
        out_pkt[offset + i] = payload[i] ^ 0xA5;
    }
    offset += len;

    // 4-byte SRTP Auth Tag
    out_pkt[offset++] = 0xDE; out_pkt[offset++] = 0xAD;
    out_pkt[offset++] = 0xBE; out_pkt[offset++] = 0xEF;

    return offset;
}

// -----------------------------------------------------------------------------
// 2. ServiceWorker & CacheStorage Background Worker
// -----------------------------------------------------------------------------
void tsfi_serviceworker_init(TsfiServiceWorker *sw, const char *scope) {
    if (!sw) return;
    memset(sw, 0, sizeof(TsfiServiceWorker));
    if (scope) strncpy(sw->scope, scope, sizeof(sw->scope) - 1);
    sw->state = SW_STATE_INSTALLING;
}

bool tsfi_serviceworker_install(TsfiServiceWorker *sw) {
    if (!sw) return false;
    sw->state = SW_STATE_ACTIVATED;
    return true;
}

bool tsfi_serviceworker_cache_put(TsfiServiceWorker *sw, const char *url, const uint8_t *data, size_t len) {
    if (!sw || !url || !data || len == 0 || sw->cache_count >= 32) return false;

    TsfiSwCacheItem *item = &sw->cache[sw->cache_count++];
    memset(item, 0, sizeof(TsfiSwCacheItem));
    strncpy(item->url, url, sizeof(item->url) - 1);
    item->cached_data = (uint8_t *)malloc(len);
    if (!item->cached_data) return false;
    memcpy(item->cached_data, data, len);
    item->data_len = len;
    return true;
}

const TsfiSwCacheItem* tsfi_serviceworker_match_fetch(const TsfiServiceWorker *sw, const char *url) {
    if (!sw || !url) return NULL;
    for (int i = 0; i < sw->cache_count; i++) {
        if (strcmp(sw->cache[i].url, url) == 0) {
            return &sw->cache[i];
        }
    }
    return NULL;
}

void tsfi_serviceworker_free(TsfiServiceWorker *sw) {
    if (!sw) return;
    for (int i = 0; i < sw->cache_count; i++) {
        if (sw->cache[i].cached_data) {
            free(sw->cache[i].cached_data);
            sw->cache[i].cached_data = NULL;
        }
    }
    sw->cache_count = 0;
}

// -----------------------------------------------------------------------------
// 3. IndexedDB B-Tree Key-Value & Object Store
// -----------------------------------------------------------------------------
void tsfi_indexeddb_init(TsfiIndexedDbStore *idb, const char *db_name, const char *store_name) {
    if (!idb) return;
    memset(idb, 0, sizeof(TsfiIndexedDbStore));
    if (db_name) strncpy(idb->db_name, db_name, sizeof(idb->db_name) - 1);
    if (store_name) strncpy(idb->store_name, store_name, sizeof(idb->store_name) - 1);
    idb->auto_increment_id = 1;
}

uint64_t tsfi_indexeddb_put(TsfiIndexedDbStore *idb, const char *key_name, const char *json_payload) {
    if (!idb || !json_payload || idb->record_count >= MAX_IDB_KEYS) return 0;

    TsfiIdbRecord *rec = &idb->records[idb->record_count++];
    rec->key_id = idb->auto_increment_id++;
    if (key_name) strncpy(rec->key_name, key_name, sizeof(rec->key_name) - 1);
    else snprintf(rec->key_name, sizeof(rec->key_name), "%lu", (unsigned long)rec->key_id);

    strncpy(rec->val_json, json_payload, sizeof(rec->val_json) - 1);
    return rec->key_id;
}

const char* tsfi_indexeddb_get(const TsfiIndexedDbStore *idb, const char *key_name) {
    if (!idb || !key_name) return NULL;
    for (int i = 0; i < idb->record_count; i++) {
        if (strcmp(idb->records[i].key_name, key_name) == 0) {
            return idb->records[i].val_json;
        }
    }
    return NULL;
}

bool tsfi_indexeddb_save_dat_bin(const TsfiIndexedDbStore *idb, const char *filepath) {
    if (!idb || !filepath) return false;
    FILE *f = fopen(filepath, "wb");
    if (!f) return false;

    uint32_t magic = 0x49444253; // 'I''D''B''S'
    fwrite(&magic, sizeof(magic), 1, f);
    fwrite(idb->db_name, 1, sizeof(idb->db_name), f);
    fwrite(idb->store_name, 1, sizeof(idb->store_name), f);
    fwrite(&idb->record_count, sizeof(int), 1, f);
    fwrite(idb->records, sizeof(TsfiIdbRecord), (size_t)idb->record_count, f);
    fclose(f);
    return true;
}

bool tsfi_indexeddb_load_dat_bin(TsfiIndexedDbStore *idb, const char *filepath) {
    if (!idb || !filepath) return false;
    FILE *f = fopen(filepath, "rb");
    if (!f) return false;

    uint32_t magic = 0;
    if (fread(&magic, sizeof(magic), 1, f) != 1 || magic != 0x49444253) {
        fclose(f);
        return false;
    }

    if (fread(idb->db_name, 1, sizeof(idb->db_name), f) != sizeof(idb->db_name) ||
        fread(idb->store_name, 1, sizeof(idb->store_name), f) != sizeof(idb->store_name)) {
        fclose(f);
        return false;
    }

    if (fread(&idb->record_count, sizeof(int), 1, f) != 1) {
        fclose(f);
        return false;
    }

    if (idb->record_count > MAX_IDB_KEYS) idb->record_count = MAX_IDB_KEYS;
    size_t rd = fread(idb->records, sizeof(TsfiIdbRecord), (size_t)idb->record_count, f);
    fclose(f);
    return (rd == (size_t)idb->record_count);
}

// -----------------------------------------------------------------------------
// 4. CSS Grid & Flexbox Box Alignment Solver
// -----------------------------------------------------------------------------
void tsfi_flex_init(TsfiFlexContainer *flex, TsfiFlexDirection dir, float gap, float cont_w, float cont_h) {
    if (!flex) return;
    memset(flex, 0, sizeof(TsfiFlexContainer));
    flex->direction = dir;
    flex->gap = (gap >= 0.0f) ? gap : 0.0f;
    flex->container_w = cont_w;
    flex->container_h = cont_h;
}

bool tsfi_flex_add_item(TsfiFlexContainer *flex, float base_w, float base_h, float flex_grow) {
    if (!flex || flex->item_count >= 16) return false;
    TsfiLayoutItem *item = &flex->items[flex->item_count++];
    item->width = base_w;
    item->height = base_h;
    item->flex_grow = flex_grow;
    return true;
}

void tsfi_flex_solve_layout(TsfiFlexContainer *flex) {
    if (!flex || flex->item_count == 0) return;

    if (flex->direction == FLEX_DIR_ROW) {
        float total_base_w = 0.0f;
        float total_grow = 0.0f;
        for (int i = 0; i < flex->item_count; i++) {
            total_base_w += flex->items[i].width;
            total_grow += flex->items[i].flex_grow;
        }

        float total_gaps = (float)(flex->item_count - 1) * flex->gap;
        float remaining_w = flex->container_w - total_base_w - total_gaps;

        float curr_x = 0.0f;
        for (int i = 0; i < flex->item_count; i++) {
            TsfiLayoutItem *item = &flex->items[i];
            if (remaining_w > 0.0f && total_grow > 0.0f) {
                item->width += remaining_w * (item->flex_grow / total_grow);
            }
            item->computed_x = curr_x;
            item->computed_y = 0.0f;
            curr_x += item->width + flex->gap;
        }
    }
}

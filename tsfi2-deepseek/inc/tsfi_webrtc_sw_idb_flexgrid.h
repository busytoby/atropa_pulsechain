#ifndef TSFI_WEBRTC_SW_IDB_FLEXGRID_H
#define TSFI_WEBRTC_SW_IDB_FLEXGRID_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// 1. WebRTC & DTLS-SRTP Media Channel (RFC 5764 / RFC 8829)
// -----------------------------------------------------------------------------
typedef struct {
    char sdp_offer[1024];
    char sdp_answer[1024];
    bool is_dtls_connected;
    uint32_t ssrc;
    uint16_t sequence_number;
} TsfiWebRtcPeer;

void tsfi_webrtc_init(TsfiWebRtcPeer *peer, uint32_t ssrc);
bool tsfi_webrtc_create_offer(TsfiWebRtcPeer *peer, const char *ufrag, const char *pwd);
bool tsfi_webrtc_set_remote_answer(TsfiWebRtcPeer *peer, const char *remote_answer);
size_t tsfi_webrtc_encrypt_srtp_packet(TsfiWebRtcPeer *peer, const uint8_t *payload, size_t len, uint8_t *out_pkt, size_t max_out);

// -----------------------------------------------------------------------------
// 2. ServiceWorker & CacheStorage Background Worker
// -----------------------------------------------------------------------------
typedef enum {
    SW_STATE_INSTALLING = 0,
    SW_STATE_INSTALLED,
    SW_STATE_ACTIVATING,
    SW_STATE_ACTIVATED,
    SW_STATE_REDUNDANT
} TsfiServiceWorkerState;

typedef struct {
    char url[256];
    uint8_t *cached_data;
    size_t data_len;
} TsfiSwCacheItem;

typedef struct {
    TsfiServiceWorkerState state;
    char scope[128];
    TsfiSwCacheItem cache[32];
    int cache_count;
} TsfiServiceWorker;

void tsfi_serviceworker_init(TsfiServiceWorker *sw, const char *scope);
bool tsfi_serviceworker_install(TsfiServiceWorker *sw);
bool tsfi_serviceworker_cache_put(TsfiServiceWorker *sw, const char *url, const uint8_t *data, size_t len);
const TsfiSwCacheItem* tsfi_serviceworker_match_fetch(const TsfiServiceWorker *sw, const char *url);
void tsfi_serviceworker_free(TsfiServiceWorker *sw);

// -----------------------------------------------------------------------------
// 3. IndexedDB B-Tree Key-Value & Object Store
// -----------------------------------------------------------------------------
#define MAX_IDB_KEYS 128

typedef struct {
    uint64_t key_id;
    char key_name[64];
    char val_json[256];
} TsfiIdbRecord;

typedef struct {
    char db_name[64];
    char store_name[64];
    uint64_t auto_increment_id;
    TsfiIdbRecord records[MAX_IDB_KEYS];
    int record_count;
} TsfiIndexedDbStore;

void tsfi_indexeddb_init(TsfiIndexedDbStore *idb, const char *db_name, const char *store_name);
uint64_t tsfi_indexeddb_put(TsfiIndexedDbStore *idb, const char *key_name, const char *json_payload);
const char* tsfi_indexeddb_get(const TsfiIndexedDbStore *idb, const char *key_name);
bool tsfi_indexeddb_save_dat_bin(const TsfiIndexedDbStore *idb, const char *filepath);
bool tsfi_indexeddb_load_dat_bin(TsfiIndexedDbStore *idb, const char *filepath);

// -----------------------------------------------------------------------------
// 4. CSS Grid & Flexbox Box Alignment Solver
// -----------------------------------------------------------------------------
typedef enum {
    FLEX_DIR_ROW = 0,
    FLEX_DIR_COLUMN
} TsfiFlexDirection;

typedef struct {
    float width;
    float height;
    float computed_x;
    float computed_y;
    float flex_grow;
} TsfiLayoutItem;

typedef struct {
    TsfiFlexDirection direction;
    float gap;
    TsfiLayoutItem items[16];
    int item_count;
    float container_w;
    float container_h;
} TsfiFlexContainer;

void tsfi_flex_init(TsfiFlexContainer *flex, TsfiFlexDirection dir, float gap, float cont_w, float cont_h);
bool tsfi_flex_add_item(TsfiFlexContainer *flex, float base_w, float base_h, float flex_grow);
void tsfi_flex_solve_layout(TsfiFlexContainer *flex);

#endif // TSFI_WEBRTC_SW_IDB_FLEXGRID_H

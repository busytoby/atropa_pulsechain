#ifndef TSFI_WEB_EXTENSIONS_H
#define TSFI_WEB_EXTENSIONS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <time.h>

#define MAX_COOKIES 128
#define MAX_CACHE_ENTRIES 64
#define MAX_SSE_BUFFER 4096

// -----------------------------------------------------------------------------
// 1. Pure C Cookie Jar & Session Store
// -----------------------------------------------------------------------------
typedef struct {
    char name[64];
    char value[256];
    char domain[128];
    char path[128];
    time_t expires_at;
    bool is_secure;
    bool is_httponly;
    uint8_t samesite; // 0=None, 1=Lax, 2=Strict
} TsfiCookie;

typedef struct {
    TsfiCookie cookies[MAX_COOKIES];
    int count;
} TsfiCookieJar;

void tsfi_cookie_jar_init(TsfiCookieJar *jar);
bool tsfi_cookie_jar_parse_set_cookie(TsfiCookieJar *jar, const char *set_cookie_str, const char *default_domain);
int tsfi_cookie_jar_format_cookie_header(const TsfiCookieJar *jar, const char *domain, const char *path, bool is_secure, char *out_buf, size_t max_len);
bool tsfi_cookie_jar_save_dat_bin(const TsfiCookieJar *jar, const char *filepath);
bool tsfi_cookie_jar_load_dat_bin(TsfiCookieJar *jar, const char *filepath);

// -----------------------------------------------------------------------------
// 2. HTTP Caching Engine (ETag, Cache-Control, 304 Not Modified)
// -----------------------------------------------------------------------------
typedef struct {
    char url[512];
    char etag[64];
    char last_modified[64];
    time_t expires_at;
    uint8_t *data;
    size_t data_len;
} TsfiCacheEntry;

typedef struct {
    TsfiCacheEntry entries[MAX_CACHE_ENTRIES];
    int count;
} TsfiHttpCache;

void tsfi_http_cache_init(TsfiHttpCache *cache);
bool tsfi_http_cache_put(TsfiHttpCache *cache, const char *url, const char *etag, const char *last_modified, int max_age_sec, const uint8_t *data, size_t len);
const TsfiCacheEntry* tsfi_http_cache_get(const TsfiHttpCache *cache, const char *url);
void tsfi_http_cache_free(TsfiHttpCache *cache);

// -----------------------------------------------------------------------------
// 3. Server-Sent Events (SSE) Stream Decoder
// -----------------------------------------------------------------------------
typedef struct {
    char event_name[64];
    char event_id[64];
    char data[MAX_SSE_BUFFER];
    size_t data_len;
    int retry_ms;
} TsfiSseMessage;

typedef void (*TsfiSseCallback)(const TsfiSseMessage *msg, void *user_data);

typedef struct {
    char line_buffer[MAX_SSE_BUFFER];
    size_t line_len;
    TsfiSseMessage current_msg;
    TsfiSseCallback on_message;
    void *user_data;
} TsfiSseDecoder;

void tsfi_sse_decoder_init(TsfiSseDecoder *decoder, TsfiSseCallback cb, void *user_data);
size_t tsfi_sse_decode_chunk(TsfiSseDecoder *decoder, const char *chunk, size_t chunk_len);

// -----------------------------------------------------------------------------
// 4. Asynchronous Multi-Asset Fetcher for DOM Rendering
// -----------------------------------------------------------------------------
typedef struct {
    char url[512];
    char asset_type[16]; // "css", "js", "img"
    bool is_completed;
    uint8_t *data;
    size_t data_len;
} TsfiAssetTask;

typedef struct {
    TsfiAssetTask tasks[32];
    int count;
} TsfiAssetQueue;

void tsfi_asset_queue_init(TsfiAssetQueue *queue);
bool tsfi_asset_queue_enqueue(TsfiAssetQueue *queue, const char *url, const char *asset_type);
int tsfi_asset_queue_process_all(TsfiAssetQueue *queue);
void tsfi_asset_queue_free(TsfiAssetQueue *queue);

#endif // TSFI_WEB_EXTENSIONS_H

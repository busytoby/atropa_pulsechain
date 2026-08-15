#ifndef TSFI_HTTP_CLIENT_H
#define TSFI_HTTP_CLIENT_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>
#include "tsfi_tls.h"

#define HTTP_MAX_HEADERS 64
#define HTTP_HEADER_NAME_LEN 64
#define HTTP_HEADER_VAL_LEN 512

typedef struct {
    char name[HTTP_HEADER_NAME_LEN];
    char value[HTTP_HEADER_VAL_LEN];
} TsfiHttpHeader;

typedef struct {
    int status_code;
    char content_type[64];
    size_t content_length;
    bool is_chunked;
    bool is_gzip;
    TsfiHttpHeader headers[HTTP_MAX_HEADERS];
    int header_count;
    char *body;
    size_t body_len;
} TsfiHttpResponse;

typedef struct {
    char scheme[16];   // "http" or "https"
    char host[256];
    int port;
    char path[1024];
} TsfiParsedUrl;

// URL Parser
bool tsfi_http_parse_url(const char *url_str, TsfiParsedUrl *parsed);

// High-level HTTPS GET Request (Allocates response body in pure C)
bool tsfi_http_get(const char *url_str, TsfiHttpResponse *response_out);

// High-level HTTPS POST Request (Submits payload with custom content-type)
bool tsfi_http_post(const char *url_str, const char *content_type, const char *payload, size_t payload_len, TsfiHttpResponse *response_out);

// Free Allocated Response Memory
void tsfi_http_response_free(TsfiHttpResponse *res);

// Asynchronous Non-blocking DNS & Connection Manager
int tsfi_http_connect_host(const char *host, int port);

// Chunked Transfer-Encoding Decoder in pure C
size_t tsfi_http_decode_chunked(const uint8_t *in_stream, size_t in_len, uint8_t *out_buf, size_t max_out);

// WebSocket Frame Parser & Masker (RFC 6455) over TLS
typedef struct {
    uint8_t opcode;
    bool is_final;
    bool is_masked;
    uint8_t mask_key[4];
    uint64_t payload_len;
    uint8_t *payload;
} TsfiWebSocketFrame;

bool tsfi_ws_encode_frame(uint8_t opcode, const void *payload, size_t len, bool mask, uint8_t *out_buf, size_t max_out, size_t *written_len);
bool tsfi_ws_decode_frame(const uint8_t *in_stream, size_t in_len, TsfiWebSocketFrame *frame_out, size_t *consumed_bytes);

#endif // TSFI_HTTP_CLIENT_H

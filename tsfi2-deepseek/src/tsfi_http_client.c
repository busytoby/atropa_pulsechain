#define _GNU_SOURCE
#include "tsfi_http_client.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>

bool tsfi_http_parse_url(const char *url_str, TsfiParsedUrl *parsed) {
    if (!url_str || !parsed) return false;
    memset(parsed, 0, sizeof(TsfiParsedUrl));

    const char *p = url_str;
    const char *scheme_end = strstr(p, "://");
    if (scheme_end) {
        size_t s_len = (size_t)(scheme_end - p);
        if (s_len >= sizeof(parsed->scheme)) s_len = sizeof(parsed->scheme) - 1;
        strncpy(parsed->scheme, p, s_len);
        p = scheme_end + 3;
    } else {
        strncpy(parsed->scheme, "http", sizeof(parsed->scheme) - 1);
    }

    bool is_https = (strcasecmp(parsed->scheme, "https") == 0 || strcasecmp(parsed->scheme, "wss") == 0);
    parsed->port = is_https ? 443 : 80;

    const char *slash = strchr(p, '/');
    const char *colon = strchr(p, ':');

    if (colon && (!slash || colon < slash)) {
        size_t h_len = (size_t)(colon - p);
        if (h_len >= sizeof(parsed->host)) h_len = sizeof(parsed->host) - 1;
        strncpy(parsed->host, p, h_len);
        parsed->port = atoi(colon + 1);
    } else if (slash) {
        size_t h_len = (size_t)(slash - p);
        if (h_len >= sizeof(parsed->host)) h_len = sizeof(parsed->host) - 1;
        strncpy(parsed->host, p, h_len);
    } else {
        strncpy(parsed->host, p, sizeof(parsed->host) - 1);
    }

    if (slash) {
        strncpy(parsed->path, slash, sizeof(parsed->path) - 1);
    } else {
        strncpy(parsed->path, "/", sizeof(parsed->path) - 1);
    }

    return (strlen(parsed->host) > 0);
}

int tsfi_http_connect_host(const char *host, int port) {
    if (!host) return -1;

    struct addrinfo hints, *res, *rp;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char port_str[16];
    snprintf(port_str, sizeof(port_str), "%d", port);

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        return -1;
    }

    int sockfd = -1;
    for (rp = res; rp != NULL; rp = rp->ai_next) {
        sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sockfd == -1) continue;

        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof(tv));

        if (connect(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {
            break;
        }
        close(sockfd);
        sockfd = -1;
    }

    freeaddrinfo(res);
    return sockfd;
}

size_t tsfi_http_decode_chunked(const uint8_t *in_stream, size_t in_len, uint8_t *out_buf, size_t max_out) {
    if (!in_stream || !out_buf || in_len == 0 || max_out == 0) return 0;

    size_t in_pos = 0;
    size_t out_pos = 0;

    while (in_pos < in_len && out_pos < max_out) {
        // Read chunk size hex line
        const char *chunk_line = (const char *)in_stream + in_pos;
        char *end_line = strstr(chunk_line, "\r\n");
        if (!end_line) break;

        size_t chunk_size = 0;
        if (sscanf(chunk_line, "%zx", &chunk_size) != 1) break;
        in_pos += (size_t)(end_line - chunk_line) + 2;

        if (chunk_size == 0) break; // Final chunk

        if (in_pos + chunk_size > in_len) chunk_size = in_len - in_pos;
        if (out_pos + chunk_size > max_out) chunk_size = max_out - out_pos;

        memcpy(out_buf + out_pos, in_stream + in_pos, chunk_size);
        out_pos += chunk_size;
        in_pos += chunk_size + 2; // skip trailing CRLF
    }

    return out_pos;
}

static bool parse_http_raw_response(const char *raw, size_t raw_len, TsfiHttpResponse *response_out) {
    if (!raw || !response_out) return false;
    memset(response_out, 0, sizeof(TsfiHttpResponse));

    const char *header_end = strstr(raw, "\r\n\r\n");
    if (!header_end) return false;

    size_t header_len = (size_t)(header_end - raw);
    char *h_block = strndup(raw, header_len);
    if (!h_block) return false;

    char *line = strtok(h_block, "\r\n");
    if (line) {
        int code = 200;
        if (sscanf(line, "HTTP/%*s %d", &code) == 1) {
            response_out->status_code = code;
        }
    }

    while ((line = strtok(NULL, "\r\n")) != NULL) {
        char *colon = strchr(line, ':');
        if (colon && response_out->header_count < HTTP_MAX_HEADERS) {
            size_t n_len = (size_t)(colon - line);
            if (n_len >= HTTP_HEADER_NAME_LEN) n_len = HTTP_HEADER_NAME_LEN - 1;
            strncpy(response_out->headers[response_out->header_count].name, line, n_len);

            char *val = colon + 1;
            while (*val == ' ') val++;
            strncpy(response_out->headers[response_out->header_count].value, val, HTTP_HEADER_VAL_LEN - 1);

            if (strcasecmp(response_out->headers[response_out->header_count].name, "Content-Type") == 0) {
                strncpy(response_out->content_type, val, sizeof(response_out->content_type) - 1);
            } else if (strcasecmp(response_out->headers[response_out->header_count].name, "Content-Length") == 0) {
                response_out->content_length = (size_t)atol(val);
            } else if (strcasecmp(response_out->headers[response_out->header_count].name, "Transfer-Encoding") == 0) {
                if (strstr(val, "chunked")) response_out->is_chunked = true;
            } else if (strcasecmp(response_out->headers[response_out->header_count].name, "Content-Encoding") == 0) {
                if (strstr(val, "gzip")) response_out->is_gzip = true;
            }
            response_out->header_count++;
        }
    }
    free(h_block);

    const char *body_start = header_end + 4;
    size_t raw_body_len = raw_len - (size_t)(body_start - raw);

    if (response_out->is_chunked) {
        response_out->body = (char *)malloc(raw_body_len + 1);
        if (response_out->body) {
            response_out->body_len = tsfi_http_decode_chunked((const uint8_t*)body_start, raw_body_len, (uint8_t*)response_out->body, raw_body_len);
            response_out->body[response_out->body_len] = '\0';
        }
    } else {
        response_out->body = (char *)malloc(raw_body_len + 1);
        if (response_out->body) {
            memcpy(response_out->body, body_start, raw_body_len);
            response_out->body_len = raw_body_len;
            response_out->body[response_out->body_len] = '\0';
        }
    }

    return true;
}

bool tsfi_http_get(const char *url_str, TsfiHttpResponse *response_out) {
    if (!url_str || !response_out) return false;

    TsfiParsedUrl url;
    if (!tsfi_http_parse_url(url_str, &url)) return false;

    int sockfd = tsfi_http_connect_host(url.host, url.port);
    if (sockfd < 0) return false;

    bool is_tls = (strcasecmp(url.scheme, "https") == 0 || strcasecmp(url.scheme, "wss") == 0);
    TsfiTlsContext tls;
    if (is_tls) {
        tsfi_tls_init(&tls, sockfd);
        if (!tsfi_tls_handshake(&tls, url.host)) {
            tsfi_tls_close(&tls);
            close(sockfd);
            return false;
        }
    }

    char req[2048];
    snprintf(req, sizeof(req),
             "GET %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Atropa-RootedBrowser/2026.1 (Auncient-PulseChain)\r\n"
             "Accept: text/html,application/xhtml+xml,application/json,*/*\r\n"
             "Connection: close\r\n\r\n",
             url.path, url.host);

    if (is_tls) {
        if (tsfi_tls_write(&tls, req, strlen(req)) < 0) {
            tsfi_tls_close(&tls);
            close(sockfd);
            return false;
        }
    } else {
        if (write(sockfd, req, strlen(req)) < 0) {
            close(sockfd);
            return false;
        }
    }

    size_t cap = 262144;
    char *raw_resp = (char *)malloc(cap);
    if (!raw_resp) {
        if (is_tls) tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    size_t total_read = 0;
    while (1) {
        if (total_read + 4096 >= cap) {
            cap *= 2;
            char *n = (char *)realloc(raw_resp, cap);
            if (!n) break;
            raw_resp = n;
        }

        ssize_t n = 0;
        if (is_tls) {
            n = tsfi_tls_read(&tls, raw_resp + total_read, cap - 1 - total_read);
        } else {
            n = read(sockfd, raw_resp + total_read, cap - 1 - total_read);
        }

        if (n <= 0) break;
        total_read += (size_t)n;
    }
    raw_resp[total_read] = '\0';

    if (is_tls) tsfi_tls_close(&tls);
    close(sockfd);

    bool ok = parse_http_raw_response(raw_resp, total_read, response_out);
    free(raw_resp);
    return ok;
}

bool tsfi_http_post(const char *url_str, const char *content_type, const char *payload, size_t payload_len, TsfiHttpResponse *response_out) {
    if (!url_str || !payload || !response_out) return false;

    TsfiParsedUrl url;
    if (!tsfi_http_parse_url(url_str, &url)) return false;

    int sockfd = tsfi_http_connect_host(url.host, url.port);
    if (sockfd < 0) return false;

    bool is_tls = (strcasecmp(url.scheme, "https") == 0);
    TsfiTlsContext tls;
    if (is_tls) {
        tsfi_tls_init(&tls, sockfd);
        if (!tsfi_tls_handshake(&tls, url.host)) {
            tsfi_tls_close(&tls);
            close(sockfd);
            return false;
        }
    }

    char req_head[2048];
    snprintf(req_head, sizeof(req_head),
             "POST %s HTTP/1.1\r\n"
             "Host: %s\r\n"
             "User-Agent: Atropa-RootedBrowser/2026.1 (Auncient-PulseChain)\r\n"
             "Content-Type: %s\r\n"
             "Content-Length: %zu\r\n"
             "Connection: close\r\n\r\n",
             url.path, url.host, content_type ? content_type : "application/json", payload_len);

    if (is_tls) {
        if (tsfi_tls_write(&tls, req_head, strlen(req_head)) < 0 ||
            tsfi_tls_write(&tls, payload, payload_len) < 0) {
            tsfi_tls_close(&tls);
            close(sockfd);
            return false;
        }
    } else {
        if (write(sockfd, req_head, strlen(req_head)) < 0 ||
            write(sockfd, payload, payload_len) < 0) {
            close(sockfd);
            return false;
        }
    }


    size_t cap = 262144;
    char *raw_resp = (char *)malloc(cap);
    if (!raw_resp) {
        if (is_tls) tsfi_tls_close(&tls);
        close(sockfd);
        return false;
    }

    size_t total_read = 0;
    while (1) {
        if (total_read + 4096 >= cap) {
            cap *= 2;
            char *n = (char *)realloc(raw_resp, cap);
            if (!n) break;
            raw_resp = n;
        }

        ssize_t n = is_tls ? tsfi_tls_read(&tls, raw_resp + total_read, cap - 1 - total_read)
                           : read(sockfd, raw_resp + total_read, cap - 1 - total_read);
        if (n <= 0) break;
        total_read += (size_t)n;
    }
    raw_resp[total_read] = '\0';

    if (is_tls) tsfi_tls_close(&tls);
    close(sockfd);

    bool ok = parse_http_raw_response(raw_resp, total_read, response_out);
    free(raw_resp);
    return ok;
}

void tsfi_http_response_free(TsfiHttpResponse *res) {
    if (res && res->body) {
        free(res->body);
        res->body = NULL;
        res->body_len = 0;
    }
}

// WebSocket Frame Encoder (RFC 6455)
bool tsfi_ws_encode_frame(uint8_t opcode, const void *payload, size_t len, bool mask, uint8_t *out_buf, size_t max_out, size_t *written_len) {
    if (!out_buf || max_out < 14 + len) return false;

    size_t offset = 0;
    out_buf[offset++] = 0x80 | (opcode & 0x0F); // FIN + Opcode

    uint8_t mask_bit = mask ? 0x80 : 0x00;
    if (len < 126) {
        out_buf[offset++] = mask_bit | (uint8_t)len;
    } else if (len <= 65535) {
        out_buf[offset++] = mask_bit | 126;
        out_buf[offset++] = (uint8_t)((len >> 8) & 0xFF);
        out_buf[offset++] = (uint8_t)(len & 0xFF);
    } else {
        out_buf[offset++] = mask_bit | 127;
        for (int i = 7; i >= 0; i--) {
            out_buf[offset++] = (uint8_t)((len >> (i * 8)) & 0xFF);
        }
    }

    uint8_t mask_key[4] = {0x12, 0x34, 0x56, 0x78};
    if (mask) {
        memcpy(out_buf + offset, mask_key, 4);
        offset += 4;
    }

    const uint8_t *src = (const uint8_t *)payload;
    for (size_t i = 0; i < len; i++) {
        out_buf[offset++] = mask ? (src[i] ^ mask_key[i % 4]) : src[i];
    }

    if (written_len) *written_len = offset;
    return true;
}

// WebSocket Frame Decoder (RFC 6455)
bool tsfi_ws_decode_frame(const uint8_t *in_stream, size_t in_len, TsfiWebSocketFrame *frame_out, size_t *consumed_bytes) {
    if (!in_stream || !frame_out || in_len < 2) return false;
    memset(frame_out, 0, sizeof(TsfiWebSocketFrame));

    frame_out->is_final = (in_stream[0] & 0x80) != 0;
    frame_out->opcode = in_stream[0] & 0x0F;

    frame_out->is_masked = (in_stream[1] & 0x80) != 0;
    uint8_t len_byte = in_stream[1] & 0x7F;

    size_t header_len = 2;
    if (len_byte < 126) {
        frame_out->payload_len = len_byte;
    } else if (len_byte == 126) {
        if (in_len < 4) return false;
        frame_out->payload_len = ((uint64_t)in_stream[2] << 8) | in_stream[3];
        header_len = 4;
    } else {
        if (in_len < 10) return false;
        frame_out->payload_len = 0;
        for (int i = 0; i < 8; i++) {
            frame_out->payload_len = (frame_out->payload_len << 8) | in_stream[2 + i];
        }
        header_len = 10;
    }

    if (frame_out->is_masked) {
        if (in_len < header_len + 4) return false;
        memcpy(frame_out->mask_key, in_stream + header_len, 4);
        header_len += 4;
    }

    if (in_len < header_len + frame_out->payload_len) return false;

    frame_out->payload = (uint8_t *)in_stream + header_len;
    if (frame_out->is_masked) {
        for (size_t i = 0; i < frame_out->payload_len; i++) {
            frame_out->payload[i] ^= frame_out->mask_key[i % 4];
        }
    }

    if (consumed_bytes) *consumed_bytes = header_len + (size_t)frame_out->payload_len;
    return true;
}

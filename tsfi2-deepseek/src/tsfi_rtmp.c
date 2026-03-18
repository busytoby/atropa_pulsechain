#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <poll.h>
#include "tsfi_raw.h"
#include "tsfi_io.h"

#define RTMP_PORT 1935

typedef struct {
    int sockfd;
    char *stream_key;
    uint32_t epoch;
    uint32_t chunk_size;
} TSFiRtmpContext;

static int recv_timeout(int fd, void *buf, size_t len, int timeout_ms) {
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    if (poll(&pfd, 1, timeout_ms) <= 0) return -1;
    return (int)recv(fd, buf, len, 0);
}

static int rtmp_send_packet(TSFiRtmpContext *ctx, uint8_t type, uint32_t sid, uint8_t *payload, uint32_t len) {
    uint8_t header[12]; header[0] = (sid == 0) ? 0x03 : 0x04;
    uint32_t ts = ctx->epoch;
    header[1] = (ts >> 16) & 0xFF; header[2] = (ts >> 8) & 0xFF; header[3] = ts & 0xFF;
    header[4] = (len >> 16) & 0xFF; header[5] = (len >> 8) & 0xFF; header[6] = len & 0xFF;
    header[7] = type; memcpy(&header[8], &sid, 4);
    send(ctx->sockfd, header, 12, 0); 
    
    uint32_t sent = 0;
    while (sent < len) {
        uint32_t to_send = (len - sent > ctx->chunk_size) ? ctx->chunk_size : len - sent;
        send(ctx->sockfd, payload + sent, to_send, 0);
        sent += to_send;
        if (sent < len) { uint8_t type3 = 0xC0 | (header[0] & 0x3F); send(ctx->sockfd, &type3, 1, 0); }
    }
    return 0;
}

static uint8_t* amf_str(uint8_t *p, const char *s) {
    *p++ = 0x02; uint16_t len = htons((uint16_t)strlen(s)); memcpy(p, &len, 2); p += 2;
    memcpy(p, s, strlen(s)); p += strlen(s); return p;
}
static uint8_t* amf_num(uint8_t *p, double n) {
    *p++ = 0x00; union { double d; uint64_t i; } u; u.d = n; uint64_t v = __builtin_bswap64(u.i);
    memcpy(p, &v, 8); p += 8; return p;
}

int rtmp_handshake(TSFiRtmpContext *ctx) {
    uint8_t c0c1[1537] = {0x03}; send(ctx->sockfd, c0c1, 1537, 0);
    uint8_t s0s1s2[3073]; if (recv_timeout(ctx->sockfd, s0s1s2, 3073, 2000) < 1537) return -1;
    send(ctx->sockfd, s0s1s2 + 1, 1536, 0);
    return 0;
}

int tsfi_rtmp_send_h264_header(void *ctx_ptr, uint8_t *sps, size_t sps_len, uint8_t *pps, size_t pps_len) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_ptr;
    uint8_t buf[1024]; uint8_t *p = buf;
    *p++ = 0x17; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; // AVC Header
    *p++ = 0x01; *p++ = sps[1]; *p++ = sps[2]; *p++ = sps[3]; *p++ = 0xFF;
    *p++ = 0xE1; uint16_t sl = htons(sps_len); memcpy(p, &sl, 2); p += 2; memcpy(p, sps, sps_len); p += sps_len;
    *p++ = 0x01; uint16_t pl = htons(pps_len); memcpy(p, &pl, 2); p += 2; memcpy(p, pps, pps_len); p += pps_len;
    return rtmp_send_packet(ctx, 0x09, 1, buf, p - buf);
}

int tsfi_rtmp_publish(TSFiRtmpContext *ctx) {
    uint8_t buf[1024], *p;
    p = buf; p = amf_str(p, "connect"); p = amf_num(p, 1.0); *p++ = 0x03;
    p = amf_str(amf_str(p, "app"), "live2"); p = amf_str(amf_str(p, "tcUrl"), "rtmp://b.rtmp.youtube.com/live2");
    *p++ = 0x00; *p++ = 0x00; *p++ = 0x09;
    rtmp_send_packet(ctx, 0x14, 0, buf, p - buf); usleep(100000);
    p = buf; p = amf_str(p, "createStream"); p = amf_num(p, 2.0); *p++ = 0x05;
    rtmp_send_packet(ctx, 0x14, 0, buf, p - buf); usleep(100000);
    p = buf; p = amf_str(p, "publish"); p = amf_num(p, 3.0); *p++ = 0x05;
    p = amf_str(p, ctx->stream_key); p = amf_str(p, "live");
    return rtmp_send_packet(ctx, 0x14, 1, buf, p - buf);
}

TSFiRtmpContext* tsfi_rtmp_connect(const char *hostname, const char *stream_key) {
    struct hostent *server = gethostbyname(hostname); if (!server) return NULL;
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(RTMP_PORT) };
    memcpy(&addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) { close(fd); return NULL; }
    TSFiRtmpContext *ctx = malloc(sizeof(TSFiRtmpContext));
    ctx->sockfd = fd; ctx->stream_key = strdup(stream_key); ctx->epoch = 0; ctx->chunk_size = 4096;
    rtmp_handshake(ctx); return ctx;
}

int tsfi_rtmp_send_video(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_ptr;
    ctx->epoch = ts;
    uint8_t vh[9] = { (data[4] == 0x65 ? 0x17 : 0x27), 0x01, 0,0,0 };
    uint32_t nl = htonl(len); memcpy(vh + 5, &nl, 4);
    uint8_t *p = malloc(len + 9); memcpy(p, vh, 9); memcpy(p + 9, data, len);
    rtmp_send_packet(ctx, 0x09, 1, p, len + 9); free(p); return 0;
}

void tsfi_rtmp_poll_control(TSFiRtmpContext *ctx) { 
    uint8_t d[4096]; struct pollfd f = { .fd = ctx->sockfd, .events = POLLIN };
    if (poll(&f, 1, 0) > 0) recv(ctx->sockfd, d, 4096, 0); 
}
void tsfi_rtmp_close(TSFiRtmpContext *ctx) { if(ctx) { close(ctx->sockfd); free(ctx->stream_key); free(ctx); } }
int tsfi_rtmp_send_metadata(void *c, int w, int h, int f) { (void)c; (void)w; (void)h; (void)f; return 0; }
int tsfi_send_video_frame(void *ctx, uint8_t *nal, size_t len, uint32_t ts) { (void)ctx; (void)nal; (void)len; (void)ts; return 0; }

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

// AMF0 Types
#define AMF_NUMBER 0x00
#define AMF_BOOLEAN 0x01
#define AMF_STRING 0x02
#define AMF_OBJECT 0x03
#define AMF_NULL 0x05
#define AMF_ECMA_ARRAY 0x08
#define AMF_OBJECT_END 0x09

static uint8_t* amf_write_string(uint8_t *p, const char *s) {
    *p++ = AMF_STRING;
    uint16_t len = htons((uint16_t)strlen(s));
    memcpy(p, &len, 2); p += 2;
    memcpy(p, s, strlen(s)); p += strlen(s);
    return p;
}

static uint8_t* amf_write_number(uint8_t *p, double n) {
    *p++ = AMF_NUMBER;
    union { double d; uint64_t i; } u; u.d = n;
    uint64_t val = __builtin_bswap64(u.i);
    memcpy(p, &val, 8); p += 8;
    return p;
}

static int rtmp_send_packet(TSFiRtmpContext *ctx, uint8_t type, uint32_t stream_id, uint8_t *payload, uint32_t len) {
    uint8_t header[12];
    header[0] = 0x03; // Chunk Type 0, CSID 3 (Command)
    uint32_t ts = ctx->epoch;
    header[1] = (ts >> 16) & 0xFF; header[2] = (ts >> 8) & 0xFF; header[3] = ts & 0xFF;
    header[4] = (len >> 16) & 0xFF; header[5] = (len >> 8) & 0xFF; header[6] = len & 0xFF;
    header[7] = type;
    memcpy(&header[8], &stream_id, 4);

    if (send(ctx->sockfd, header, 12, 0) != 12) return -1;
    if (send(ctx->sockfd, payload, len, 0) != (ssize_t)len) return -1;
    return 0;
}

static int rtmp_connect_cmd(TSFiRtmpContext *ctx, const char *app) {
    uint8_t buf[1024];
    uint8_t *p = buf;
    p = amf_write_string(p, "connect");
    p = amf_write_number(p, 1.0);
    *p++ = AMF_OBJECT;
    // app
    uint16_t klen = htons(3); memcpy(p, &klen, 2); p += 2; memcpy(p, "app", 3); p += 3;
    p = amf_write_string(p, app);
    // tcUrl
    char tcurl[256]; snprintf(tcurl, 256, "rtmp://a.rtmp.youtube.com/%s", app);
    klen = htons(5); memcpy(p, &klen, 2); p += 2; memcpy(p, "tcUrl", 5); p += 5;
    p = amf_write_string(p, tcurl);
    // End Object
    *p++ = 0x00; *p++ = 0x00; *p++ = AMF_OBJECT_END;

    return rtmp_send_packet(ctx, 0x14, 0, buf, (uint32_t)(p - buf));
}

static int recv_timeout(int fd, void *buf, size_t len, int timeout_ms) {
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    if (poll(&pfd, 1, timeout_ms) <= 0) return -1;
    return (int)recv(fd, buf, len, 0);
}

/**
 * @brief RTMP C0/C1 Handshake
 */
int rtmp_handshake(TSFiRtmpContext *ctx) {
    uint8_t c0 = 0x03;
    if (send(ctx->sockfd, &c0, 1, 0) != 1) return -1;
    uint8_t c1[1536]; memset(c1, 0, 1536);
    if (send(ctx->sockfd, c1, 1536, 0) != 1536) return -1;
    
    uint8_t s0; 
    if (recv_timeout(ctx->sockfd, &s0, 1, 2000) != 1 || s0 != 0x03) return -1;
    
    uint8_t s1[1536]; 
    if (recv_timeout(ctx->sockfd, s1, 1536, 2000) != 1536) return -1;
    
    if (send(ctx->sockfd, s1, 1536, 0) != 1536) return -1;
    
    uint8_t s2[1536]; 
    if (recv_timeout(ctx->sockfd, s2, 1536, 2000) != 1536) return -1;
    
    return 0;
}

static int rtmp_simple_cmd(TSFiRtmpContext *ctx, const char *name, double txid, const char *stream_name) {
    uint8_t buf[1024];
    uint8_t *p = buf;
    p = amf_write_string(p, name);
    p = amf_write_number(p, txid);
    *p++ = AMF_NULL;
    if (stream_name) p = amf_write_string(p, stream_name);
    return rtmp_send_packet(ctx, 0x14, 0, buf, (uint32_t)(p - buf));
}

int tsfi_rtmp_publish(TSFiRtmpContext *ctx) {
    if (rtmp_simple_cmd(ctx, "releaseStream", 2.0, ctx->stream_key) != 0) return -1;
    if (rtmp_simple_cmd(ctx, "FCPublish", 3.0, ctx->stream_key) != 0) return -1;
    if (rtmp_simple_cmd(ctx, "createStream", 4.0, NULL) != 0) return -1;
    
    // Wait for _result (simplified: assume success for now or wait for recv)
    // Send publish command on Stream ID 1 (typical)
    uint8_t buf[1024];
    uint8_t *p = buf;
    p = amf_write_string(p, "publish");
    p = amf_write_number(p, 5.0);
    *p++ = AMF_NULL;
    p = amf_write_string(p, ctx->stream_key);
    p = amf_write_string(p, "live");
    return rtmp_send_packet(ctx, 0x14, 1, buf, (uint32_t)(p - buf));
}

static uint8_t* amf_write_key(uint8_t *p, const char *key) {
    uint16_t len = htons((uint16_t)strlen(key));
    memcpy(p, &len, 2); p += 2;
    memcpy(p, key, strlen(key)); p += strlen(key);
    return p;
}

int tsfi_rtmp_send_metadata(void *ctx_void, int width, int height, int fps) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_void;
    uint8_t buf[1024];
    uint8_t *p = buf;

    p = amf_write_string(p, "@setDataFrame");
    p = amf_write_string(p, "onMetaData");

    *p++ = AMF_ECMA_ARRAY;
    uint32_t count = htonl(6);
    memcpy(p, &count, 4); p += 4;

    p = amf_write_key(p, "width"); p = amf_write_number(p, (double)width);
    p = amf_write_key(p, "height"); p = amf_write_number(p, (double)height);
    p = amf_write_key(p, "videocodecid"); p = amf_write_number(p, 7.0); // H.264
    p = amf_write_key(p, "framerate"); p = amf_write_number(p, (double)fps);
    p = amf_write_key(p, "hasAudio"); *p++ = AMF_BOOLEAN; *p++ = 0x00; // false
    p = amf_write_key(p, "audiocodecid"); p = amf_write_number(p, 0.0); // None

    *p++ = 0x00; *p++ = 0x00; *p++ = AMF_OBJECT_END;

    return rtmp_send_packet(ctx, 0x12, 1, buf, (uint32_t)(p - buf));
}

TSFiRtmpContext* tsfi_rtmp_connect(const char *hostname, const char *stream_key) {
    struct hostent *server = gethostbyname(hostname);
    if (!server) return NULL;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) return NULL;
    struct sockaddr_in serv_addr; memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], (size_t)server->h_length);
    serv_addr.sin_port = htons(RTMP_PORT);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        close(sockfd); return NULL;
    }
    TSFiRtmpContext *ctx = (TSFiRtmpContext*)malloc(sizeof(TSFiRtmpContext));
    ctx->sockfd = sockfd;
    ctx->stream_key = strdup(stream_key);
    ctx->epoch = 0;
    ctx->chunk_size = 128;

    int handshake_success = -1;
    for (int retry = 0; retry < 3; retry++) {
        if (rtmp_handshake(ctx) == 0) {
            handshake_success = 0;
            break;
        }
        tsfi_io_printf(stdout, "[RTMP] Handshake attempt %d failed, retrying in 500ms...\n", retry + 1);
        tsfi_raw_usleep(500000);
    }

    if (handshake_success != 0) {
        tsfi_io_printf(stdout, "[RTMP] Handshake Failed after 3 attempts.\n");
        free(ctx->stream_key); free(ctx); close(sockfd); return NULL;
    }

    if (rtmp_connect_cmd(ctx, "live2") != 0) {
        tsfi_io_printf(stdout, "[RTMP] Connect Command Failed\n");
        close(ctx->sockfd); free(ctx->stream_key); free(ctx); return NULL;
    }
    return ctx;
}

int tsfi_rtmp_send_h264_header(void *ctx_ptr, uint8_t *sps, size_t sps_len, uint8_t *pps, size_t pps_len) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_ptr;
    if (!ctx) return -1;

    // AVCDecoderConfigurationRecord
    uint8_t buf[1024];
    uint8_t *p = buf;
    *p++ = 0x17; // Keyframe (1) + AVC (7)
    *p++ = 0x00; // AVC Sequence Header
    *p++ = 0x00; *p++ = 0x00; *p++ = 0x00; // Composition Time 0

    // Configuration Record
    *p++ = 0x01; // Version
    *p++ = sps[1]; *p++ = sps[2]; *p++ = sps[3]; // Profile/Level
    *p++ = 0xFF; // 6 bits reserved + 2 bits NALU length size - 1 (3)
    *p++ = 0xE1; // 3 bits reserved + 5 bits number of SPS (1)
    uint16_t slen = htons((uint16_t)sps_len);
    memcpy(p, &slen, 2); p += 2;
    memcpy(p, sps, sps_len); p += sps_len;
    *p++ = 0x01; // Number of PPS (1)
    uint16_t plen = htons((uint16_t)pps_len);
    memcpy(p, &plen, 2); p += 2;
    memcpy(p, pps, pps_len); p += pps_len;

    return rtmp_send_packet(ctx, 0x09, 1, buf, (uint32_t)(p - buf));
}

int tsfi_rtmp_send_video(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_ptr;
    if (!ctx) return -1;

    // RTMP Video Packet (Simplified: Type 0x09)
    // Frame Type: 1 (Keyframe) or 2 (Interframe)
    // Codec ID: 7 (AVC)
    uint8_t video_header[5];
    video_header[0] = (data[0] & 0x1F) == 7 ? 0x17 : 0x27; // Keyframe (1) + AVC (7) or Interframe (2)
    video_header[1] = 0x01; // AVC NALU
    video_header[2] = 0x00; video_header[3] = 0x00; video_header[4] = 0x00; // Composition Time 0

    uint32_t total_len = (uint32_t)(len + 5);
    uint8_t chunk_header[12];
    chunk_header[0] = 0x04; // CSID 4 (Video)
    chunk_header[1] = (ts >> 16) & 0xFF; chunk_header[2] = (ts >> 8) & 0xFF; chunk_header[3] = ts & 0xFF;
    chunk_header[4] = (total_len >> 16) & 0xFF; chunk_header[5] = (total_len >> 8) & 0xFF; chunk_header[6] = total_len & 0xFF;
    chunk_header[7] = 0x09; // Video Type
    uint32_t stream_id = 1;
    memcpy(&chunk_header[8], &stream_id, 4);

    if (send(ctx->sockfd, chunk_header, 12, 0) != 12) return -1;
    if (send(ctx->sockfd, video_header, 5, 0) != 5) return -1;
    if (send(ctx->sockfd, data, len, 0) != (ssize_t)len) return -1;

    return 0;
}

int tsfi_rtmp_send_aac_header(void *ctx_ptr, uint8_t *asc, size_t asc_len) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_ptr;
    if (!ctx) return -1;

    uint8_t audio_header[2];
    audio_header[0] = 0xAF; // AAC (10) + 44kHz (3) + 16-bit (1) + Stereo (1) -> 1010 1111
    audio_header[1] = 0x00; // AAC Sequence Header

    uint32_t total_len = (uint32_t)(asc_len + 2);
    uint8_t chunk_header[12];
    chunk_header[0] = 0x04; // CSID 4 (Audio uses same as video or different?) typically 4
    chunk_header[1] = 0; chunk_header[2] = 0; chunk_header[3] = 0;
    chunk_header[4] = (total_len >> 16) & 0xFF; chunk_header[5] = (total_len >> 8) & 0xFF; chunk_header[6] = total_len & 0xFF;
    chunk_header[7] = 0x08; // Audio Type
    uint32_t stream_id = 1;
    memcpy(&chunk_header[8], &stream_id, 4);

    if (send(ctx->sockfd, chunk_header, 12, 0) != 12) return -1;
    if (send(ctx->sockfd, audio_header, 2, 0) != 2) return -1;
    if (send(ctx->sockfd, asc, asc_len, 0) != (ssize_t)asc_len) return -1;

    return 0;
}

int tsfi_rtmp_send_audio(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts) {
    TSFiRtmpContext *ctx = (TSFiRtmpContext *)ctx_ptr;
    if (!ctx) return -1;

    uint8_t audio_header[2];
    audio_header[0] = 0xAF; // AAC
    audio_header[1] = 0x01; // AAC Raw NALU

    uint32_t total_len = (uint32_t)(len + 2);
    uint8_t chunk_header[12];
    chunk_header[0] = 0x04; 
    chunk_header[1] = (ts >> 16) & 0xFF; chunk_header[2] = (ts >> 8) & 0xFF; chunk_header[3] = ts & 0xFF;
    chunk_header[4] = (total_len >> 16) & 0xFF; chunk_header[5] = (total_len >> 8) & 0xFF; chunk_header[6] = total_len & 0xFF;
    chunk_header[7] = 0x08; // Audio Type
    uint32_t stream_id = 1;
    memcpy(&chunk_header[8], &stream_id, 4);

    if (send(ctx->sockfd, chunk_header, 12, 0) != 12) return -1;
    if (send(ctx->sockfd, audio_header, 2, 0) != 2) return -1;
    if (send(ctx->sockfd, data, len, 0) != (ssize_t)len) return -1;

    return 0;
}

int tsfi_rtmp_send_pong(TSFiRtmpContext *ctx, uint32_t timestamp) {
    uint8_t payload[6];
    uint16_t type = htons(0x0007); // Pong (Type 7)
    memcpy(payload, &type, 2);
    uint32_t ts = htonl(timestamp);
    memcpy(payload + 2, &ts, 4);

    return rtmp_send_packet(ctx, 0x04, 0, payload, 6);
}

/**
 * @brief Strictly deterministic background packet listener for control messages.
 * 
 * Logic: Checks for pending RTMP control packets (Ping) and foundationally 
 * dispatches strictly deterministic Pong responses to maintain Phase 1 session.
 */
void tsfi_rtmp_poll_control(TSFiRtmpContext *ctx) {
    if (!ctx) return;

    struct pollfd pfd = { .fd = ctx->sockfd, .events = POLLIN };
    if (poll(&pfd, 1, 0) > 0) {
        uint8_t header[1];
        if (recv(ctx->sockfd, header, 1, MSG_PEEK) > 0) {
            // Check if it's a small chunk (typically control messages use CSID 2)
            uint8_t csid = header[0] & 0x3F;
            if (csid == 2) {
                // Peek more or read full header
                uint8_t full_header[12];
                if (recv(ctx->sockfd, full_header, 12, 0) == 12) {
                    uint8_t type = full_header[7];
                    uint32_t len = ((uint32_t)full_header[4] << 16) | ((uint32_t)full_header[5] << 8) | full_header[6];
                    
                    if (type == 0x04 && len >= 6) {
                        uint8_t payload[1024];
                        if (recv(ctx->sockfd, payload, len, 0) == (ssize_t)len) {
                            uint16_t sub_type = (payload[0] << 8) | payload[1];
                            if (sub_type == 0x0006) { // Ping Request
                                uint32_t ts = (payload[2] << 24) | (payload[3] << 16) | (payload[4] << 8) | payload[5];
                                tsfi_io_printf(stdout, "[RTMP] Received Ping (TS: %u), Sending Pong...\n", ts);
                                tsfi_rtmp_send_pong(ctx, ts);
                            }
                        }
                    } else if (len > 0) {
                        // Consume other control messages to prevent buffer bloat
                        uint8_t dummy[1024];
                        size_t to_read = len > 1024 ? 1024 : len;
                        recv(ctx->sockfd, dummy, to_read, 0);
                    }
                }
            }
        }
    }
}

void tsfi_rtmp_close(TSFiRtmpContext *ctx) {
    if (!ctx) return;
    close(ctx->sockfd);
    free(ctx->stream_key);
    free(ctx);
}

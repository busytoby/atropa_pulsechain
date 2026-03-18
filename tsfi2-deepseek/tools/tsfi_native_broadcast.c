#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include "tsfi_controlnet_shm.h"
#include "tsfi_io.h"

// Forward declarations from tsfi_rtmp.c
typedef struct TSFiRtmpContext TSFiRtmpContext;
extern TSFiRtmpContext* tsfi_rtmp_connect(const char *hostname, const char *stream_key);
extern int tsfi_rtmp_publish(TSFiRtmpContext *ctx);
extern int tsfi_rtmp_send_metadata(void *ctx_void, int width, int height, int fps);
extern int tsfi_rtmp_send_h264_header(void *ctx_ptr, uint8_t *sps, size_t sps_len, uint8_t *pps, size_t pps_len);
extern int tsfi_rtmp_send_video(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts);
extern void tsfi_rtmp_poll_control(TSFiRtmpContext *ctx);
extern void tsfi_rtmp_close(TSFiRtmpContext *ctx);

// Forward declarations from tsfi_soft_encode.c
extern int tsfi_soft_encode_frame(uint8_t *rgb, int w, int h, uint8_t **out_nal, size_t *out_len);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <stream_key>\n", argv[0]);
        return 1;
    }

    const char *key = argv[1];
    printf("[SYSTEM] Initializing Native C RTMP Broadcaster...\n");
    
    // Connect to BOTH Primary and Backup
    TSFiRtmpContext *ctx_a = tsfi_rtmp_connect("a.rtmp.youtube.com", key);
    TSFiRtmpContext *ctx_b = tsfi_rtmp_connect("b.rtmp.youtube.com", key);

    if (!ctx_a || !ctx_b) {
        printf("[FRACTURE] Failed to connect to YouTube Ingest Servers.\n");
        return 1;
    }

    tsfi_rtmp_publish(ctx_a);
    tsfi_rtmp_publish(ctx_b);
    
    // YouTube QoS: Mandatory Metadata
    tsfi_rtmp_send_metadata(ctx_a, 1280, 720, 24);
    tsfi_rtmp_send_metadata(ctx_b, 1280, 720, 24);

    // YouTube QoS: Mandatory H.264 Handshake Headers
    uint8_t sps[] = { 0x67, 0x42, 0xC0, 0x1F, 0xDA, 0x01, 0x40, 0x16, 0xE8, 0x06, 0xD0, 0xA1, 0x35 };
    uint8_t pps[] = { 0x68, 0xCE, 0x3C, 0x80 };
    tsfi_rtmp_send_h264_header(ctx_a, sps, sizeof(sps), pps, sizeof(pps));
    tsfi_rtmp_send_h264_header(ctx_b, sps, sizeof(sps), pps, sizeof(pps));

    printf("[PASS] Dual-Stream Active (QoS Headers Pulsed).\n");

    const char *neural_path = "/tmp/tsfi_neural_out.raw";
    size_t frame_size = 1280 * 720 * 3;
    uint8_t *rgb_buf = (uint8_t*)malloc(frame_size);

    uint32_t ts = 0;
    while (1) {
        FILE *f = fopen(neural_path, "rb");
        if (f) {
            if (fread(rgb_buf, 1, frame_size, f) == frame_size) {
                uint8_t *nal; size_t nlen;
                if (tsfi_soft_encode_frame(rgb_buf, 1280, 720, &nal, &nlen) == 0) {
                    tsfi_rtmp_send_video(ctx_a, nal, nlen, ts);
                    tsfi_rtmp_send_video(ctx_b, nal, nlen, ts);
                }
            }
            fclose(f);
        }

        tsfi_rtmp_poll_control(ctx_a);
        tsfi_rtmp_poll_control(ctx_b);

        ts += 41; // ~24 FPS
        usleep(41000); 
    }

    free(rgb_buf);
    tsfi_rtmp_close(ctx_a);
    tsfi_rtmp_close(ctx_b);
    return 0;
}

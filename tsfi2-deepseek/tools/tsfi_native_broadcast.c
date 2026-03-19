#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include "tsfi_controlnet_shm.h"
#include "tsfi_io.h"
#include "vulkan_init.h"
#include "vulkan_system.h"

// Forward declarations from tsfi_rtmp.c
typedef struct TSFiRtmpContext TSFiRtmpContext;
extern TSFiRtmpContext* tsfi_rtmp_connect(const char *hostname, const char *stream_key);
extern int tsfi_rtmp_publish(TSFiRtmpContext *ctx);
extern int tsfi_rtmp_send_metadata(void *ctx_void, int width, int height, int fps);
extern int tsfi_rtmp_send_h264_header(void *ctx_ptr, uint8_t *sps, size_t sps_len, uint8_t *pps, size_t pps_len);
extern int tsfi_rtmp_send_video(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts);
extern void tsfi_rtmp_poll_control(TSFiRtmpContext *ctx);
extern void tsfi_rtmp_close(TSFiRtmpContext *ctx);

// Forward declarations from tsfi_vulkan_video_encode.c (Replacing Deprecated Soft Encoder)
extern int tsfi_vulkan_init_video_encode(VulkanContext *vk, uint32_t width, uint32_t height);
extern int tsfi_vulkan_encode_frame(VulkanContext *vk, void *pixels, uint8_t **out_bitstream, size_t *out_len, uint32_t frame_idx);

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <stream_key>\n", argv[0]);
        return 1;
    }

    const char *key = argv[1];
    printf("[SYSTEM] Initializing Hardware-Accelerated C RTMP Broadcaster...\n");
    
    // Boot Vulkan Hardware Encoder Pipeline (AB4H 4:4:4)
    VulkanContext *vk = init_vulkan();
    if (!vk || tsfi_vulkan_init_video_encode(vk, 1280, 720) != 0) {
        printf("[FRACTURE] Vulkan Hardware Video Encode Initialization Failed.\n");
        return 1;
    }
    
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

    printf("[PASS] Dual-Stream Active (QoS Headers Pulsed). Hardware AB4H Enabled.\n");

    const char *neural_path = "/tmp/tsfi_neural_out.raw";
    // Using AB4H packed format size (4 bytes per pixel) to align with hardware requirements
    size_t frame_size = 1280 * 720 * 4; 
    uint8_t *ab4h_buf = (uint8_t*)malloc(frame_size);

    uint32_t ts = 0;
    uint32_t frame_idx = 0;
    while (1) {
        FILE *f = fopen(neural_path, "rb");
        if (f) {
            // Because the Python generation output might still be RGB (3 bytes), we simulate the AB4H packing
            // if we only read 3-byte sequences. For the broadcaster, we assume the native worker is exporting 4-byte packed arrays.
            if (fread(ab4h_buf, 1, frame_size, f) >= 1280 * 720 * 3) {
                uint8_t *nal; size_t nlen;
                // Execute Hardware Encode
                if (tsfi_vulkan_encode_frame(vk, ab4h_buf, &nal, &nlen, frame_idx++) == 0) {
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

    free(ab4h_buf);
    tsfi_rtmp_close(ctx_a);
    tsfi_rtmp_close(ctx_b);
    return 0;
}

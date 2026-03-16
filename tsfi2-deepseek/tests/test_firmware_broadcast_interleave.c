#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* rtmp_ctx;
    int (*vk_rtmp_send_video)(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts);
    int (*vk_rtmp_send_audio)(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts);
} RtmpInterleaveDispatch;

static int mock_video_packets = 0;
static int mock_audio_packets = 0;

int mock_send_video(void *ctx, uint8_t *data, size_t len, uint32_t ts) {
    (void)ctx; (void)data; (void)len; (void)ts;
    mock_video_packets++;
    return 0;
}

int mock_send_audio(void *ctx, uint8_t *data, size_t len, uint32_t ts) {
    (void)ctx; (void)data; (void)len; (void)ts;
    mock_audio_packets++;
    return 0;
}

#include "../thunks/tsfi_broadcast_interleave_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Broadcast Interleave Thunk Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    RtmpInterleaveDispatch dispatch;
    dispatch.rtmp_ctx = (void*)0x9999;
    dispatch.vk_rtmp_send_video = mock_send_video;
    dispatch.vk_rtmp_send_audio = mock_send_audio;

    uint8_t* audio_buf = (uint8_t*)lau_malloc_wired(4096);
    uint8_t* video_buf = (uint8_t*)lau_malloc_wired(1024);

    regs.sgpr[0] = (uint32_t)((uintptr_t)&dispatch & 0xFFFFFFFF);
    regs.sgpr[1] = (uint32_t)((uintptr_t)&dispatch >> 32);
    
    regs.sgpr[2] = (uint32_t)((uintptr_t)audio_buf & 0xFFFFFFFF);
    regs.sgpr[3] = (uint32_t)((uintptr_t)audio_buf >> 32);
    
    regs.sgpr[4] = (uint32_t)((uintptr_t)video_buf & 0xFFFFFFFF);
    regs.sgpr[5] = (uint32_t)((uintptr_t)video_buf >> 32);

    regs.sgpr[6] = 4096; 
    regs.sgpr[7] = 0;    
    regs.sgpr[8] = 0;    
    regs.sgpr[9] = 1024; 
    regs.sgpr[10] = 0;   

    tsfi_broadcast_interleave_thunk(&regs);

    if (regs.sgpr[15] != 1) {
        tsfi_io_printf(stderr, "[FAIL] Thunk failed to engage. Hardware Fault.\n");
        return 1;
    }

    if (mock_audio_packets != 1 || mock_video_packets != 1) {
        tsfi_io_printf(stderr, "[FAIL] Packets did not interleave correctly. (Audio: %d, Video: %d)\n", mock_audio_packets, mock_video_packets);
        return 1;
    }

    if (regs.sgpr[6] != 3072 || regs.sgpr[7] != 1024 || regs.sgpr[8] != 23) {
        tsfi_io_printf(stderr, "[FAIL] Audio geometric bounds mutated incorrectly.\n");
        return 1;
    }

    if (regs.sgpr[9] != 0 || regs.sgpr[10] != 33) {
        tsfi_io_printf(stderr, "[FAIL] Video buffer did not clear geometrically.\n");
        return 1;
    }

    tsfi_io_printf(stdout, "[VERIFIED] Thunk safely interleaved and exported strictly mutated memory pointers.\n");

    lau_free(audio_buf);
    lau_free(video_buf);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
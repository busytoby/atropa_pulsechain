#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* rtmp_ctx;
    const char* stream_key;
    const char* rtmp_url;
    void* (*vk_rtmp_connect)(const char *hostname, const char *stream_key);
    int (*vk_rtmp_publish)(void *ctx);
    int (*vk_rtmp_send_metadata)(void *ctx, int w, int h, int fps);
} RtmpStateDispatch;

static int mock_connect_calls = 0;
static int mock_publish_calls = 0;

void* mock_connect(const char *hostname, const char *stream_key) {
    (void)hostname; (void)stream_key;
    mock_connect_calls++;
    if (mock_connect_calls < 2) return NULL; 
    return (void*)0x8888;
}

int mock_publish(void *ctx) {
    (void)ctx;
    mock_publish_calls++;
    return 0;
}

#include "../thunks/tsfi_broadcast_reconnect_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Broadcast Reconnect Thunk Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    RtmpStateDispatch dispatch;
    dispatch.rtmp_ctx = NULL;
    dispatch.stream_key = "abc-123";
    dispatch.rtmp_url = "rtmp://mock";
    dispatch.vk_rtmp_connect = mock_connect;
    dispatch.vk_rtmp_publish = mock_publish;
    dispatch.vk_rtmp_send_metadata = NULL;

    regs.sgpr[0] = (uint32_t)((uintptr_t)&dispatch & 0xFFFFFFFF);
    regs.sgpr[1] = (uint32_t)((uintptr_t)&dispatch >> 32);

    regs.sgpr[2] = 0; 
    tsfi_broadcast_reconnect_thunk(&regs);
    
    if (regs.sgpr[2] != 1) {
        tsfi_io_printf(stderr, "[FAIL] Failed to transition IDLE -> CONNECTING\n");
        return 1;
    }

    regs.sgpr[6] = 0x99AABBCC; 
    tsfi_broadcast_reconnect_thunk(&regs);

    if (regs.sgpr[2] != 4 || regs.sgpr[3] != 1 || regs.sgpr[4] == 0) {
        tsfi_io_printf(stderr, "[FAIL] Failed to execute backoff logic properly.\n");
        return 1;
    }
    
    uint32_t wait_epoch = regs.sgpr[4];

    regs.sgpr[5] = wait_epoch - 5; 
    tsfi_broadcast_reconnect_thunk(&regs);
    if (regs.sgpr[2] != 4) {
         tsfi_io_printf(stderr, "[FAIL] Backoff epoch failed to hold state.\n");
         return 1;
    }

    regs.sgpr[5] = wait_epoch + 10;
    tsfi_broadcast_reconnect_thunk(&regs);
    if (regs.sgpr[2] != 1) {
         tsfi_io_printf(stderr, "[FAIL] Epoch maturity failed to re-trigger CONNECTING.\n");
         return 1;
    }

    tsfi_broadcast_reconnect_thunk(&regs);
    if (regs.sgpr[2] != 2) {
         tsfi_io_printf(stderr, "[FAIL] CONNECTING failed to secure valid RTMP socket pointer.\n");
         return 1;
    }

    tsfi_broadcast_reconnect_thunk(&regs);
    if (regs.sgpr[2] != 3 || mock_publish_calls != 1) {
         tsfi_io_printf(stderr, "[FAIL] PUBLISHING failed to transition to ACTIVE.\n");
         return 1;
    }

    tsfi_io_printf(stdout, "[VERIFIED] Thunk safely executed full state matrix solely via hardware epochs and geometric entropy.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
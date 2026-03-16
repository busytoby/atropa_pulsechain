#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* rtmp_ctx;
    int (*vk_rtmp_poll_control)(void* ctx);
    int (*vk_rtmp_send_ping)(void* ctx, uint32_t timestamp);
} RtmpDispatchTable;

static int mock_polls = 0;
static int mock_pings = 0;

int mock_poll_control(void* ctx) { (void)ctx; mock_polls++; return 0; }
int mock_send_ping(void* ctx, uint32_t ts) { (void)ctx; (void)ts; mock_pings++; return 0; }

#include "../thunks/tsfi_broadcast_keepalive_thunk.c"

int main() {
    tsfi_io_printf(stdout, "=== TSFi Firmware: Broadcast Keepalive Thunk Validation ===\n");
    
    LauRegisterBank regs;
    memset(&regs, 0, sizeof(LauRegisterBank));

    RtmpDispatchTable dispatch;
    dispatch.rtmp_ctx = (void*)0x8888;
    dispatch.vk_rtmp_poll_control = mock_poll_control;
    dispatch.vk_rtmp_send_ping = mock_send_ping;

    regs.sgpr[12] = (uint32_t)((uintptr_t)&dispatch & 0xFFFFFFFF);
    regs.sgpr[13] = (uint32_t)((uintptr_t)&dispatch >> 32);

    regs.sgpr[11] = 2999;
    tsfi_broadcast_keepalive_thunk(&regs);

    if (regs.sgpr[0] != 1 || regs.sgpr[1] != 0 || mock_polls != 1 || mock_pings != 0) {
        tsfi_io_printf(stderr, "[FAIL] Passive keepalive evaluation failed.\n");
        return 1;
    }
    tsfi_io_printf(stdout, "[VERIFIED] Passive Epoch %u successfully polled without executing Active Ping.\n", regs.sgpr[11]);

    regs.sgpr[11] = 3000;
    tsfi_broadcast_keepalive_thunk(&regs);

    if (regs.sgpr[0] != 1 || regs.sgpr[1] != 1 || mock_polls != 2 || mock_pings != 1) {
        tsfi_io_printf(stderr, "[FAIL] Active keepalive boundary failed to pulse.\n");
        return 1;
    }
    tsfi_io_printf(stdout, "[VERIFIED] Active Epoch %u successfully engaged exact hardware Ping Strobe.\n", regs.sgpr[11]);

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
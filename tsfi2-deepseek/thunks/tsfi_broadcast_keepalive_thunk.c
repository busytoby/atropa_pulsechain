#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"

// The RTMP Hardware Dispatch Table
typedef struct {
    void* rtmp_ctx;
    int (*vk_rtmp_poll_control)(void* ctx);
    int (*vk_rtmp_send_ping)(void* ctx, uint32_t timestamp);
} RtmpDispatchTable;

/**
 * TSFi Firmware Standard Cell: Broadcast Keepalive Thunk
 * 
 * This standard cell natively embeds the RTMP heartbeat and control polling 
 * directly into the hardware execution loop. It eradicates passive software 
 * timeouts by actively pulsing the network strictly according to the 
 * Verilog clock epoch.
 */
void tsfi_broadcast_keepalive_thunk(LauRegisterBank* regs) {
    if (!regs) return;

    // sgpr[11]: Current RTL Strobe Cycle / YI Epoch
    // sgpr[12/13]: Pointer to RtmpDispatchTable
    
    uint32_t current_epoch = regs->sgpr[11];
    uintptr_t dispatch_ptr = ((uintptr_t)regs->sgpr[13] << 32) | (uintptr_t)regs->sgpr[12];
    RtmpDispatchTable* dispatch = (RtmpDispatchTable*)dispatch_ptr;

    if (!dispatch || !dispatch->rtmp_ctx || !dispatch->vk_rtmp_poll_control) {
        regs->sgpr[0] = 0; // Hardware Fault: Null Dispatch Table
        return;
    }

    // 1. Reactive Polling (Drain incoming Server Pings)
    int poll_status = dispatch->vk_rtmp_poll_control(dispatch->rtmp_ctx);
    if (poll_status < 0) {
        regs->sgpr[0] = 2; // Critical: Socket Collapse / Remote Disconnect
        return;
    }

    // 2. Sovereign Active Heartbeat
    // We physically force a heartbeat transmission exactly every 3000 clock cycles.
    // This physically prevents the ingest server from terminating a structurally quiet stream.
    if (current_epoch % 3000 == 0) {
        if (dispatch->vk_rtmp_send_ping) {
            int ping_status = dispatch->vk_rtmp_send_ping(dispatch->rtmp_ctx, current_epoch);
            if (ping_status < 0) {
                regs->sgpr[0] = 2; // Critical: Ping Transmission Failure
                return;
            }
            regs->sgpr[1] = 1; // Flag: Active Heartbeat Executed
        }
    } else {
        regs->sgpr[1] = 0; // Flag: Passive Poll Executed
    }

    regs->sgpr[0] = 1; // Success: Matrix Stable
}
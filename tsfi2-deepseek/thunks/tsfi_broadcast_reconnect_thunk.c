#include "tsfi_wire_firmware.h"

// RTMP Hardware Dispatch Table
typedef struct {
    void* rtmp_ctx;
    const char* stream_key;
    const char* rtmp_url;
    void* (*vk_rtmp_connect)(const char *hostname, const char *stream_key);
    int (*vk_rtmp_publish)(void *ctx);
    int (*vk_rtmp_send_metadata)(void *ctx, int w, int h, int fps);
} RtmpStateDispatch;

// Hardware Epoch Conversion (Assuming 120 epochs per ms equivalent for simulation bounds)
#define EPOCHS_PER_MS 120

static uint32_t tsfi_broadcaster_calculate_backoff_epochs(int attempts, uint32_t daiichi_entropy) {
    if (attempts <= 0) return 0;
    
    // Base Exponential Backoff (seconds -> ms)
    uint32_t base_sec = 1 << (attempts - 1);
    if (base_sec > 32) base_sec = 32;
    uint32_t base_ms = base_sec * 1000;

    // Cryptographic Deterministic Jitter (+/- 10%)
    uint32_t variance_ms = (base_ms / 10); 
    
    uint32_t final_ms;
    if (daiichi_entropy & 0x1) {
        final_ms = base_ms + (daiichi_entropy % variance_ms);
    } else {
        final_ms = base_ms - (daiichi_entropy % variance_ms);
    }
    
    return final_ms * EPOCHS_PER_MS;
}

/**
 * TSFi Firmware Standard Cell: Broadcast Reconnect State
 * 
 * Physically orchestrates RTMP state transitions (IDLE->CONNECT->PUBLISH->ACTIVE)
 * using strict hardware Verilog epochs and geometric Daiichi entropy.
 */
void tsfi_broadcast_reconnect_thunk(LauRegisterBank* regs) {
    if (!regs) return;

    // Boundary Extraction
    uintptr_t dispatch_ptr = ((uintptr_t)regs->sgpr[1] << 32) | (uintptr_t)regs->sgpr[0];
    RtmpStateDispatch* dispatch = (RtmpStateDispatch*)dispatch_ptr;

    if (!dispatch || !dispatch->vk_rtmp_connect) {
        regs->sgpr[15] = 0; // Fault
        return;
    }

    uint32_t state = regs->sgpr[2];
    uint32_t attempt_count = regs->sgpr[3];
    uint32_t next_retry_epoch = regs->sgpr[4];
    uint32_t current_epoch = regs->sgpr[5];
    uint32_t daiichi_entropy = regs->sgpr[6];

    // State Transitions
    // 0: IDLE, 1: CONNECTING, 2: PUBLISHING, 3: ACTIVE, 4: WAITING_RECONNECT
    switch (state) {
        case 0: // IDLE
            state = 1;
            attempt_count = 0;
            break;

        case 1: // CONNECTING
            dispatch->rtmp_ctx = dispatch->vk_rtmp_connect(dispatch->rtmp_url, dispatch->stream_key);
            
            if (dispatch->rtmp_ctx) {
                state = 2; // PUBLISHING
            } else {
                attempt_count++;
                uint32_t backoff_epochs = tsfi_broadcaster_calculate_backoff_epochs(attempt_count, daiichi_entropy);
                next_retry_epoch = current_epoch + backoff_epochs;
                state = 4; // WAITING_RECONNECT
            }
            break;

        case 2: // PUBLISHING
            if (dispatch->vk_rtmp_publish && dispatch->vk_rtmp_publish(dispatch->rtmp_ctx) == 0) {
                if (dispatch->vk_rtmp_send_metadata) {
                    dispatch->vk_rtmp_send_metadata(dispatch->rtmp_ctx, 1280, 720, 30);
                }
                state = 3; // ACTIVE
                attempt_count = 0;
            } else {
                state = 0; // Fail back to IDLE
            }
            break;

        case 4: // WAITING_RECONNECT
            if (current_epoch >= next_retry_epoch) {
                state = 1; // Retry connection
            }
            break;

        case 3: // ACTIVE
            // Assume active until Host intercepts a socket fault and overrides the state
            if (!dispatch->rtmp_ctx) {
                state = 1;
            }
            break;

        default:
            state = 0;
            break;
    }

    // Export mutated state variables
    regs->sgpr[2] = state;
    regs->sgpr[3] = attempt_count;
    regs->sgpr[4] = next_retry_epoch;
    
    // Export active context pointer bounds back to Host
    regs->sgpr[8] = (uint32_t)((uintptr_t)dispatch->rtmp_ctx & 0xFFFFFFFF);
    regs->sgpr[9] = (uint32_t)((uintptr_t)dispatch->rtmp_ctx >> 32);

    regs->sgpr[15] = 1; // Valid Execution
}
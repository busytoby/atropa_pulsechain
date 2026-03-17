#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"

// The RTMP Hardware Dispatch Table
typedef struct {
    void* rtmp_ctx;
    int (*vk_rtmp_send_video)(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts);
    int (*vk_rtmp_send_audio)(void *ctx_ptr, uint8_t *data, size_t len, uint32_t ts);
} RtmpInterleaveDispatch;

/**
 * TSFi Firmware Standard Cell: Broadcast Interleave Thunk
 * 
 * Physically orchestrates AV multiplexing across the network boundary 
 * directly from within the `YI` execution matrix, utilizing zero-copy 
 * linear read pointer mechanics.
 */
void tsfi_broadcast_interleave_thunk(LauRegisterBank* regs) {
    if (!regs) return;

    // SGPR Memory Boundary Mapping
    uintptr_t dispatch_ptr = ((uintptr_t)regs->sgpr[1] << 32) | (uintptr_t)regs->sgpr[0];
    uintptr_t audio_buf_ptr = ((uintptr_t)regs->sgpr[3] << 32) | (uintptr_t)regs->sgpr[2];
    uintptr_t video_buf_ptr = ((uintptr_t)regs->sgpr[5] << 32) | (uintptr_t)regs->sgpr[4];

    RtmpInterleaveDispatch* dispatch = (RtmpInterleaveDispatch*)dispatch_ptr;
    uint8_t* audio_buffer = (uint8_t*)audio_buf_ptr;
    uint8_t* video_buffer = (uint8_t*)video_buf_ptr;

    size_t audio_size = regs->sgpr[6];
    size_t audio_read_offset = regs->sgpr[7];
    uint32_t audio_sample_epoch = regs->sgpr[8];
    
    size_t video_size = regs->sgpr[9];
    uint32_t stream_epoch = regs->sgpr[10];

    if (!dispatch || !dispatch->rtmp_ctx || !dispatch->vk_rtmp_send_audio || !dispatch->vk_rtmp_send_video) {
        regs->sgpr[15] = 0; // Hardware Fault: Null Vector Route
        return;
    }

    // 1. Drain Audio Pipeline (Zero-Copy Interleave)
    while (audio_size > 0 && audio_sample_epoch <= stream_epoch) {
        size_t chunk_len = audio_size > 1024 ? 1024 : audio_size;
        
        if (dispatch->vk_rtmp_send_audio(dispatch->rtmp_ctx, audio_buffer + audio_read_offset, chunk_len, audio_sample_epoch) == 0) {
            audio_sample_epoch += 23; // Geometric clock increment
            audio_size -= chunk_len;
            audio_read_offset += chunk_len;
            
            // Structural memory compaction signal (handled by Host via returned offset)
            if (audio_size == 0) {
                audio_read_offset = 0;
            }
        } else {
            break; // Network layer blockage, halt processing
        }
    }

    // 2. Dispatch Video Frame
    if (video_size > 0) {
        if (dispatch->vk_rtmp_send_video(dispatch->rtmp_ctx, video_buffer, video_size, stream_epoch) == 0) {
            stream_epoch += 33; // Geometric clock advance
            video_size = 0; // Buffer entirely consumed
        }
    }

    // Export mutated state variables back across the register boundary
    regs->sgpr[6] = (uint32_t)audio_size;
    regs->sgpr[7] = (uint32_t)audio_read_offset;
    regs->sgpr[8] = audio_sample_epoch;
    regs->sgpr[9] = (uint32_t)video_size;
    regs->sgpr[10] = stream_epoch;
    
    regs->sgpr[15] = 1; // Execution Valid
}
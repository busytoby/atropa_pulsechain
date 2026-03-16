#ifndef TSFI_BROADCASTER_H
#define TSFI_BROADCASTER_H

#include "tsfi_types.h"
#include "tsfi_wire_pty.h"
#include "../plugins/vulkan/vulkan_system.h"
#include <stdatomic.h>

#define BROADCASTER_COLS 160
#define BROADCASTER_ROWS 60

typedef enum {
    LB_STATE_IDLE = 0,
    LB_STATE_CONNECTING,
    LB_STATE_HANDSHAKING,
    LB_STATE_PUBLISHING,
    LB_STATE_ACTIVE,
    LB_STATE_WAITING_RECONNECT,
    LB_STATE_ERROR
} LauBroadcasterState;

/**
 * @brief LauBroadcaster: A Resident Wavefront for RTMP Streaming.
 */
DEFINE_MAPPED_STRUCT(LauBroadcaster,
    // Network Layer (RTMP)
    int rtmp_sockfd;
    void *rtmp_ctx;             // TSFiRtmpContext pointer
    uint32_t stream_epoch;
    char stream_key[128];
    LauBroadcasterState connection_state;
    int attempt_count;
    uint64_t next_retry_ns;

    // Ingest Layer (PTY)
    int pty_master_fd;
    LauWirePty *pty_wire;
    uint32_t char_BASE[BROADCASTER_ROWS][BROADCASTER_COLS];
    uint8_t color_BASE[BROADCASTER_ROWS][BROADCASTER_COLS];
    bool char_BASE_dirty[BROADCASTER_ROWS][BROADCASTER_COLS];
    bool is_dirty;
    int cursor_x;
    int cursor_y;

    // Visual Layer (Vulkan)
    VulkanSystem *v_sys;        // Linkage to active manifold
    void *vulkan_image_handle;
    void *vulkan_memory_handle;
    uint64_t *gpu_sem_addr;     // Zhong Timeline Sync
    uint64_t gpu_sem_target;

    // Vulkan Video Encode Handles
    VkVideoSessionKHR           video_session;
    VkVideoSessionParametersKHR video_params;
    VkVideoProfileInfoKHR       video_profile;
    VkBuffer                    bitstream_vk_buffer;
    VkDeviceMemory              bitstream_vk_memory;

    // Bitstream Buffer (Encoded H.264)
    uint8_t *bitstream_buffer;
    size_t bitstream_size;

    // Audio Layer (AAC/PCM - Internal Synthesis)
    uint8_t *audio_buffer;
    size_t audio_size;
    size_t audio_read_offset; // Zero-copy linear read pointer
    uint32_t audio_sample_epoch; // Sample-based timestamp

    // Pulse/Intensity
    float broadcast_intensity;  // 1.0 = Active, 0.0 = At Rest
    bool stream_active;
    bool soft_encode_active;    // Fallback to AVX-512 soft encoding

    // Telemetry
    float current_fps;
    float current_latency_ms;
    int reconnect_count;

    // Performance Thunk State
    float cpu_load;
    float gpu_load;
    uint64_t last_perf_update_ns;

    // Bandwidth Throttling (Physical Context)
    uint64_t bytes_sent_window;
    uint64_t window_start_ns;
    uint64_t target_bitrate_bps; // e.g. 2500000 for 2.5 Mbps

    // Logging Layer
    int session_log_fd;
)

/**
 * @brief Initialize and wire the broadcaster into the Helmholtz List.
 */
LauBroadcaster* lau_wire_broadcaster(const char *stream_key, void *v_sys, pid_t *out_pid);

/**
 * @brief Update the terminal BASE with PTY output.
 */
void vulkan_broadcaster_update_BASE(LauBroadcaster *lb, const char *buf, size_t n);

/**
 * @brief Global accessor for active broadcaster.
 */
LauBroadcaster* get_active_broadcaster(void);

/**
 * @brief Execute Performance Benchmarking Thunk.
 */
void tsfi_broadcaster_perf_update(LauBroadcaster *lb);

/**
 * @brief Mark a character cell as dirty.
 */
void tsfi_broadcaster_dirty_char(LauBroadcaster *lb, int x, int y);

/**
 * @brief Rasterize Telemetry Overlay onto the BASE.
 */
void tsfi_broadcaster_ui_draw(LauBroadcaster *lb);

/**
 * @brief Foundational Session Logging.
 */
void tsfi_broadcaster_log_write(LauBroadcaster *lb, const char *buf, size_t n);

#endif // TSFI_BROADCASTER_H

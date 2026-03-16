#include "tsfi_broadcaster.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_io.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/mman.h>
#include <pty.h>
#include <fcntl.h>
#include <utmp.h>
#include <stdlib.h>
#include <errno.h>

/**
 * @brief Broadcaster Logic: State Phase (Coordination)
 */
static bool broadcaster_logic_state(void *obj) {
    LauBroadcaster *lb = (LauBroadcaster *)obj;
    bool active = lb->stream_active && (lb->broadcast_intensity > 0.1f);
    tsfi_io_printf(stdout, "[BROADCAST] State called, returning %d\n", active);
    return active;
}

/**
 * @brief Initialize Vulkan Video Encode Session (H.264).
 */
static int vulkan_video_encode_init(LauBroadcaster *lb) {
    if (!lb->v_sys || !lb->v_sys->vk) return -1;
    VulkanContext *vk = lb->v_sys->vk;

    lb->video_profile.sType = VK_STRUCTURE_TYPE_VIDEO_PROFILE_INFO_KHR;
    lb->video_profile.videoCodecOperation = VK_VIDEO_CODEC_OPERATION_ENCODE_H264_BIT_KHR;
    lb->video_profile.chromaSubsampling = VK_VIDEO_CHROMA_SUBSAMPLING_420_BIT_KHR;
    lb->video_profile.lumaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;
    lb->video_profile.chromaBitDepth = VK_VIDEO_COMPONENT_BIT_DEPTH_8_BIT_KHR;

    VkVideoSessionCreateInfoKHR sessionInfo = {
        .sType = VK_STRUCTURE_TYPE_VIDEO_SESSION_CREATE_INFO_KHR,
        .pVideoProfile = &lb->video_profile,
        .maxCodedExtent = { 1280, 720 },
        .referencePictureFormat = VK_FORMAT_G8_B8R8_2PLANE_420_UNORM,
        .maxDpbSlots = 1,
        .maxActiveReferencePictures = 0
    };

    typedef VkResult (VKAPI_PTR *PFN_vkCreateVideoSessionKHR)(VkDevice, const VkVideoSessionCreateInfoKHR*, const VkAllocationCallbacks*, VkVideoSessionKHR*);
    PFN_vkCreateVideoSessionKHR pvkCreateVideoSessionKHR = (PFN_vkCreateVideoSessionKHR)vkGetInstanceProcAddr(NULL, "vkCreateVideoSessionKHR");
    
    if (!pvkCreateVideoSessionKHR || pvkCreateVideoSessionKHR(vk->device, &sessionInfo, NULL, &lb->video_session) != VK_SUCCESS) {
        return -1;
    }
    return 0;
}

/**
 * @brief Transduce BASE to H.264 bitstream.
 */
static void broadcaster_encode_frame(LauBroadcaster *lb) {
    if (!lb->v_sys || !lb->rtmp_ctx) return;
    
    if (lb->stream_epoch == 0) {
        uint8_t sps[] = { 0x67, 0x42, 0xC0, 0x1F, 0xDA, 0x01, 0x40, 0x16, 0xE8, 0x06, 0xD0, 0xA1, 0x35 };
        uint8_t pps[] = { 0x68, 0xCE, 0x3C, 0x80 };
        extern int tsfi_rtmp_send_h264_header(void *ctx, uint8_t *sps, size_t slen, uint8_t *pps, size_t plen);
        tsfi_rtmp_send_h264_header(lb->rtmp_ctx, sps, sizeof(sps), pps, sizeof(pps));
    }

    if (lb->soft_encode_active) {
        extern void tsfi_soft_encode_frame(LauBroadcaster *lb);
        tsfi_soft_encode_frame(lb);
    }
    
    extern int tsfi_rtmp_send_video(void *ctx, uint8_t *data, size_t len, uint32_t ts);
    if (lb->bitstream_size > 0 && lb->bitstream_buffer) {
        tsfi_rtmp_send_video(lb->rtmp_ctx, lb->bitstream_buffer, lb->bitstream_size, lb->stream_epoch * 33);
    }
}

void tsfi_broadcaster_dirty_char(LauBroadcaster *lb, int x, int y) {
    if (x >= 0 && x < BROADCASTER_COLS && y >= 0 && y < BROADCASTER_ROWS) {
        lb->char_BASE_dirty[y][x] = true;
        lb->is_dirty = true;
    }
}

static void dirty_char_if_changed(LauBroadcaster *lb, int x, int y, uint32_t new_cp, uint8_t new_color) {
    if (x >= 0 && x < BROADCASTER_COLS && y >= 0 && y < BROADCASTER_ROWS) {
        if (lb->char_BASE[y][x] != new_cp || lb->color_BASE[y][x] != new_color) {
            lb->char_BASE[y][x] = new_cp;
            lb->color_BASE[y][x] = new_color;
            tsfi_broadcaster_dirty_char(lb, x, y);
        }
    }
}

void vulkan_broadcaster_update_BASE(LauBroadcaster *lb, const char *buf, size_t n) {
    static int state = 0; 
    static int vals[4] = {0};
    static int val_idx = 0;
    static uint32_t utf8_code = 0;
    static int utf8_state = 0; 
    static uint8_t current_color = 0x07; // Default: White on Black

    int old_cx = lb->cursor_x;
    int old_cy = lb->cursor_y;

    for (size_t i = 0; i < n; i++) {
        unsigned char c = buf[i];
        
        if (state == 0) {
            if (c == 27) { state = 1; val_idx = 0; memset(vals, 0, sizeof(vals)); utf8_state = 0; }
            else if (c == '\n') { lb->cursor_y++; utf8_state = 0; }
            else if (c == '\r') { lb->cursor_x = 0; utf8_state = 0; }
            else if (c == '\b') { if (lb->cursor_x > 0) lb->cursor_x--; utf8_state = 0; }
            else if (c == '\t') { lb->cursor_x = (lb->cursor_x + 8) & ~7; utf8_state = 0; }
            else {
                if (utf8_state == 0) {
                    if (c < 0x80) { utf8_code = c; utf8_state = 0; }
                    else if ((c & 0xE0) == 0xC0) { utf8_code = c & 0x1F; utf8_state = 1; }
                    else if ((c & 0xF0) == 0xE0) { utf8_code = c & 0x0F; utf8_state = 2; }
                    else if ((c & 0xF8) == 0xF0) { utf8_code = c & 0x07; utf8_state = 3; }
                } else {
                    if ((c & 0xC0) == 0x80) { utf8_code = (utf8_code << 6) | (c & 0x3F); utf8_state--; }
                    else { utf8_code = '?'; utf8_state = 0; }
                }

                if (utf8_state == 0 && utf8_code >= 32) {
                    if (lb->cursor_y >= 0 && lb->cursor_y < BROADCASTER_ROWS && 
                        lb->cursor_x >= 0 && lb->cursor_x < BROADCASTER_COLS) {
                        
                        // Strict Geometry Masking
                        if (utf8_code > 0xFFFF) utf8_code = '?'; // Cap for safety
                        
                        dirty_char_if_changed(lb, lb->cursor_x, lb->cursor_y, utf8_code, current_color);
                        lb->cursor_x++;
                    }
                }
            }
        } else if (state == 1) {
            if (c == '[') state = 2;
            else if (c == ']') state = 3;
            else state = 0;
        } else if (state == 2) { 
            if (c >= '0' && c <= '9') { vals[val_idx] = vals[val_idx] * 10 + (c - '0'); }
            else if (c == ';') { if (val_idx < 3) val_idx++; }
            else {
                if (c == 'H' || c == 'f') {
                    lb->cursor_y = (vals[0] > 0) ? (vals[0] - 1) : 0;
                    lb->cursor_x = (vals[1] > 0) ? (vals[1] - 1) : 0;
                } else if (c == 'G') { 
                    lb->cursor_x = (vals[0] > 0) ? (vals[0] - 1) : 0;
                } else if (c == 'd') { 
                    lb->cursor_y = (vals[0] > 0) ? (vals[0] - 1) : 0;
                } else if (c == 'X') { 
                    int n_erase = vals[0] ? vals[0] : 1;
                    if (lb->cursor_y >= 0 && lb->cursor_y < BROADCASTER_ROWS) {
                        for (int x = lb->cursor_x; x < lb->cursor_x + n_erase && x < BROADCASTER_COLS; x++) {
                            dirty_char_if_changed(lb, x, lb->cursor_y, ' ', current_color);
                        }
                    }
                } else if (c == 'J') {
                    if (vals[0] == 2) { 
                        for (int y = 0; y < BROADCASTER_ROWS; y++) {
                            for (int x = 0; x < BROADCASTER_COLS; x++) {
                                dirty_char_if_changed(lb, x, y, ' ', current_color);
                            }
                        }
                        lb->cursor_x = 0; lb->cursor_y = 0;
                    } else if (vals[0] == 0) { 
                        if (lb->cursor_y >= 0 && lb->cursor_y < BROADCASTER_ROWS) {
                            for (int x = lb->cursor_x; x < BROADCASTER_COLS; x++) dirty_char_if_changed(lb, x, lb->cursor_y, ' ', current_color);
                            for (int y = lb->cursor_y + 1; y < BROADCASTER_ROWS; y++) {
                                for (int x = 0; x < BROADCASTER_COLS; x++) dirty_char_if_changed(lb, x, y, ' ', current_color);
                            }
                        }
                    }
                } else if (c == 'K') {
                    if (vals[0] == 0) { 
                        if (lb->cursor_y >= 0 && lb->cursor_y < BROADCASTER_ROWS) {
                            for (int x = lb->cursor_x; x < BROADCASTER_COLS; x++) dirty_char_if_changed(lb, x, lb->cursor_y, ' ', current_color);
                        }
                    } else if (vals[0] == 2) { 
                        if (lb->cursor_y >= 0 && lb->cursor_y < BROADCASTER_ROWS) {
                            for (int x = 0; x < BROADCASTER_COLS; x++) dirty_char_if_changed(lb, x, lb->cursor_y, ' ', current_color);
                        }
                    }
                } else if (c == 'A') { lb->cursor_y -= vals[0] ? vals[0] : 1; }
                else if (c == 'B') { lb->cursor_y += vals[0] ? vals[0] : 1; }
                else if (c == 'C') { lb->cursor_x += vals[0] ? vals[0] : 1; }
                else if (c == 'D') { lb->cursor_x -= vals[0] ? vals[0] : 1; }
                else if (c == 'm') { 
                    // Select Graphic Rendition
                    for (int v = 0; v <= val_idx; v++) {
                        int code = vals[v];
                        if (code == 0) current_color = 0x07; // Reset
                        else if (code >= 30 && code <= 37) current_color = (current_color & 0xF0) | (code - 30); // FG
                        else if (code >= 40 && code <= 47) current_color = (current_color & 0x0F) | ((code - 40) << 4); // BG
                        else if (code == 1) current_color |= 0x08; // Bold flag
                    }
                }
                state = 0;
            }
        } else if (state == 3) { 
            if (c == 27 || c == '\a' || c == '\\') state = 0;
        }

        if (lb->cursor_x >= BROADCASTER_COLS) { lb->cursor_x = 0; lb->cursor_y++; }
        if (lb->cursor_x < 0) lb->cursor_x = 0;
        
        while (lb->cursor_y >= BROADCASTER_ROWS) {
            for (int row = 1; row < BROADCASTER_ROWS; row++) {
                for (int x = 0; x < BROADCASTER_COLS; x++) {
                    uint32_t next_cp = lb->char_BASE[row][x];
                    uint8_t next_color = lb->color_BASE[row][x];
                    if (lb->char_BASE[row - 1][x] != next_cp || lb->color_BASE[row - 1][x] != next_color) {
                        lb->char_BASE[row - 1][x] = next_cp;
                        lb->color_BASE[row - 1][x] = next_color;
                        tsfi_broadcaster_dirty_char(lb, x, row - 1);
                    }
                }
            }
            for (int x = 0; x < BROADCASTER_COLS; x++) {
                if (lb->char_BASE[BROADCASTER_ROWS - 1][x] != ' ' || lb->color_BASE[BROADCASTER_ROWS - 1][x] != current_color) {
                    lb->char_BASE[BROADCASTER_ROWS - 1][x] = ' ';
                    lb->color_BASE[BROADCASTER_ROWS - 1][x] = current_color;
                    tsfi_broadcaster_dirty_char(lb, x, BROADCASTER_ROWS - 1);
                }
            }
            lb->cursor_y--;
        }
        if (lb->cursor_y < 0) lb->cursor_y = 0;
    }

    tsfi_broadcaster_dirty_char(lb, old_cx, old_cy);
    tsfi_broadcaster_dirty_char(lb, lb->cursor_x, lb->cursor_y);
}

/**
 * @brief Broadcaster Logic: Epoch Phase (Execution)
 */
static void broadcaster_logic_epoch(void *obj) {
    LauBroadcaster *lb = (LauBroadcaster *)obj;
    tsfi_io_printf(stdout, "[BROADCAST] Epoch called\n");
    
    // 1. Dispatch PTY Thread (Autonomous Subshell)
    if (lb->pty_wire) {
        static int dp_count = 0;
        if (dp_count++ % 100 == 0) {
            tsfi_io_printf(stdout, "[BROADCAST] Dispatching pty_wire: %p, fn: %p\n", (void*)lb->pty_wire, (void*)lb->pty_wire->step_safety_epoch);
        }
        if (lb->pty_wire->step_safety_epoch) {
            // R15-relative shift is required for safety chain
            void (*dispatch_fn)(void) = lb->pty_wire->step_safety_epoch;
            __asm__ __volatile__(
                "push %%r15\n\t"
                "mov %%rdi, %%r15\n\t"
                "sub $8, %%rsp\n\t"
                "call *%%rax\n\t"
                "add $8, %%rsp\n\t"
                "pop %%r15\n\t"
                :
                : "D"(lb->pty_wire), "a"(dispatch_fn)
                : "rcx", "rdx", "rsi", "r8", "r9", "r10", "r11", "memory"
            );
        }
        if (!lb->pty_wire->is_active) {
            tsfi_io_printf(stdout, "[BROADCAST] PTY Thread reached rest state.\n");
            lb->stream_active = false;
        }
    } else {
        static bool warned = false;
        if (!warned) { tsfi_io_printf(stdout, "[BROADCAST] WARNING: No PTY thread to dispatch!\n"); warned = true; }
    }

    // 2. Telemetry and Encoding (Now managed by internal Thunks)
    broadcaster_encode_frame(lb);
    lb->stream_epoch++;
}

static void broadcaster_logic_provenance(void *obj) { (void)obj; }

static void broadcaster_logic_directive(void *obj, char *cmd) {
    LauBroadcaster *lb = (LauBroadcaster *)obj;
    if (strcmp(cmd, "STOP") == 0) {
        lb->stream_active = false;
    } else if (strcmp(cmd, "RESONATE") == 0) {
        lb->stream_active = true;
    }
}

static LauBroadcaster *g_active_broadcaster = NULL;
LauBroadcaster* get_active_broadcaster(void) { return g_active_broadcaster; }

LauBroadcaster* lau_wire_broadcaster(const char *stream_key, void *v_sys, pid_t *out_pid) {
    LauBroadcaster *lb = (LauBroadcaster *)lau_malloc_wired(sizeof(LauBroadcaster));
    if (!lb) return NULL;

    memset(lb, 0, sizeof(LauBroadcaster));
    for (int y = 0; y < BROADCASTER_ROWS; y++) {
        for (int x = 0; x < BROADCASTER_COLS; x++) {
            lb->char_BASE[y][x] = ' ';
            lb->char_BASE_dirty[y][x] = true; // Initial render should be full
        }
    }
    lb->is_dirty = true;
    
    g_active_broadcaster = lb;
    strncpy(lb->stream_key, stream_key, sizeof(lb->stream_key) - 1);
    lb->broadcast_intensity = 1.0f;
    lb->stream_active = true;
    lb->v_sys = (VulkanSystem *)v_sys;
    tsfi_io_printf(stdout, "[DEBUG] lau_wire_broadcaster: lb->stream_active set to true, lb=%p\n", (void*)lb);

    // Launch Subshell via Resident Wire PTY
    lb->pty_wire = tsfi_wire_pty_fork("echo 'VTY_START'; exec /bin/bash");
    if (!lb->pty_wire) {
        tsfi_io_printf(stderr, "[BROADCAST] Failed to create PTY thread\n");
        lau_free(lb);
        return NULL;
    }
    lb->pty_master_fd = lb->pty_wire->master_fd;
    if (out_pid) *out_pid = lb->pty_wire->child_pid;

    // Wire PTY wavefronts if Vulkan is present
    if (lb->v_sys) {
        memcpy(&lb->pty_wire->log_stdin, &lb->v_sys->wavefront.stdin_log, sizeof(LauWireLog));
        memcpy(&lb->pty_wire->log_stdout, &lb->v_sys->wavefront.stdout_log, sizeof(LauWireLog));
    }

    if (lb->v_sys) {
        if (vulkan_video_encode_init(lb) != 0) {
            tsfi_io_printf(stdout, "[BROADCAST] WARNING: Hardware encoder failed. Engaging Wave512 Soft-Encoder.\n");
            lb->soft_encode_active = true;
        }
    }

    LauSystemHeader *h = (LauSystemHeader *)((char *)lb - offsetof(LauSystemHeader, payload));
    h->logic_epoch = (void (*)(int*))broadcaster_logic_epoch;
    h->logic_state = broadcaster_logic_state;
    h->logic_provenance = (void (*)(void*))broadcaster_logic_provenance;
    h->logic_directive = (void (*)(int*, char*))broadcaster_logic_directive;

    if (!h->proxy) h->proxy = ThunkProxy_create();
    if (h->proxy) {
        void *entry = ThunkProxy_emit_safety_chain(h->proxy, h->logic_state, (void(*)(void*))h->logic_epoch, lb);
        lb->step_safety_epoch = (void(*)(void))entry;
        lb->provenance = (void(*)(void))ThunkProxy_emit_baked(h->proxy, (void*)h->logic_provenance, 1, lb);
        lb->step_executor_directive = (void(*)(char*))ThunkProxy_emit_mixed((ThunkProxy*)h->proxy, (void*)h->logic_directive, lb);
        lau_mprotect(((ThunkProxy*)h->proxy)->thunk_pool, PROT_READ | PROT_EXEC);
    }

    return lb;
}

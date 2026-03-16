#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "tsfi_c_math.h"
#include "tsfi_broadcaster.h"
#include "vulkan_main.h"
#include "tsfi_raw.h"

#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include "tsfi_raw.h"

extern void* tsfi_rtmp_connect(const char *hostname, const char *stream_key);
extern int tsfi_rtmp_publish(void *ctx);
extern void tsfi_rtmp_close(void *ctx);
extern VulkanSystem* create_vulkan_headless_system(void);

#include <sys/wait.h>
#include <signal.h>
#include "tsfi_raw.h"

static pid_t gemini_pid = 0;

void handle_sigint(int sig) {
    (void)sig;
    if (gemini_pid > 0) kill(gemini_pid, SIGTERM);
    exit(0);
}

extern void vulkan_broadcaster_update_BASE(LauBroadcaster *lb, const char *buf, size_t n);

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    signal(SIGTERM, handle_sigint);

    bool test_mode = false;
    const char *key = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--test") == 0) test_mode = true;
        else if (!key) key = argv[i];
    }

    if (!test_mode && !key) {
        printf("Usage: %s <STREAM_KEY> [--test]\n", argv[0]);
        printf("       %s --test (Visual test without broadcast)\n", argv[0]);
        return 1;
    }

    if (!key) key = "TEST_KEY_DUMMY";

    setvbuf(stdout, NULL, _IONBF, 0);

    if (test_mode) {
        printf("=== TSFi Live Broadcaster: TEST MODE (Window + Proxy) ===\n");
    } else {
        printf("=== TSFi Live Broadcaster: HEADLESS MODE ===\n");
    }

    // 1. Initialize Vulkan Manifold
    VulkanSystem *vs = NULL;
    if (test_mode) {
        extern VulkanSystem* create_vulkan_system(void);
        vs = create_vulkan_system();
    } else {
        vs = create_vulkan_headless_system();
    }

    if (!vs) {
        printf("[FAIL] Failed to initialize manifold.\n");
        return 1;
    }

    // 2. Initialize Resident Broadcaster linked to Vulkan
    LauBroadcaster *lb = lau_wire_broadcaster(key, vs, &gemini_pid);
    if (!lb) {
        printf("[FAIL] Failed to allocate broadcaster.\n");
        return 1;
    }
    printf("[DEBUG] Broadcaster entry point: %p\n", (void*)lb->step_safety_epoch);

    // 3. Connect to RTMP (Only if not in test mode or if key is real)
    if (!test_mode || (argc > 1 && strcmp(argv[1], "--test") != 0)) {
        lb->rtmp_ctx = tsfi_rtmp_connect("a.rtmp.youtube.com", key);
        if (!lb->rtmp_ctx || tsfi_rtmp_publish(lb->rtmp_ctx) != 0) {
            printf("[FAIL] RTMP Connection/Publish failed.\n");
            if (gemini_pid > 0) kill(gemini_pid, SIGTERM);
            return 1;
        }
        printf("[PASS] Broadcast Active.\n");
    } else {
        printf("[INFO] Broadcast skipped in --test mode.\n");
    }

    printf("[PASS] Session Live. Gemini-CLI interactive in this terminal.\n");
    printf("[INFO] Press ESC or Ctrl+C to terminate.\n");
    fflush(stdout);

    int reconnect_attempts = 0;
    time_t last_reconnect_time = 0;

    struct timespec last_frame_time;
    clock_gettime(CLOCK_MONOTONIC, &last_frame_time);

    fd_set fds;
    int frame_count = 0;
    printf("\r\n[DEBUG] lb->stream_active=%d, vs->running=%d\r\n", lb->stream_active, vs ? vs->running : -1);
    fflush(stdout);

    while (lb->stream_active && (!vs || vs->running)) {
        static int debug_loop = 0;
        if (debug_loop++ % 100 == 0) { printf("\r\n[DEBUG] Live loop running %d\r\n", debug_loop); fflush(stdout); }
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        double delta = (current_time.tv_sec - last_frame_time.tv_sec) + 
                       (current_time.tv_nsec - last_frame_time.tv_nsec) / 1e9;
        if (delta > 0) lb->current_fps = 0.9f * lb->current_fps + 0.1f * (1.0f / (float)delta);
        last_frame_time = current_time;

        // 1. Reconnection Monitor
        if (!lb->rtmp_ctx && (!test_mode || (argc > 1 && strcmp(argv[1], "--test") != 0))) {
            time_t now = time(NULL);
            int backoff = (1 << reconnect_attempts);
            if (backoff > 32) backoff = 32;

            if (now - last_reconnect_time >= backoff) {
                printf("\r[INFO] Attempting RTMP Reconnect (%d)...          \n", reconnect_attempts + 1);
                lb->rtmp_ctx = tsfi_rtmp_connect("a.rtmp.youtube.com", key);
                if (lb->rtmp_ctx && tsfi_rtmp_publish(lb->rtmp_ctx) == 0) {
                    printf("[PASS] Reconnected.\n");
                    reconnect_attempts = 0;
                    lb->reconnect_count++;
                } else {
                    if (lb->rtmp_ctx) {
                        tsfi_rtmp_close(lb->rtmp_ctx);
                        lb->rtmp_ctx = NULL;
                    }
                    reconnect_attempts++;
                    last_reconnect_time = now;
                }
            }
        }

        tsfi_broadcaster_ui_draw(lb);

        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(lb->pty_master_fd, &fds);
        
        int max_fd = (STDIN_FILENO > lb->pty_master_fd) ? STDIN_FILENO : lb->pty_master_fd;
        int wl_fd = -1;

        if (vs && vs->display) {
            wl_fd = wl_display_get_fd(vs->display);
            FD_SET(wl_fd, &fds);
            if (wl_fd > max_fd) max_fd = wl_fd;
            
            while (wl_display_prepare_read(vs->display) != 0)
                wl_display_dispatch_pending(vs->display);
            wl_display_flush(vs->display);
        }

        struct timeval tv = { .tv_usec = 10000 }; // 10ms poll
        int sel = select(max_fd + 1, &fds, NULL, NULL, &tv);
        
        if (sel > 0 && FD_ISSET(lb->pty_master_fd, &fds)) {
            static bool pty_active = false;
            if (!pty_active) { printf("[DEBUG] PTY Activity Detected!\n"); pty_active = true; }
        }

        if (vs && vs->display) {
            if (sel > 0 && FD_ISSET(wl_fd, &fds)) {
                wl_display_read_events(vs->display);
            } else {
                wl_display_cancel_read(vs->display);
            }
            wl_display_dispatch_pending(vs->display);
        }

        if (sel > 0) {
            if (FD_ISSET(STDIN_FILENO, &fds)) {
                char c;
                if (read(STDIN_FILENO, &c, 1) > 0) {
                    if (c == 3) { // Ctrl+C
                        printf("[INFO] Ctrl+C pressed. Terminating...\n");
                        lb->stream_active = false;
                    } else if (c == 27) { // ESC
                        printf("[INFO] ESC pressed. Terminating...\n");
                        lb->stream_active = false;
                    } else if (c == 22) { // Ctrl+V
                        printf("[INFO] Ctrl+V: Triggering ANALYZE_VISUALS...\n");
                        master_logic_directive(vs->counter, "ANALYZE_VISUALS");
                    }
                    ssize_t nw = write(lb->pty_master_fd, &c, 1);
                    (void)nw;
                }
            }
        }
        
        // Execute Broadcaster Epoch (Rasterize + Encode + Push + Dispatch PTY Thread)
        lb->step_safety_epoch();
        
        // Execute Vulkan System Epoch (Process Wayland Events + Render)
        if (vs) {
            vs->step_safety_epoch();
            if (!vs->running) {
                printf("\r[INFO] Window closed by user. Terminating...\n");
                lb->stream_active = false;
            }
        }

        tsfi_raw_usleep(1000);

        // 4. Vision Verification (Real-time Character Production)
        if (frame_count++ % 10 == 0) {
            if (vs) master_logic_directive(vs->counter, "ANALYZE_VISUALS");
            
            int non_space = 0;
            for (int y = 0; y < BROADCASTER_ROWS; y++) {
                for (int x = 0; x < BROADCASTER_COLS; x++) {
                    uint32_t ch = lb->char_BASE[y][x];
                    if (ch > 32) non_space++;
                }
            }
            if (non_space > 0) {
                printf("\r[VISION] Verified: %d characters in BASE.          ", non_space);
            } else {
                printf("\r[VISION] Warning: BASE is EMPTY. Check PTY/Gemini status.    ");
            }
            fflush(stdout);
        }

        // Check if child died
        int status;
        if (waitpid(gemini_pid, &status, WNOHANG) > 0) {
            printf("\r[INFO] Gemini session terminated.\n");
            lb->stream_active = false;
        }
    }

    if (gemini_pid > 0) {
        printf("\r\n[DEBUG] Killing child pid %d\r\n", gemini_pid); fflush(stdout);
        kill(gemini_pid, SIGTERM);
        printf("[DEBUG] Waiting for child pid %d\r\n", gemini_pid); fflush(stdout);
        waitpid(gemini_pid, NULL, WNOHANG); // Use WNOHANG to avoid blocking indefinitely in tests
        printf("[DEBUG] Child terminated.\r\n"); fflush(stdout);
    }
    
    if (vs) destroy_vulkan_system(vs);
    
    printf("\n=== Live Test Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <poll.h>
#include <termios.h>
#include <sys/ioctl.h>
#include "tsfi_wire_firmware.h"
#include "tsfi_wire_pty.h"
#include "lau_memory.h"
#include "tsfi_raw.h"

// --- Helper implementation of `get_vulkan_system` and `create_vulkan_headless_system`
// as they are required by the firmware link but we won't use Vulkan directly here.
void draw_frame(void *s) { (void)s; }
void* create_vulkan_display_system() { return NULL; }
void keyboard_handle_key(void *s, void *k, uint32_t t, uint32_t x, uint32_t c, int p) { (void)s;(void)k;(void)t;(void)x;(void)c;(void)p; }
void draw_ui_elements(void *s) { (void)s; }
void recreate_swapchain(void *s) { (void)s; }
void draw_gemini_monitor(void *s) { (void)s; }
void* create_dma_buffer(void *s, int w, int h) { (void)s;(void)w;(void)h;return NULL; }
bool init_swapchain(void *s) { (void)s; return true; }
void init_staging_vk_buffer(void *s, size_t n) { (void)s;(void)n; }
void cleanup_vulkan(void *v) { (void)v; }
void* init_vulkan() { return NULL; }
void* create_staging_buffer(size_t w, size_t h) { (void)w;(void)h;return NULL; }
void staging_clear_avx512(void *b, uint32_t c) { (void)b;(void)c; }
void destroy_staging_buffer(void *b) { (void)b; }
void save_staging_buffer_to_ppm(void *b, const char *f) { (void)b;(void)f; }
void vulkan_logic_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }

struct { void (*global)(void*, void*, uint32_t, const char*, uint32_t); void (*global_remove)(void*, void*, uint32_t); } feedback_listener, seat_listener, xdg_wm_base_listener, xdg_surface_listener, xdg_toplevel_listener, data_device_listener;

static struct termios orig_termios;
static int pty_master_fd = -1; // For SIGWINCH updates

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
    printf("\r\n"); // Ensure clean line on exit
}

void sigwinch_handler(int sig) {
    (void)sig;
    if (pty_master_fd >= 0) {
        struct winsize ws;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1) {
            ioctl(pty_master_fd, TIOCSWINSZ, &ws);
        }
    }
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disable_raw_mode);
    struct termios raw = orig_termios;
    
    // POSIX manual cfmakeraw equivalent
    raw.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    raw.c_oflag &= ~OPOST;
    raw.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    raw.c_cflag &= ~(CSIZE | PARENB);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    
    // Register SIGWINCH
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigwinch_handler;
    sigaction(SIGWINCH, &sa, NULL);
}

int main(int argc, char **argv) {
    printf("=== Interactive PTY Shell (Firmware Driven) ===\r\n");
    printf("All keystrokes and output are routed through zero-copy firmware wavefronts.\r\n");
    printf("Press Ctrl+] (ASCII 29) or Ctrl+Q (ASCII 17) to forcefully disconnect from the wrapper.\r\n");
    printf("-----------------------------------------------\r\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    char cmd[65] = {0};
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            strncat(cmd, argv[i], sizeof(cmd) - strlen(cmd) - 1);
            if (i < argc - 1) strncat(cmd, " ", sizeof(cmd) - strlen(cmd) - 1);
        }
    } else {
        strcpy(cmd, "cat"); // Default to cat
    }

    // 1. Send Directive to Spawn Process
    memset(&fw->rtl.wrf[0], 0, 64);
    memcpy(&fw->rtl.wrf[0], cmd, strlen(cmd));
    
    fw->rtl.directive_cmd = 0x47454D49; // GEMINI (Spawn PTY)
    fw->rtl.directive_strobe = true;
    fw->rtl.epoch_strobe = true;
    fw->rtl.session_id = 1;
    LauWireFirmware_eval_sequential(&fw->rtl);
    fw->rtl.directive_strobe = false;
    fw->rtl.epoch_strobe = false;

    // Flush combinatorial logic so the peripheral strobes catch up
    LauWireFirmware_eval_combinatorial(&fw->rtl);
    fw->rtl.periph_directive_strobe = true;
    fw->rtl.periph_directive_cmd = 0x47454D49;
    
    // Step peripheral to actually fork the PTY
    tsfi_wire_firmware_step_peripheral(fw, NULL);

    if (!fw->rtl.periph_state_ready) {
        printf("[ERROR] Failed to establish PTY session.\r\n");
        return 1;
    }

    pty_master_fd = tsfi_wire_firmware_get_session_fd(1);
    enable_raw_mode();
    sigwinch_handler(0); // Trigger an initial resize immediately

    struct pollfd pfd[1];
    pfd[0].fd = STDIN_FILENO;
    pfd[0].events = POLLIN;

    uint32_t host_epoch = 1;
    char out_buf[4096];
    char in_buf[16];
    bool exit_requested = false;
    int drain_loops = 0;

    while (fw->rtl.periph_state_ready) {
        // Increment epoch
        tsfi_wire_firmware_strobe(fw, 0x0D, (uint64_t)host_epoch++);
        
        // Step Hardware
        fw->rtl.session_id = 1;
        tsfi_wire_firmware_step_peripheral(fw, NULL);

        // Print stdout from PTY (cat echoing our keystrokes)
        size_t n = fw->cell_extract_stdout(1, out_buf, sizeof(out_buf) - 1);
        if (n > 0) {
            out_buf[n] = '\0';
            printf("%s", out_buf);
            fflush(stdout);
        }

        if (exit_requested) {
            // Drain remaining echoes before exiting
            if (drain_loops++ > 20) break;
            tsfi_raw_usleep(5000);
            continue;
        }

        // Check our STDIN (Raw keystrokes)
        int ret = poll(pfd, 1, 10); // 10ms poll
        if (ret > 0 && (pfd[0].revents & POLLIN)) {
            ssize_t in_n = read(STDIN_FILENO, in_buf, sizeof(in_buf) - 1);
            if (in_n > 0) {
                // Pass keys directly to the firmware. 
                // Intercept Ctrl+] (ASCII 29) or Ctrl+Q (ASCII 17) as a hard-escape for the wrapper itself.
                for (ssize_t i = 0; i < in_n; i++) {
                    if (in_buf[i] == 29 || in_buf[i] == 17) { // Ctrl+] or Ctrl+Q
                        exit_requested = true;
                        in_n = i;
                        break;
                    }
                }
                if (in_n > 0) fw->cell_inject_stdin(1, in_buf, in_n);
            }
        }
    }

    disable_raw_mode();
    printf("\r\n[INFO] Closing Session...\r\n");
    fw->rtl.periph_directive_strobe = true;
    fw->rtl.periph_directive_cmd = 0x53544F50; // STOP
    fw->rtl.session_id = 1;
    tsfi_wire_firmware_step_peripheral(fw, NULL);

    return 0;
}
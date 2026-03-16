#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "tsfi_wire_firmware.h"
#include "tsfi_wire_pty.h"
#include "lau_memory.h"
#include "vulkan/vulkan_system.h"
#include "tsfi_wiring.h"
#include "tsfi_trace.h"
#include "tsfi_logic.h"

// --- Mocks ---
static char captured_pty_data[8192];
static size_t captured_len = 0;

ssize_t tsfi_wire_pty_write(LauWirePty *pty, const char *buf, size_t n) {
    (void)pty;
    if (captured_len + n < sizeof(captured_pty_data)) {
        memcpy(captured_pty_data + captured_len, buf, n);
        captured_len += n;
    }
    return (ssize_t)n;
}

// Minimal mocks for other symbols
void draw_frame(void *s) { (void)s; }
void* create_vulkan_display_system() { return NULL; }
void keyboard_handle_key(void *s, void *k, uint32_t t, uint32_t x, uint32_t c, int p) { (void)s;(void)k;(void)t;(void)x;(void)c;(void)p; }
void draw_ui_elements(void *s) { (void)s; }
void* get_active_broadcaster() { return NULL; }
void recreate_swapchain(void *s) { (void)s; }
void* create_dma_buffer(void *s, int w, int h) { (void)s;(void)w;(void)h;return NULL; }
bool init_swapchain(void *s) { (void)s; return true; }
void init_staging_vk_buffer(void *s, size_t n) { (void)s;(void)n; }
void cleanup_vulkan(void *v) { (void)v; }
void* init_vulkan() { return NULL; }
StagingBuffer* create_staging_buffer(size_t w, size_t h) { (void)w;(void)h;return NULL; }
void staging_clear_avx512(StagingBuffer *b, uint32_t c) { (void)b;(void)c; }
void destroy_staging_buffer(StagingBuffer *b) { (void)b; }
void save_staging_buffer_to_ppm(StagingBuffer *b, const char *f) { (void)b;(void)f; }
uint8_t tsfi_vision_max_value_raw(uint8_t *d, size_t s) { (void)d;(void)s;return 0; }
void tsfi_vision_analyze_glyph(void *d, int w, int h, void *p, void *r) { (void)d;(void)w;(void)h;(void)p;(void)r; }
typedef struct { int class_id; float confidence; } TSFiClassification;
TSFiClassification tsfi_vision_classify(void *r) { (void)r; TSFiClassification c = {0, 0.0f}; return c; }
void tsfi_wire_pty_step(LauWirePty *pty) { (void)pty; }
LauWirePty* tsfi_wire_pty_fork(void (*callback)(void*, const char*, size_t), void *arg, const char *cmd) {
    (void)callback;(void)arg;(void)cmd;
    LauWirePty *pty = (LauWirePty*)lau_malloc_wired(sizeof(LauWirePty));
    memset(pty, 0, sizeof(LauWirePty));
    pty->is_active = true;
    return pty;
}

struct { void (*global)(void*, void*, uint32_t, const char*, uint32_t); void (*global_remove)(void*, void*, uint32_t); } feedback_listener, seat_listener, xdg_wm_base_listener, xdg_surface_listener, xdg_toplevel_listener, data_device_listener;

void apply_traced_resonance(WaveSystem *ws, void (*thunk)(void*), void *arg, const char *fn_name) {
    (void)fn_name; if (thunk) thunk(ws); (void)arg;
}

int main() {
    printf("=== TSFi log_stdin Wrap-Around Verification ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    // Create a mock PTY state using Directive
    const char *cmd = "mock";
    memcpy(&fw->rtl.wrf[0], cmd, strlen(cmd));
    fw->step_executor_directive("GEMINI");
    tsfi_wire_firmware_step_peripheral(fw, NULL); // Now g_gemini_pty is set
    
    // Setup log_stdin_ptr
    char *stdin_buf = (char *)lau_malloc_wired(4096);
    fw->rtl.log_stdin_ptr = (uintptr_t)stdin_buf;
    
    // FORCE WRAP STATE
    // Tail at 4090, Head at 10.
    // Bytes [4090...4095] (6 bytes) and [0...9] (10 bytes) = 16 bytes total.
    fw->rtl.log_stdin_tail = 4090;
    fw->rtl.log_stdin_head = 10;
    
    memset(stdin_buf, 0, 4096);
    memcpy(stdin_buf + 4090, "WRAP", 4);
    memcpy(stdin_buf + 4094, "!!", 2);
    memcpy(stdin_buf, "START_OF_BUF", 10);
    
    // Trigger Peripheral Step
    fw->rtl.periph_state_ready = 1;
    fw->rtl.plugin_epoch = 0;
    fw->rtl.host_epoch = 1; // host > plugin triggers epoch_strobe
    
    printf("[INFO] Triggering step with wrapped log_stdin...\n");
    captured_len = 0;
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    
    printf("[DEBUG] Captured Length: %zu\n", captured_len);
    if (captured_len == 16) {
        printf("[PASS] Correct number of bytes captured.\n");
        printf("Captured Data: %.*s\n", (int)captured_len, captured_pty_data);
        if (memcmp(captured_pty_data, "WRAP!!START_OF_B", 16) == 0) {
            printf("[PASS] Data order and content verified.\n");
        } else {
            printf("[FAIL] Data content mismatch.\n");
            return 1;
        }
    } else {
        printf("[FAIL] Expected 16 bytes, got %zu. Logic error persists.\n", captured_len);
        return 1;
    }
    
    if (fw->rtl.log_stdin_tail == 10) {
        printf("[PASS] log_stdin_tail correctly synchronized to head.\n");
    } else {
        printf("[FAIL] log_stdin_tail sync failed. Tail: %u\n", fw->rtl.log_stdin_tail);
        return 1;
    }

    printf("=== Wrap-Around Test Passed ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

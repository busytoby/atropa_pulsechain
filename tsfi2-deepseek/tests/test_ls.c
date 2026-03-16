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
#include "tsfi_raw.h"

// --- Mocks for Linker ---
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

struct { void (*global)(void*, void*, uint32_t, const char*, uint32_t); void (*global_remove)(void*, void*, uint32_t); } feedback_listener, seat_listener, xdg_wm_base_listener, xdg_surface_listener, xdg_toplevel_listener, data_device_listener;

int main() {
    printf("=== TSFi ls -l Zero-Copy Unit Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    // Create a mock VulkanSystem to provide the log buffers
    VulkanSystem *sys = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    memset(sys, 0, sizeof(VulkanSystem));
    sys->system_id = 0; // Target for wiring

    // Setup zero-copy stdout log
    char *stdout_buf = (char *)lau_malloc_wired(4096);
    memset(stdout_buf, 0, 4096);
    sys->log_stdout.buffer = stdout_buf;
    sys->log_stdout.head = &fw->rtl.log_stdout_head;
    sys->log_stdout.tail = &fw->rtl.log_stdout_tail;
    sys->log_stdout.valid_head = &fw->rtl.log_stdout_valid_head;
    sys->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    sys->log_stdout.capacity = 4096;

    // Setup mock stdin log
    sys->log_stdin.buffer = (char *)lau_malloc_wired(4096);
    sys->log_stdin.head = &fw->rtl.log_stdin_head;
    sys->log_stdin.tail = &fw->rtl.log_stdin_tail;
    sys->log_stdin.lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;
    sys->log_stdin.capacity = 4096;

    // Wire the system
    tsfi_wire_firmware_load_struct(fw, sys);

    // 1. Load "ls -l" into wrf[0]
    const char *cmd = "ls -l";
    memcpy(&fw->rtl.wrf[0], cmd, strlen(cmd));

    // 2. Issue GEMINI directive
    fw->rtl.directive_cmd = 0x47454D49; // GEMINI
    fw->rtl.directive_strobe = true;
    fw->rtl.epoch_strobe = true;
    LauWireFirmware_eval_sequential(&fw->rtl);
    fw->rtl.directive_strobe = false;
    fw->rtl.epoch_strobe = false;
    LauWireFirmware_eval_combinatorial(&fw->rtl);

    // 3. Step peripheral to fork PTY
    printf("[DEBUG] Periph Directive Strobe: %d, Cmd: 0x%08X\n", fw->rtl.periph_directive_strobe, fw->rtl.periph_directive_cmd);
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    printf("[DEBUG] Periph State Ready: %d\n", fw->rtl.periph_state_ready);

    // 4. Poll until PTY finishes
    int loops = 0;
    while (loops++ < 500) {
        // Increment host_epoch to trigger strobe
        fw->rtl.prov_data = loops;
        fw->rtl.prov_addr = 0x0D; // host_epoch
        fw->rtl.prov_strobe = true;
        fw->rtl.epoch_strobe = true;
        LauWireFirmware_eval_sequential(&fw->rtl);
        fw->rtl.prov_strobe = false;
        fw->rtl.epoch_strobe = false;
        
        tsfi_wire_firmware_step_peripheral(fw, NULL);
        if (fw->rtl.log_stdout_head != fw->rtl.log_stdout_tail) break;
        tsfi_raw_usleep(1000);
    }

    // 5. Verify stdout log has data
    uint32_t head = fw->rtl.log_stdout_head;
    uint32_t tail = fw->rtl.log_stdout_tail;
    printf("[DEBUG] log_stdout_ptr: %p\n", (void*)(uintptr_t)fw->rtl.log_stdout_ptr);
    printf("[DEBUG] head: %u, tail: %u\n", head, tail);
    
    if (head != tail) {
        printf("[PASS] ls -l output captured in zero-copy log_stdout.\n");
        printf("Output Sample: %.*s\n", 64, stdout_buf);
    } else {
        printf("[FAIL] No output captured from ls -l.\n");
        return 1;
    }

    printf("=== ls -l Test Passed ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

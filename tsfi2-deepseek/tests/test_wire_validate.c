#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include "tsfi_wire_firmware.h"
#include "lau_memory.h"
#include "vulkan/vulkan_system.h"
#include "tsfi_wiring.h"

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
    printf("=== TSFi Zero-Copy Wire Validation Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    VulkanSystem *sys = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    memset(sys, 0, sizeof(VulkanSystem));
    sys->system_id = 0;

    char *stdout_buf = (char *)lau_malloc_wired(4096);
    memset(stdout_buf, 0, 4096);
    sys->log_stdout.buffer = stdout_buf;
    sys->log_stdout.head = &fw->rtl.log_stdout_head;
    sys->log_stdout.tail = &fw->rtl.log_stdout_tail;
    sys->log_stdout.valid_head = &fw->rtl.log_stdout_valid_head;
    sys->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    sys->log_stdout.capacity = 4096;

    tsfi_wire_firmware_load_struct(fw, sys);

    // 1. Inject "SECRET_DATA" into the buffer
    const char *sensitive = "TOP_SECRET_INFO";
    memcpy(stdout_buf, sensitive, strlen(sensitive));
    fw->rtl.log_stdout_head = strlen(sensitive);
    fw->rtl.log_stdout_valid_head = 0;

    printf("[INFO] Buffer before validation: %s\n", stdout_buf);

    // 2. Trigger Validation Strobe
    tsfi_wire_firmware_validate(fw);

    // 3. Verify masking
    printf("[INFO] Buffer after validation: %s\n", stdout_buf);
    
    if (strstr(stdout_buf, "SECRET") == NULL && strstr(stdout_buf, "******") != NULL) {
        printf("[PASS] Sensitive data masked correctly in zero-copy buffer.\n");
    } else {
        printf("[FAIL] Masking failed or didn't trigger. Buffer: %s\n", stdout_buf);
        return 1;
    }

    printf("=== Wire Validation Test Passed ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

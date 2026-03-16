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

// Trace Implementation
void apply_traced_resonance(WaveSystem *ws, void (*thunk)(void*), void *arg, const char *fn_name) {
    if (!thunk) return;
    (void)fn_name;
    // Just call directly for the mock test
    if (strcmp(fn_name, "step_executor_directive") == 0) {
        ((void(*)(int*, char*))thunk)((int*)ws->counter, (char*)arg);
    } else {
        thunk(ws);
    }
}

int main() {
    printf("=== TSFi Gemini CLI Zero-Copy Interactive Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    // Create a mock VulkanSystem
    VulkanSystem *sys = (VulkanSystem *)lau_malloc_wired(sizeof(VulkanSystem));
    memset(sys, 0, sizeof(VulkanSystem));
    LauWiredHeader *sys_h = (LauWiredHeader *)((char *)sys - offsetof(LauWiredHeader, payload));
    sys->system_id = 0;
    
    // Setup counter pointers in the header/payload overlap correctly
    static int mock_counter = 0;
    static int mock_version = 1;
    static char *mock_resonance = "MOCK_START";
    static bool mock_ftw = false;
    static bool mock_autonomous = false;

    *(int**)&sys->counter = &mock_counter;
    *(int**)&sys->version = &mock_version;
    *(char***)&sys->resonance_as_status = &mock_resonance;
    *(bool**)&sys->ftw = &mock_ftw;
    *(bool**)&sys->is_autonomous_excuse_active = &mock_autonomous;

    // Setup zero-copy logs
    sys->log_stdout.buffer = (char *)lau_malloc_wired(4096);
    memset(sys->log_stdout.buffer, 0, 4096);
    sys->log_stdout.head = &fw->rtl.log_stdout_head;
    sys->log_stdout.tail = &fw->rtl.log_stdout_tail;
    sys->log_stdout.valid_head = &fw->rtl.log_stdout_valid_head;
    sys->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    sys->log_stdout.capacity = 4096;

    sys->log_stdin.buffer = (char *)lau_malloc_wired(4096);
    memset(sys->log_stdin.buffer, 0, 4096);
    sys->log_stdin.head = &fw->rtl.log_stdin_head;
    sys->log_stdin.tail = &fw->rtl.log_stdin_tail;
    sys->log_stdin.lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;
    sys->log_stdin.capacity = 4096;

    // Use default logic for mock
    lau_wire_system((WaveSystem*)sys, (LauSystemHeader*)sys_h, tsfi_get_default_logic());

    // 1. Set command to mock gemini-cli
    const char *cmd = "sh";
    memcpy(&fw->rtl.wrf[0], cmd, strlen(cmd));

    // 2. Launch Gemini PTY via Firmware Directive
    if (fw->step_executor_directive) {
        printf("[DEBUG] Issuing GEMINI directive to firmware.\n");
        fw->step_executor_directive("GEMINI");
    }

    // Step peripheral to perform the fork
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    if (!fw->rtl.periph_state_ready) {
        printf("[FAIL] Gemini PTY failed to launch. strobe=%d, cmd=0x%08X\n", 
               fw->rtl.periph_directive_strobe, fw->rtl.periph_directive_cmd);
        return 1;
    }
    printf("[PASS] Gemini PTY Launched.\n");

    // 3. Interactive Input via log_stdin
    const char *input = "echo 'HELMHOLTZ_SYNC_SUCCESS'\n";
    if (fw->cell_inject_stdin) {
        fw->cell_inject_stdin(0, input, strlen(input));
    }
    
    printf("[INFO] Sent input to Gemini: %s", input);

    // 4. Synchronize via firmware using HELMHOLTZ list
    int loops = 0;
    while (loops < 1000) {
        // Increment host_epoch to allow periph_epoch_strobe
        fw->rtl.host_epoch = loops + 1;
        
        // Dispatch via HELMHOLTZ (simulates host-plugin sync)
        HELMHOLTZ_RESONANCE_LIST(STEP, (WaveSystem*)sys, 1.0);
        
        // Step peripheral (Simulates the firmware background loop)
        tsfi_wire_firmware_step_peripheral(fw, NULL);
        
        if (strstr(sys->log_stdout.buffer, "HELMHOLTZ_SYNC_SUCCESS") != NULL) break;
        
        tsfi_raw_usleep(1000);
        loops++;
    }

    if (strstr(sys->log_stdout.buffer, "HELMHOLTZ_SYNC_SUCCESS") != NULL) {
        printf("[PASS] Captured interactive response from Gemini via zero-copy stdout.\n");
        printf("Output Sample: %s", sys->log_stdout.buffer);
    } else {
        printf("[FAIL] Failed to capture interactive response. Log Stdout (Head=%u, Tail=%u): %s\n", 
               fw->rtl.log_stdout_head, fw->rtl.log_stdout_tail, sys->log_stdout.buffer);
        return 1;
    }

    // Cleanup
    if (fw->step_executor_directive) fw->step_executor_directive("STOP_GEMINI");
    tsfi_wire_firmware_step_peripheral(fw, NULL);

    printf("=== Gemini CLI Interactive Test Passed ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

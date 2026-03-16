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

// --- Mocks ---
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

void apply_traced_resonance(WaveSystem *ws, void (*thunk)(void*), void *arg, const char *fn_name) {
    if (!thunk) return;
    if (strcmp(fn_name, "step_executor_directive") == 0) {
        ((void(*)(int*, char*))thunk)((int*)ws->counter, (char*)arg);
    } else {
        thunk(ws);
    }
}

static VulkanSystem *g_test_sys = NULL;

static void flexible_pty_callback(void *arg, const char *buf, size_t n) {
    (void)arg;
    if (!g_test_sys) return;
    char *out_buf = g_test_sys->log_stdout.buffer;
    uint32_t *head = g_test_sys->log_stdout.head;
    uint32_t *tail = g_test_sys->log_stdout.tail;
    
    for (size_t i = 0; i < n; i++) {
        uint32_t next = (*head + 1) % 4096;
        if (next != *tail) {
            out_buf[*head] = buf[i];
            *head = next;
        }
    }
}

static void run_test(const char *test_name, const char *cmd, const char *input, const char *expected_pattern) {
    printf("--- Running Test: %s (%s) ---\n", test_name, cmd);
    
    LauWireFirmware *fw = tsfi_wire_firmware_get();
    
    // Clear logs
    memset(g_test_sys->log_stdout.buffer, 0, 4096);
    memset(g_test_sys->log_stdin.buffer, 0, 4096);
    *g_test_sys->log_stdout.head = 0;
    *g_test_sys->log_stdout.tail = 0;
    *g_test_sys->log_stdin.head = 0;
    *g_test_sys->log_stdin.tail = 0;

    // 1. Set command in wave reg
    memset(&fw->rtl.wrf[0], 0, 64);
    memcpy(&fw->rtl.wrf[0], cmd, strlen(cmd));

    // 2. Launch GEMINI
    // Note: We'll use a hack to set the callback for this specific test
    // tsfi_wire_firmware.c uses rx_pin_pty_out_impl which uses g_firmware->rtl.log_stdout_ptr
    // So our existing infrastructure should work if we initialize correctly.
    
    fw->step_executor_directive("GEMINI");
    tsfi_wire_firmware_step_peripheral(fw, NULL);
    assert(fw->rtl.periph_state_ready == 1);

    // 3. Send Input
    if (input) {
        _Atomic uint32_t *in_lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;
        while (atomic_exchange_explicit(in_lock, 1, memory_order_acquire)) { __builtin_ia32_pause(); }
        strcpy(g_test_sys->log_stdin.buffer, input);
        fw->rtl.log_stdin_head = strlen(input);
        atomic_store_explicit(in_lock, 0, memory_order_release);
    }

    // 4. Poll
    int loops = 0;
    while (loops < 3000) {
        fw->rtl.host_epoch = loops + 1;
        HELMHOLTZ_RESONANCE_LIST(STEP, (WaveSystem*)g_test_sys, 1.0);
        tsfi_wire_firmware_step_peripheral(fw, NULL);
        if (strstr(g_test_sys->log_stdout.buffer, expected_pattern) != NULL) break;
        tsfi_raw_usleep(1000);
        loops++;
    }

    if (strstr(g_test_sys->log_stdout.buffer, expected_pattern) != NULL) {
        printf("[PASS] %s: Found pattern '%s'\n", test_name, expected_pattern);
    } else {
        printf("[FAIL] %s: Pattern '%s' not found. Log: %s\n", test_name, expected_pattern, g_test_sys->log_stdout.buffer);
        exit(1);
    }

    // Cleanup
    fw->step_executor_directive("STOP_GEMINI");
    tsfi_wire_firmware_step_peripheral(fw, NULL);
}

int main() {
    printf("=== TSFi Firmware Flexible PTY Multi-Test ===\n");

    tsfi_wire_firmware_init();
    LauWireFirmware *fw = tsfi_wire_firmware_get();

    LauSystemHeader *sys_h = (LauSystemHeader *)lau_malloc_wired(sizeof(VulkanSystem));
    memset(sys_h, 0, sizeof(LauSystemHeader) + sizeof(VulkanSystem));
    g_test_sys = (VulkanSystem *)sys_h->payload;
    g_test_sys->system_id = 0;
    
    // Setup mock components
    static StagingBuffer mock_sb = {0};
    static LauTelemetryState mock_telem = {0};
    g_test_sys->paint_buffer = &mock_sb;
    g_test_sys->telem = &mock_telem;

    static int mock_counter = 0;
    static int mock_version = 1;
    static char *mock_resonance = "MOCK";
    static bool mock_ftw = false;
    static bool mock_auto = false;
    *(int**)&g_test_sys->counter = &mock_counter;
    *(int**)&g_test_sys->version = &mock_version;
    *(char***)&g_test_sys->resonance_as_status = &mock_resonance;
    *(bool**)&g_test_sys->ftw = &mock_ftw;
    *(bool**)&g_test_sys->is_autonomous_excuse_active = &mock_auto;

    g_test_sys->log_stdout.buffer = (char *)lau_malloc_wired(4096);
    g_test_sys->log_stdin.buffer = (char *)lau_malloc_wired(4096);
    
    g_test_sys->log_stdout.head = &fw->rtl.log_stdout_head;
    g_test_sys->log_stdout.tail = &fw->rtl.log_stdout_tail;
    g_test_sys->log_stdout.valid_head = &fw->rtl.log_stdout_valid_head;
    g_test_sys->log_stdout.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
    g_test_sys->log_stdout.capacity = 4096;

    g_test_sys->log_stdin.head = &fw->rtl.log_stdin_head;
    g_test_sys->log_stdin.tail = &fw->rtl.log_stdin_tail;
    g_test_sys->log_stdin.lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;
    g_test_sys->log_stdin.capacity = 4096;

    lau_wire_system((WaveSystem*)g_test_sys, sys_h, tsfi_get_default_logic());

    run_test("Python3", "python3", "print('PY_RES_' + str(40+2))\n", "PY_RES_42");
    run_test("Node.js", "node", "console.log('NODE_RES_' + (20+22))\n", "NODE_RES_42");
    run_test("Bash", "sh", "expr 21 + 21 | xargs -I{} echo BASH_RES_{}\n", "BASH_RES_42");

    printf("=== All Flexible PTY Tests Passed ===\n");
        extern void tsfi_wire_firmware_teardown(void);
    tsfi_wire_firmware_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

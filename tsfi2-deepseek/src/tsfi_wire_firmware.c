#define _GNU_SOURCE
#include "tsfi_wire_firmware.h"
#include "tsfi_reaction.h"
#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_wire_pty.h"
#include "tsfi_raw.h"
#include "tsfi_io.h"
#include "lau_audit.h"
#include "vulkan/vulkan_system.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <stdatomic.h>
#include <sys/wait.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <poll.h>

// Forward declarations for external checksums
extern uint32_t calculate_checksum_wired_external(const void *h);
extern uint32_t calculate_checksum_basic_external(const void *h);

LauWireFirmware *g_tsfi_firmware = NULL;
static LauWirePty *g_sessions[64] = {0};
static FILE *g_fw_stdin_log = NULL;
static FILE *g_fw_stdout_log = NULL;
// g_init_in_progress linked from registry

// Forward declaration for classification thunk (Weakly linked)
extern int tsfi_vision_classify_thunk(void *obj) __attribute__((weak));

extern void vulkan_broadcaster_update_BASE(void *lb, const char *buf, size_t n);
__attribute__((weak)) void vulkan_broadcaster_update_BASE(void *lb, const char *buf, size_t n) { (void)lb; tsfi_io_printf(stdout, "%.*s", (int)n, buf); tsfi_io_flush(stdout); }

static ssize_t tx_pin_pty_in_impl(void *context, const char *data, size_t len) {
    (void)context; if (!g_tsfi_firmware) return -1;
    LauWireLog *wl = &g_tsfi_firmware->wavefront.stdin_log;
    char *buf = (char *)(uintptr_t)g_tsfi_firmware->rtl.log_stdin_ptr;
    if (!buf || !wl->head || !wl->tail) return -1;
    
    _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)wl->head;
    _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)wl->tail;

    uint32_t head = atomic_load_explicit(head_ptr, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_relaxed);
    
    size_t written = 0;
    while (written < len) {
        uint32_t next_head = (head + 1) % wl->capacity;
        if (next_head == tail) break;
        buf[head] = data[written++];
        head = next_head;
    }
    atomic_store_explicit(head_ptr, head, memory_order_release);
    return (ssize_t)written;
}

static void rx_pin_pty_out_impl(void *context, const char *data, size_t len) {
    (void)context; (void)data; (void)len;
    // Standard Rx implementation usually handled by direct buffer writes or PTY logic.
}

// Stub implementations for required firmware cells
static ssize_t cell_inject_stdin_impl(int session_id, const char *data, size_t len) { (void)session_id; return tx_pin_pty_in_impl(NULL, data, len); }
static size_t cell_extract_stdout_impl(int session_id, char *buf, size_t max_len) { 
    (void)session_id; if (!g_tsfi_firmware) return 0;
    LauWireLog *wl = &g_tsfi_firmware->wavefront.stdout_log;
    char *fbuf = (char *)(uintptr_t)g_tsfi_firmware->rtl.log_stdout_ptr;
    if (!fbuf || !wl->head || !wl->tail) return 0;

    _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)wl->head;
    _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)wl->tail;

    uint32_t head = atomic_load_explicit(head_ptr, memory_order_acquire);
    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_relaxed);

    size_t read_bytes = 0;
    while (read_bytes < max_len && tail != head) {
        buf[read_bytes++] = fbuf[tail];
        tail = (tail + 1) % wl->capacity;
    }
    atomic_store_explicit(tail_ptr, tail, memory_order_relaxed);
    return read_bytes;
}

static void cell_brush_teeth_impl(int timeout_sec) { (void)timeout_sec; /* Emulated physical maintenance */ }
static void cell_mcp_execute_impl(LauMasterWavefront *mw) { (void)mw; }
static void cell_wave_exec_impl(uint32_t op, uint8_t dest, uint8_t src1, uint8_t src2) { (void)op; (void)dest; (void)src1; (void)src2; }
static void cell_neural_observe_impl(int session_id, char *out_intent, char *out_insight) { (void)session_id; (void)out_intent; (void)out_insight; }
static int cell_printf_impl(int level, const char *fmt, ...) { 
    (void)level; va_list ap; va_start(ap, fmt); 
    if (g_fw_stdout_log) vfprintf(g_fw_stdout_log, fmt, ap);
    int ret = vprintf(fmt, ap); 
    va_end(ap);
    return ret;
}
static void cell_zhong_sync_wait_impl(uint64_t handle, uint64_t val) { (void)handle; (void)val; }
static void cell_zhong_sync_signal_impl(uint64_t handle, uint64_t val) { (void)handle; (void)val; }
static void cell_zhong_atomic_wait_impl(uint64_t addr, uint64_t val) { (void)addr; (void)val; }
static void cell_seal_capture_impl(void *ptr) { (void)ptr; }
static void cell_seal_release_impl(void *ptr) { (void)ptr; }
static void cell_reg_insert_impl(LauMetadata *m) { lau_registry_insert(m); }
static void cell_reg_remove_impl(void *ptr) { (void)ptr; }
static LauMetadata* cell_reg_lookup_impl(void *ptr) { return lau_registry_find(ptr); }
static void cell_reg_strobe_zmm_impl(int zmm_idx, void *ptr) { (void)zmm_idx; (void)ptr; }
static void* cell_mem_genesis_impl(size_t sz, LauHeaderType type, const char *file, int line) {
    if (type == LAU_TYPE_WIRED) return lau_memalign_wired_loc(512, sz, file, line);
    return lau_malloc_loc(sz, file, line);
}
static void cell_mem_reclaim_impl(void *ptr, size_t size) { (void)size; lau_free(ptr); }

static int cell_hardware_poll_impl(int timeout_ms, char *buf, size_t max) {
    struct pollfd pfd = { STDIN_FILENO, POLLIN, 0 };
    int ret = poll(&pfd, 1, timeout_ms);
    if (ret > 0 && (pfd.revents & POLLIN)) {
        if (fgets(buf, (int)max, stdin)) return 0;
    }
    return -1;
}

static void fw_logic_directive(int *cnt, char *dir) { (void)cnt; (void)dir; }
static void default_firmware_epoch(int *ver) { if (ver) (*ver)++; }
static bool default_firmware_state(void *obj) { (void)obj; return true; }

void tsfi_wire_firmware_teardown(void) {
    if (!g_tsfi_firmware) return;
    if (g_fw_stdin_log) fclose(g_fw_stdin_log);
    if (g_fw_stdout_log) fclose(g_fw_stdout_log);
    
    // Explicit cleanup of recursive buffers
    if (g_tsfi_firmware->wavefront.stdin_log.buffer) lau_free(g_tsfi_firmware->wavefront.stdin_log.buffer);
    if (g_tsfi_firmware->wavefront.stdout_log.buffer) lau_free(g_tsfi_firmware->wavefront.stdout_log.buffer);
    
    if (g_tsfi_firmware->core_yi.Psi) freeSHIO(g_tsfi_firmware->core_yi.Psi);
    if (g_tsfi_firmware->core_yi.Xi) tsfi_bn_free(g_tsfi_firmware->core_yi.Xi);
    if (g_tsfi_firmware->core_yi.Ring) tsfi_bn_free(g_tsfi_firmware->core_yi.Ring);
    
    // Drain static memory pools to resolve final active leaks
    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    
    lau_unseal_object(g_tsfi_firmware);
    shm_unlink("/tsfi_firmware_manifold");
    lau_free(g_tsfi_firmware);
    g_tsfi_firmware = NULL;
}

void tsfi_wire_firmware_init(void) {
    if (g_tsfi_firmware) {
        memset(&g_tsfi_firmware->manifold, 0, sizeof(LauRegistryManifold));
        return;
    }
    if (atomic_load_explicit(&g_init_in_progress, memory_order_relaxed)) return;
    atomic_store_explicit(&g_init_in_progress, 1, memory_order_relaxed);
    
    // Allocate the main firmware struct in Shared Memory (SHM)
    size_t fw_size = sizeof(LauWireFirmware);
    if (fw_size < 65536) fw_size = 65536; 
    
    const char *shm_name = "/tsfi_firmware_manifold";
    int shm_fd = shm_open(shm_name, O_RDWR | O_CREAT, 0666);
    if (shm_fd < 0) {
        perror("shm_open firmware");
        atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
        return;
    }
    
    // Check if we are the creator
    struct stat sb;
    fstat(shm_fd, &sb);
    bool is_creator = (sb.st_size == 0);
    
    if (is_creator) {
        if (ftruncate(shm_fd, fw_size) == -1) {
            perror("ftruncate firmware");
            close(shm_fd);
            atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
            return;
        }
    }

    LauWireFirmware *fw = (LauWireFirmware*)mmap(NULL, fw_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (fw == MAP_FAILED) {
        perror("mmap firmware shm");
        close(shm_fd);
        atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
        return;
    }
    
    if (is_creator) {
        memset(fw, 0, fw_size);
        // Bind Standard Cells only on creation
        fw->rx_pin_pty_out = rx_pin_pty_out_impl; 
        fw->tx_pin_pty_in = tx_pin_pty_in_impl;
        
        // Initial setup for the creator process
        fw->cell_inject_stdin = cell_inject_stdin_impl; 
        fw->cell_extract_stdout = cell_extract_stdout_impl;
        fw->cell_brush_teeth = cell_brush_teeth_impl;
        fw->cell_mcp_execute = cell_mcp_execute_impl; 
        fw->cell_wave_exec = cell_wave_exec_impl;
        fw->cell_neural_observe = cell_neural_observe_impl; 
        fw->cell_printf = cell_printf_impl;
        fw->cell_zhong_sync_wait = cell_zhong_sync_wait_impl; 
        fw->cell_zhong_sync_signal = cell_zhong_sync_signal_impl;
        fw->cell_zhong_atomic_wait = cell_zhong_atomic_wait_impl; 
        fw->cell_seal_capture = cell_seal_capture_impl;
        fw->cell_seal_release = cell_seal_release_impl; 
        fw->cell_reg_insert = cell_reg_insert_impl;
        fw->cell_reg_remove = cell_reg_remove_impl; 
        fw->cell_reg_lookup = cell_reg_lookup_impl;
        fw->cell_reg_strobe_zmm = cell_reg_strobe_zmm_impl; 
        fw->cell_mem_genesis = cell_mem_genesis_impl;
        fw->cell_mem_reclaim = cell_mem_reclaim_impl;
        fw->cell_hardware_poll = cell_hardware_poll_impl;

        memset(&fw->manifold, 0, sizeof(LauRegistryManifold));
        // Backfill manifold using local pointer
        // tsfi_registry_backfill_manifold(); 
        
        if (!g_fw_stdin_log) g_fw_stdin_log = fopen("logs/firmware_stdin.log", "ab");
        if (!g_fw_stdout_log) g_fw_stdout_log = fopen("logs/firmware_stdout.log", "ab");
        
        fw->rtl.reset_strobe = true; LauWireFirmware_eval_sequential(&fw->rtl);
        fw->rtl.reset_strobe = false;
        
        LauTelemetryState* boot_telem = lau_telemetry_get_state();
        if (boot_telem) {
            snprintf((char*)boot_telem->recip_dai, 128, "0x3632C8EB5AF3B");
            boot_telem->recip_symmetry = 1.0f;
        }
        
        // Use fixed header offset for SHM mapping
        LauSystemHeader *h = (LauSystemHeader *)((char *)fw + fw_size - 7168);
        h->logic_directive = (void (*)(int*, char*))fw_logic_directive;
        h->logic_epoch = (void (*)(int*))default_firmware_epoch;
        h->logic_state = (bool (*)(void*))default_firmware_state;
        lau_vram_init(&fw->vram);
        
        void* stdin_buf = lau_malloc_wired(4096);
        void* stdout_buf = lau_malloc_wired(4096);
        
        fw->wavefront.stdin_log.buffer = (char*)stdin_buf;
        fw->wavefront.stdin_log.head = &fw->rtl.log_stdin_head;
        fw->wavefront.stdin_log.tail = &fw->rtl.log_stdin_tail;
        fw->wavefront.stdin_log.lock = (_Atomic uint32_t *)&fw->rtl.log_stdin_lock;
        fw->wavefront.stdin_log.capacity = 4096;
        fw->rtl.log_stdin_ptr = (uintptr_t)stdin_buf;
        fw->wavefront.stdout_log.buffer = (char*)stdout_buf;
        fw->wavefront.stdout_log.head = &fw->rtl.log_stdout_head;
        fw->wavefront.stdout_log.tail = &fw->rtl.log_stdout_tail;
        fw->wavefront.stdout_log.valid_head = &fw->rtl.log_stdout_valid_head;
        fw->wavefront.stdout_log.lock = (_Atomic uint32_t *)&fw->rtl.log_stdout_lock;
        fw->wavefront.stdout_log.capacity = 4096;
        fw->rtl.log_stdout_ptr = (uintptr_t)stdout_buf;
        
        tsfi_reaction_boot_firmware_yi(&fw->core_yi);
    }

    g_tsfi_firmware = fw;
    atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
}

LauWireFirmware* tsfi_wire_firmware_get(void) { if (!g_tsfi_firmware) tsfi_wire_firmware_init(); return g_tsfi_firmware; }
LauWireFirmware* tsfi_wire_firmware_get_no_init(void) { return g_tsfi_firmware; }
int tsfi_wire_firmware_get_session_fd(int session_id) { if (session_id >= 0 && session_id < 64 && g_sessions[session_id]) return g_sessions[session_id]->master_fd; return -1; }

void tsfi_wire_firmware_setup_signals(void) { /* Signal handling setup */ }
void tsfi_wire_firmware_load_struct(LauWireFirmware *fw, void *ptr) { (void)fw; (void)ptr; }
void tsfi_wire_firmware_load_waveform(LauWireFirmware *fw, int reg_idx, void *wave_ptr) { (void)fw; (void)reg_idx; (void)wave_ptr; }
void tsfi_wire_firmware_step_peripheral(LauWireFirmware *fw, void *pty) { (void)fw; (void)pty; }

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
    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_acquire);
    
    size_t count = 0;
    for (size_t i = 0; i < len; i++) {
        uint32_t next_head = (head + 1) % 4096;
        if (next_head != tail) { buf[head] = data[i]; head = next_head; count++; }
        else break;
    }
    atomic_store_explicit(head_ptr, head, memory_order_release);

    if (g_fw_stdin_log) {
        tsfi_io_write(g_fw_stdin_log, data, count);
        tsfi_io_flush(g_fw_stdin_log);
    }

    return count;
}

static void rx_pin_pty_out_impl(void *context, const char *data, size_t len) {
    (void)context; if (!g_tsfi_firmware) return;
    lau_vram_write_string(&g_tsfi_firmware->vram, data, len);

    LauWireLog *wl = &g_tsfi_firmware->wavefront.stdout_log;
    char *buf = (char *)(uintptr_t)g_tsfi_firmware->rtl.log_stdout_ptr;
    if (!buf || !wl->head || !wl->tail) return;
    
    _Atomic uint32_t *head_ptr = (_Atomic uint32_t *)wl->head;
    _Atomic uint32_t *tail_ptr = (_Atomic uint32_t *)wl->tail;

    uint32_t head = atomic_load_explicit(head_ptr, memory_order_relaxed);
    uint32_t tail = atomic_load_explicit(tail_ptr, memory_order_acquire);
    
    for (size_t i = 0; i < len; i++) {
        uint32_t next_head = (head + 1) % 4096;
        if (next_head != tail) { buf[head] = data[i]; head = next_head; }
        else break;
    }
    atomic_store_explicit(head_ptr, head, memory_order_release);

    if (g_fw_stdout_log) {
        tsfi_io_write(g_fw_stdout_log, data, len);
        tsfi_io_flush(g_fw_stdout_log);
    }
}

static void strobe_prov(LauWireFirmware *fw, uint8_t addr, uint64_t data) {
    fw->rtl.prov_data = data; fw->rtl.prov_addr = addr; fw->rtl.prov_strobe = true; fw->rtl.epoch_strobe = true;
    LauWireFirmware_eval_sequential(&fw->rtl);
    fw->rtl.epoch_strobe = false; fw->rtl.prov_strobe = false; LauWireFirmware_eval_combinatorial(&fw->rtl);
}

void tsfi_wire_firmware_strobe(LauWireFirmware *fw, uint8_t addr, uint64_t data) { if (!fw) return; strobe_prov(fw, addr, data); }

void tsfi_wire_firmware_load_struct(LauWireFirmware *fw, void *ptr) {
    if (!fw || !ptr) return;
    char lbuf[128];
    snprintf(lbuf, sizeof(lbuf), "FIRMWARE_PROV_LOAD: PTR=%p", ptr);
    lau_log_status(lbuf);
}

void tsfi_wire_firmware_load_manifest(LauWireFirmware *fw, void *manifest) { (void)fw; (void)manifest; }
void tsfi_wire_firmware_load_waveform(LauWireFirmware *fw, int reg_idx, void *wave_ptr) { (void)fw; (void)reg_idx; (void)wave_ptr; }
void tsfi_wire_firmware_math_strobe(LauWireFirmware *fw) { (void)fw; }
void tsfi_wire_firmware_validate(LauWireFirmware *fw) {
    if (!fw) return;
    char *buf = (char *)(uintptr_t)fw->rtl.log_stdout_ptr;
    if (!buf) return;
    char *p = buf;
    while ((p = strstr(p, "SECRET")) != NULL) {
        memcpy(p, "******", 6);
        p += 6;
    }
}
void tsfi_wire_firmware_execute(LauWireFirmware *fw, void *handle, void *context) {
    if (!fw || !handle) return;
    void (*thunk)(void*) = (void (*)(void*))handle;
    thunk(context);
    fw->rtl.exec_done = 1;
}

static ssize_t cell_inject_stdin_impl(int session_id, const char *data, size_t len) {
    (void)session_id;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (!fw || !data || len == 0) return 0;
    
    LauWireLog *log = &fw->wavefront.stdin_log;
    uint32_t head = *log->head;
    uint32_t tail = *log->tail;
    
    size_t written = 0;
    while (written < len) {
        uint32_t next = (head + 1) % log->capacity;
        if (next == tail) break; // Full
        log->buffer[head] = data[written++];
        head = next;
    }
    *log->head = head;
    return written;
}

static size_t cell_extract_stdout_impl(int session_id, char *buf, size_t max_len) {
    (void)session_id;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (!fw || !buf || max_len == 0) return 0;
    
    LauWireLog *log = &fw->wavefront.stdout_log;
    uint32_t head = *log->head;
    uint32_t tail = *log->tail;
    
    size_t extracted = 0;
    while (extracted < max_len && tail != head) {
        buf[extracted++] = log->buffer[tail];
        tail = (tail + 1) % log->capacity;
    }
    *log->tail = tail;
    return extracted;
}
static void cell_brush_teeth_impl(int timeout_sec) { 
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw) {
        // Physical Scrubbing: wipe logical state from hardware grids
        memset(&fw->vram, 0, sizeof(LauVRAM));
    }
    if (timeout_sec > 0) alarm(timeout_sec);
}
static void cell_mcp_execute_impl(LauMasterWavefront *mw) {
    if (!mw) return;
    if (mw->command_id == 1) { // READ
        mw->result = *(uint64_t*)mw->arg0;
        mw->status = 2; // DONE
    } else if (mw->command_id == 3) { // CALL
        void (*fn)(void) = (void (*)(void))mw->arg0;
        if (fn) fn();
        mw->status = 2; // DONE
    }
}
static void cell_wave_exec_impl(uint32_t op, uint8_t dest, uint8_t src1, uint8_t src2) { 
    (void)op; (void)dest; (void)src1; (void)src2; 
    // Execute thunk if op is a function pointer (legacy compatibility)
    if (op > 0x1000) {
        void (*fn)(void) = (void (*)(void))(uintptr_t)op;
        fn();
    }
}
static void cell_neural_observe_impl(int session_id, char *out_intent, char *out_insight) { 
    (void)session_id;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (!fw) return;
    
    LauWireLog *log = &fw->wavefront.stdout_log;
    if (!log->buffer) return;
    
    uint32_t head = *log->head;
    uint32_t tail = *log->tail;
    uint32_t cap = log->capacity;
    
    // Create a temporary linear buffer for parsing (sampling only)
    char *linear = (char *)malloc(cap + 1);
    uint32_t len = 0;
    uint32_t t = tail;
    while (t != head) {
        linear[len++] = log->buffer[t];
        t = (t + 1) % cap;
    }
    linear[len] = '\0';
    
    // Parse [INTENT]
    char *p = strstr(linear, "[INTENT] ");
    if (p && out_intent) {
        p += 9;
        char *end = strchr(p, '\n');
        if (end) {
            size_t n = end - p;
            if (n > 255) n = 255;
            memcpy(out_intent, p, n);
            out_intent[n] = '\0';
        } else strcpy(out_intent, p);
    }
    
    // Parse [INSIGHT]
    p = strstr(linear, "[INSIGHT] ");
    if (p && out_insight) {
        p += 10;
        char *end = strchr(p, '\n');
        if (end) {
            size_t n = end - p;
            if (n > 1023) n = 1023;
            memcpy(out_insight, p, n);
            out_insight[n] = '\0';
        } else strcpy(out_insight, p);
    }
    
    free(linear);
}
static void cell_zhong_sync_wait_impl(uint64_t handle, uint64_t val) { (void)handle; (void)val; }
static void cell_zhong_sync_signal_impl(uint64_t handle, uint64_t val) { (void)handle; (void)val; }
static void cell_zhong_atomic_wait_impl(uint64_t addr, uint64_t val) { (void)addr; (void)val; }
static void cell_seal_capture_impl(void *ptr) { (void)ptr; }
static void cell_seal_release_impl(void *ptr) { (void)ptr; }
static void cell_reg_insert_impl(LauMetadata *m) { if (m) lau_registry_insert(m); }
static void cell_reg_remove_impl(void *ptr) { if (ptr) { LauMetadata *m = lau_registry_find(ptr); if (m) lau_registry_remove(m); } }
#include <sys/signalfd.h>
#include <poll.h>
static int g_fw_sfd = -1;
static bool g_fw_first_run = true;

static int cell_hardware_poll_impl(int timeout_ms, char *out_stdin_buf, size_t max_len) {
    if (out_stdin_buf && max_len > 0) out_stdin_buf[0] = '\0';

    if (g_fw_first_run) {
        g_fw_first_run = false;
        if (out_stdin_buf && max_len > 1) {
            out_stdin_buf[0] = '\n';
            out_stdin_buf[1] = '\0';
        }
        return 1;
    }

    struct pollfd fds[2];
    int nfds = 0;
    
    int stdin_idx = -1;
    int sig_idx = -1;

    fds[nfds].fd = STDIN_FILENO;
    fds[nfds].events = POLLIN;
    stdin_idx = nfds++;

    if (g_fw_sfd != -1) {
        fds[nfds].fd = g_fw_sfd;
        fds[nfds].events = POLLIN;
        sig_idx = nfds++;
    }

    int ret = poll(fds, nfds, timeout_ms);
    if (ret <= 0) return ret;

    if (sig_idx != -1 && (fds[sig_idx].revents & POLLIN)) {
        struct signalfd_siginfo fdsi;
        ssize_t s = read(g_fw_sfd, &fdsi, sizeof(struct signalfd_siginfo));
        if (s == sizeof(struct signalfd_siginfo)) {
            if (fdsi.ssi_signo == SIGINT || fdsi.ssi_signo == SIGTERM) return -1;
            if (fdsi.ssi_signo == SIGWINCH) return 2; // Geometric resize vector
        }
    }

    if (stdin_idx != -1 && (fds[stdin_idx].revents & (POLLIN | POLLHUP | POLLERR))) {
        if (out_stdin_buf && max_len > 0) {
            if (!fgets(out_stdin_buf, max_len, stdin)) {
                return -1;
            }
        }
    }

    return ret;
}

static LauMetadata* cell_reg_lookup_impl(void *ptr) { return ptr ? lau_registry_find(ptr) : NULL; }
static void cell_reg_strobe_zmm_impl(int zmm_idx, void *ptr) { (void)zmm_idx; (void)ptr; }

static void* cell_mem_genesis_impl(size_t size, LauHeaderType type, const char *file, int line) {
    size_t h_size = (type == LAU_TYPE_WIRED) ? 7168 : ((type == LAU_TYPE_GPU) ? 64 : sizeof(LauHeader));
    size_t padded_size = (size + 511) & ~511;
    size_t total_size = h_size + padded_size;
    
    // Zhong-Preferred Allocation Path
    extern void* lau_rebar_alloc_external(size_t size);
    void *block = lau_rebar_alloc_external(total_size);
    if (!block) {
        block = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    }
    
    if (block == MAP_FAILED || !block) return NULL;
    
    // Physical Clean Slate: Ensure hardware cell is verifiably blank
    memset(block, 0, total_size);
    
    void *payload = (char*)block + h_size;
    LauFooter *f = (LauFooter *)((char*)payload - sizeof(LauFooter));
    f->magic = LAU_MAGIC;
    f->type = (uint8_t)type;
    f->alloc_line = (uint16_t)line;

    LauMetadata *m = (LauMetadata *)block; // Top of structure
    m->alloc_size = total_size | ((size_t)type << 56);
    m->actual_start = block;
    m->alloc_file = file;
    m->current_prot = PROT_READ | PROT_WRITE;
    
    // Bind to Physical Reality immediately using external topography-aware checksums
    if (type == LAU_TYPE_WIRED) f->checksum = calculate_checksum_wired_external(block);
    else f->checksum = calculate_checksum_basic_external(block);
    
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw && fw->cell_reg_insert) fw->cell_reg_insert(m);
    else lau_registry_insert(m);
    
    extern _Atomic size_t g_alloc_count;
    extern _Atomic size_t g_alloc_bytes;
    extern _Atomic size_t g_active_allocs;
    extern _Atomic size_t g_max_active_bytes;
    extern _Atomic size_t g_active_bytes;
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    return payload;
}

static void cell_mem_reclaim_impl(void *ptr, size_t size) {
    if (!ptr || size == 0) return;
    
    // Physical Subsystem Cleanup: ensure metadata is removed before physical unmap
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw) {
        // Manually remove from master manifold if it exists
        uint32_t count = atomic_load(&fw->manifold.count);
        for (uint32_t i = 0; i < count; i++) {
            if (fw->manifold.search_index[i] == (uintptr_t)ptr) {
                uint32_t last = atomic_fetch_sub(&fw->manifold.count, 1) - 1;
                if (i != last) {
                    fw->manifold.slots[i] = fw->manifold.slots[last];
                    fw->manifold.search_index[i] = fw->manifold.search_index[last];
                }
                break;
            }
        }
    }

    void *stack_top = &ptr;
    if ((uintptr_t)stack_top >= (uintptr_t)ptr && (uintptr_t)stack_top < (uintptr_t)ptr + size) return;
    if (g_tsfi_firmware && (uintptr_t)g_tsfi_firmware >= (uintptr_t)ptr && (uintptr_t)g_tsfi_firmware < (uintptr_t)ptr + size) return;
    syscall(SYS_munmap, ptr, size);
}

static void fw_logic_directive(int *status, char *cmd) { (void)status; (void)cmd; }
static void default_firmware_epoch(void *obj) { tsfi_wire_firmware_validate((LauWireFirmware *)obj); }
static bool default_firmware_state(void *obj) { (void)obj; return true; }
extern void tsfi_registry_backfill_manifold(void);

static int cell_printf_impl(int session_id, const char *format, ...) {
    FILE *stream = (session_id < 0) ? stderr : stdout;
    va_list args; va_start(args, format);
    int ret = vfprintf(stream, format, args); 
    va_end(args); 
    fflush(stream);
    return ret;
}

static void tsfi_sigchld_handler(int sig) {
    (void)sig;
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

void tsfi_wire_firmware_setup_signals(void) {
    // Geometrically map the sub-process reaper asynchronously
    struct sigaction sa;
    sa.sa_handler = tsfi_sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGWINCH); // Capture terminal geometry scaling
    
    // Note: Do not block SIGCHLD here as it breaks waitpid/system in tests.
    sigprocmask(SIG_BLOCK, &mask, NULL);
    g_fw_sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
    if (g_fw_sfd != -1) {
        printf("[TSFI_FIRMWARE] Interrupt Controller Active (sfd: %d)\n", g_fw_sfd);
    }
}

void tsfi_wire_firmware_teardown(void) {
    if (!g_tsfi_firmware) return;
    if (g_tsfi_firmware->wavefront.stdin_log.buffer) { lau_unseal_object(g_tsfi_firmware->wavefront.stdin_log.buffer); lau_free(g_tsfi_firmware->wavefront.stdin_log.buffer); }
    if (g_tsfi_firmware->wavefront.stdout_log.buffer) { lau_unseal_object(g_tsfi_firmware->wavefront.stdout_log.buffer); lau_free(g_tsfi_firmware->wavefront.stdout_log.buffer); }
    
    // Manual core_yi teardown to bypass recycleYI caching the embedded pointer
    extern void freeSHIO(struct SHIO*);
    if (g_tsfi_firmware->core_yi.Psi) freeSHIO(g_tsfi_firmware->core_yi.Psi);
    if (g_tsfi_firmware->core_yi.Xi) tsfi_bn_free(g_tsfi_firmware->core_yi.Xi);
    if (g_tsfi_firmware->core_yi.Ring) tsfi_bn_free(g_tsfi_firmware->core_yi.Ring);
    
    // Drain static memory pools to resolve final active leaks
    extern void tsfi_bn_drain_pool(void);
    extern void tsfi_dysnomia_drain_pools(void);
    tsfi_bn_drain_pool();
    tsfi_dysnomia_drain_pools();
    
    lau_unseal_object(g_tsfi_firmware);
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
        fw->rx_pin_pty_out = rx_pin_pty_out_impl; fw->tx_pin_pty_in = tx_pin_pty_in_impl;
        // ... (rest of function bindings)
    }

    g_tsfi_firmware = fw;
    
    if (is_creator) {
        // Initial setup for the creator process
        fw->cell_inject_stdin = cell_inject_stdin_impl; fw->cell_extract_stdout = cell_extract_stdout_impl;
        fw->cell_brush_teeth = cell_brush_teeth_impl;
        fw->cell_mcp_execute = cell_mcp_execute_impl; fw->cell_wave_exec = cell_wave_exec_impl;
        fw->cell_neural_observe = cell_neural_observe_impl; fw->cell_printf = cell_printf_impl;
        fw->cell_zhong_sync_wait = cell_zhong_sync_wait_impl; fw->cell_zhong_sync_signal = cell_zhong_sync_signal_impl;
        fw->cell_zhong_atomic_wait = cell_zhong_atomic_wait_impl; fw->cell_seal_capture = cell_seal_capture_impl;
        fw->cell_seal_release = cell_seal_release_impl; fw->cell_reg_insert = cell_reg_insert_impl;
        fw->cell_reg_remove = cell_reg_remove_impl; fw->cell_reg_lookup = cell_reg_lookup_impl;
        fw->cell_reg_strobe_zmm = cell_reg_strobe_zmm_impl; fw->cell_mem_genesis = cell_mem_genesis_impl;
        fw->cell_mem_reclaim = cell_mem_reclaim_impl;
        fw->cell_hardware_poll = cell_hardware_poll_impl;

        memset(&fw->manifold, 0, sizeof(LauRegistryManifold));
        tsfi_registry_backfill_manifold();
        if (!g_fw_stdin_log) g_fw_stdin_log = fopen("logs/firmware_stdin.log", "ab");
        if (!g_fw_stdout_log) g_fw_stdout_log = fopen("logs/firmware_stdout.log", "ab");
        g_tsfi_firmware->rtl.reset_strobe = true; LauWireFirmware_eval_sequential(&g_tsfi_firmware->rtl);
        g_tsfi_firmware->rtl.reset_strobe = false;
        
        LauTelemetryState* boot_telem = lau_telemetry_get_state();
        if (boot_telem) {
            snprintf((char*)boot_telem->recip_dai, 128, "0x3632C8EB5AF3B");
            boot_telem->recip_symmetry = 1.0f;
        }
        
        // Use fixed header offset for SHM mapping
        LauSystemHeader *h = (LauSystemHeader *)((char *)g_tsfi_firmware + fw_size - 7168);
        h->logic_directive = (void (*)(int*, char*))fw_logic_directive;
        h->logic_epoch = (void (*)(int*))default_firmware_epoch;
        h->logic_state = (bool (*)(void*))default_firmware_state;
        lau_vram_init(&g_tsfi_firmware->vram);
        
        void* stdin_buf = lau_malloc_wired(4096);
        void* stdout_buf = lau_malloc_wired(4096);
        
        g_tsfi_firmware->wavefront.stdin_log.buffer = (char*)stdin_buf;
        g_tsfi_firmware->wavefront.stdin_log.head = &g_tsfi_firmware->rtl.log_stdin_head;
        g_tsfi_firmware->wavefront.stdin_log.tail = &g_tsfi_firmware->rtl.log_stdin_tail;
        g_tsfi_firmware->wavefront.stdin_log.lock = (_Atomic uint32_t *)&g_tsfi_firmware->rtl.log_stdin_lock;
        g_tsfi_firmware->wavefront.stdin_log.capacity = 4096;
        g_tsfi_firmware->rtl.log_stdin_ptr = (uintptr_t)stdin_buf;
        g_tsfi_firmware->wavefront.stdout_log.buffer = (char*)stdout_buf;
        g_tsfi_firmware->wavefront.stdout_log.head = &g_tsfi_firmware->rtl.log_stdout_head;
        g_tsfi_firmware->wavefront.stdout_log.tail = &g_tsfi_firmware->rtl.log_stdout_tail;
        g_tsfi_firmware->wavefront.stdout_log.valid_head = &g_tsfi_firmware->rtl.log_stdout_valid_head;
        g_tsfi_firmware->wavefront.stdout_log.lock = (_Atomic uint32_t *)&g_tsfi_firmware->rtl.log_stdout_lock;
        g_tsfi_firmware->wavefront.stdout_log.capacity = 4096;
        g_tsfi_firmware->rtl.log_stdout_ptr = (uintptr_t)stdout_buf;
        
        tsfi_reaction_boot_firmware_yi(&g_tsfi_firmware->core_yi);
    }

    atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
}

LauWireFirmware* tsfi_wire_firmware_get(void) { if (!g_tsfi_firmware) tsfi_wire_firmware_init(); return g_tsfi_firmware; }
LauWireFirmware* tsfi_wire_firmware_get_no_init(void) { extern LauWireFirmware *g_tsfi_firmware; return g_tsfi_firmware; }
int tsfi_wire_firmware_get_session_fd(int session_id) { if (session_id >= 0 && session_id < 64 && g_sessions[session_id]) return g_sessions[session_id]->master_fd; return -1; }

void tsfi_wire_firmware_step_peripheral(LauWireFirmware *fw, void *pty_obj) {
    if (!fw) return;
    LauWirePty *pty = (LauWirePty *)pty_obj;
    LauWireFirmware_eval_combinatorial(&fw->rtl);
    if (fw->rtl.zhong_strobe && !fw->rtl.zhong_done) {
        switch (fw->rtl.zhong_op) {
            case 0x10: if (fw->cell_zhong_sync_wait) fw->cell_zhong_sync_wait(fw->rtl.zhong_timeline_handle, fw->rtl.zhong_timeline_wait_val); break;
            case 0x11: if (fw->cell_zhong_sync_signal) fw->cell_zhong_sync_signal(fw->rtl.zhong_timeline_handle, fw->rtl.zhong_timeline_sig_val); break;
            case 0x20: { uint8_t *rebar = (uint8_t *)(uintptr_t)fw->rtl.zhong_rebar_ptr; if (rebar && fw->rtl.zhong_rebar_size >= 512) memcpy(&fw->rtl.wrf[0], rebar, 512); break; }
            case 0x21: { uint8_t *rebar = (uint8_t *)(uintptr_t)fw->rtl.zhong_rebar_ptr; if (rebar && fw->rtl.zhong_rebar_size >= 512) memcpy(rebar, &fw->rtl.wrf[0], 512); break; }
            case 0x30: if (fw->cell_zhong_atomic_wait) fw->cell_zhong_atomic_wait(fw->rtl.zhong_timeline_handle, fw->rtl.zhong_timeline_wait_val); break;
        }
        fw->rtl.zhong_done = true; fw->rtl.zhong_status = 1; LauWireFirmware_eval_combinatorial(&fw->rtl);
    } else if (!fw->rtl.zhong_strobe) { fw->rtl.zhong_done = false; fw->rtl.zhong_status = 0; }
    int sid = fw->rtl.session_id % 64;
    if (!pty) pty = g_sessions[sid];
    if (fw->rtl.periph_directive_strobe) {
        if (fw->rtl.periph_directive_cmd == 0x47454D49 && !g_sessions[sid]) {
            char cmd_buf[65]; memset(cmd_buf, 0, 65); memcpy(cmd_buf, &fw->rtl.wrf[0], 64);
            g_sessions[sid] = tsfi_wire_pty_fork(cmd_buf);
            if (g_sessions[sid]) {
                fw->rtl.bunch_session_mask |= (1ULL << sid);
                memcpy(&g_sessions[sid]->log_stdin, &fw->wavefront.stdin_log, sizeof(LauWireLog));
                memcpy(&g_sessions[sid]->log_stdout, &fw->wavefront.stdout_log, sizeof(LauWireLog));
            }
            if (!pty) pty = g_sessions[sid];
        } else if (fw->rtl.periph_directive_cmd == 0x53544F50 && g_sessions[sid]) {
            g_sessions[sid]->is_active = false; if (g_sessions[sid]->child_pid > 0) kill(g_sessions[sid]->child_pid, SIGTERM);
            lau_free(g_sessions[sid]); g_sessions[sid] = NULL;
            fw->rtl.bunch_session_mask &= ~(1ULL << sid);
            if (pty == g_sessions[sid]) pty = NULL;
        }
    }
    fw->rtl.periph_state_ready = (pty != NULL && pty->is_active);
    LauWireFirmware_eval_combinatorial(&fw->rtl);
    if (fw->rtl.periph_epoch_strobe && pty && pty->is_active) { tsfi_wire_pty_step(pty); strobe_prov(fw, 0x0E, fw->rtl.plugin_epoch + 1); }
    else if (pty && !pty->is_active) {
        if (pty->child_pid > 0) waitpid(pty->child_pid, NULL, WNOHANG);
        if (pty == g_sessions[sid]) { lau_free(g_sessions[sid]); g_sessions[sid] = NULL; fw->rtl.bunch_session_mask &= ~(1ULL << sid); }
    }
}

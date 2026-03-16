#define _GNU_SOURCE
#include <sys/syscall.h>
#include "tsfi_io.h"
#include <fcntl.h>

int g_tsfi_verbose = 0;

#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdatomic.h>
#include <time.h>
#include "lau_audit.h"
#include "tsfi_math.h"
#include "lau_vram.h"
#include "lau_thunk.h"
#include "tsfi_time.h"
#include <poll.h>

static atomic_flag g_io_lock = ATOMIC_FLAG_INIT;
static _Atomic size_t g_io_seq = 0;

static ThunkProxy *g_poll_proxy = NULL;
static int (*g_poll_thunk)(void**, uint32_t*, int, int) = NULL;

void tsfi_io_cleanup(void) {
    if (g_poll_proxy) {
        extern void ThunkProxy_destroy_authoritative(ThunkProxy *p);
        ThunkProxy_destroy_authoritative(g_poll_proxy);
        g_poll_proxy = NULL;
        g_poll_thunk = NULL;
    }
}

int tsfi_io_wait_wavefront(void **addrs, uint32_t *masks, int count, int timeout_ms) {
    if (!g_poll_proxy) {
        g_poll_proxy = ThunkProxy_create();
        g_poll_thunk = (int (*)(void**, uint32_t*, int, int))ThunkProxy_emit_io_poll(g_poll_proxy, 32);
        ThunkProxy_seal(g_poll_proxy);
    }
    
    long long iters = (long long)timeout_ms * 100000;
    if (iters <= 0) iters = 1;
    
    return g_poll_thunk(addrs, masks, count, (int)iters);
}

int tsfi_io_wait_universal(int *fds, int fd_count, void **mem_addrs, uint32_t *mem_masks, int mem_count, int timeout_ms) {
    unsigned long long start = get_time_ns();
    unsigned long long limit = (unsigned long long)timeout_ms * 1000000ULL;
    
    while (1) {
        if (fd_count > 0) {
            struct pollfd pfds[32];
            int n = (fd_count > 32) ? 32 : fd_count;
            for (int i = 0; i < n; i++) {
                pfds[i].fd = fds[i];
                pfds[i].events = POLLIN;
                pfds[i].revents = 0;
            }
            if (poll(pfds, n, 0) > 0) {
                for (int i = 0; i < n; i++) {
                    if (pfds[i].revents & POLLIN) return i; 
                }
            }
        }

        if (mem_count > 0) {
            int res = tsfi_io_wait_wavefront(mem_addrs, mem_masks, mem_count, 1); 
            if (res >= 0) return fd_count + res;
        }

        if (timeout_ms >= 0) {
            if (get_time_ns() - start >= limit) return -1;
        }

        __asm__ volatile ("pause" ::: "memory");
    }
}

/**
 * TSFi Monitor: Transcendent replacement for select(), poll(), and epoll().
 * Monitors both traditional file descriptors and high-speed memory waveforms
 * simultaneously within the performance manifold.
 */
int tsfi_monitor(int *fds, int fd_count, void **mem_addrs, uint32_t *mem_masks, int mem_count, int timeout_ms) {
    return tsfi_io_wait_universal(fds, fd_count, mem_addrs, mem_masks, mem_count, timeout_ms);
}

static const char* level_to_str(TSFiLogLevel level) {
    switch (level) {
        case TSFI_LOG_INFO:     return "INFO";
        case TSFI_LOG_WARN:     return "WARN";
        case TSFI_LOG_FAIL:     return "FAIL";
        case TSFI_LOG_ZHONG:    return "ZHONG";
        case TSFI_LOG_PHYSICAL: return "PHYSICAL";
        case TSFI_LOG_MEMORY:   return "MEMORY";
        default:                return "UNKNOWN";
    }
}

int tsfi_io_printf(FILE* stream, const char* format, ...) {
    if (!stream) return 0;
    while (atomic_flag_test_and_set(&g_io_lock)) __asm__ volatile ("pause");
    va_list args;
    va_start(args, format);
    int res = vfprintf(stream, format, args);
    va_end(args);
    atomic_flag_clear(&g_io_lock);
    return res;
}

void tsfi_io_flush(FILE* stream) {
    while (atomic_flag_test_and_set(&g_io_lock)) __asm__ volatile ("pause");
    fflush(stream);
    atomic_flag_clear(&g_io_lock);
}

void tsfi_io_hex_dump(FILE *stream, const char *tag, const void *ptr, size_t size) {
    const uint8_t *b = (const uint8_t *)ptr;
    tsfi_io_printf(stream, "[HEX_DUMP:%s] size=%zu addr=%p\n", tag, size, ptr);
    for (size_t i = 0; i < size; i += 16) {
        tsfi_io_printf(stream, "  %04zu: ", i);
        for (int j = 0; j < 16; j++) {
            if (i + j < size) tsfi_io_printf(stream, "%02x ", b[i + j]);
            else tsfi_io_printf(stream, "   ");
        }
        tsfi_io_printf(stream, " | ");
        for (int j = 0; j < 16; j++) {
            if (i + j < size) {
                uint8_t c = b[i + j];
                tsfi_io_printf(stream, "%c", (c >= 32 && c <= 126) ? c : '.');
            }
        }
        tsfi_io_printf(stream, "\n");
    }
}

void tsfi_io_log(FILE *stream, TSFiLogLevel level, const char *tag, const char *format, ...) {
    size_t seq = atomic_fetch_add(&g_io_seq, 1);
    char date_buf[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date_buf, sizeof(date_buf), "%Y-%m-%d %H:%M:%S", t);

    FILE *out = stream ? stream : stdout;
    char msg[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(msg, sizeof(msg), format, args);
    va_end(args);

    tsfi_io_printf(out, "[SEQ:%08zu] [%s] [%s] [%s] [%s] %s\n", seq, date_buf, TSFI_PROVENANCE_KEY, level_to_str(level), tag ? tag : "CORE", msg);
}

size_t tsfi_io_write(FILE *stream, const void *ptr, size_t size) {
    while (atomic_flag_test_and_set(&g_io_lock)) __asm__ volatile ("pause");
    size_t res = fwrite(ptr, 1, size, stream);
    atomic_flag_clear(&g_io_lock);
    return res;
}

void* tsfi_io_map_file(const char* path, size_t* out_size) {
    if (!path) return NULL;

    int fd = open(path, O_RDONLY);
    if (fd == -1) return NULL;

    struct stat st;
    if (fstat(fd, &st) == -1) {
        close(fd);
        return NULL;
    }

    size_t size = st.st_size;
    if (out_size) *out_size = size;

    void *ptr = lau_malloc_wired(size);
    if (!ptr) {
        close(fd);
        return NULL;
    }

    ssize_t bytes_read = read(fd, ptr, size);
    close(fd);

    if (bytes_read != (ssize_t)size) {
        lau_free(ptr);
        return NULL;
    }

    lau_seal_object(ptr);

    return ptr;
}

size_t tsfi_io_read_vectorized(const char* path, void* dest, size_t max_size) {
    if (!path || !dest) return 0;

    FILE* f = fopen(path, "rb");
    if (!f) return 0;

    size_t total = 0;
    char buffer[4096] __attribute__((aligned(64)));
    
    while (total < max_size) {
        size_t to_read = (max_size - total > 4096) ? 4096 : (max_size - total);
        size_t n = fread(buffer, 1, to_read, f);
        if (n == 0) break;
        
        memcpy((uint8_t*)dest + total, buffer, n);
        total += n;
    }

    fclose(f);
    return total;
}

int tsfi_shm_open(const char *name, int oflag, mode_t mode) {
    char path[512];
    if (name[0] == '/') {
        snprintf(path, sizeof(path), "/dev/shm%s", name);
    } else {
        snprintf(path, sizeof(path), "/dev/shm/%s", name);
    }
    return open(path, oflag, mode);
}

int tsfi_shm_unlink(const char *name) {
    char path[512];
    if (name[0] == '/') {
        snprintf(path, sizeof(path), "/dev/shm%s", name);
    } else {
        snprintf(path, sizeof(path), "/dev/shm/%s", name);
    }
    return unlink(path);
}

int tsfi_mprotect(void *addr, size_t len, int prot) {
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (SYS_mprotect), "D" (addr), "S" (len), "d" (prot)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

int tsfi_madvise(void *addr, size_t length, int advice) {
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (SYS_madvise), "D" (addr), "S" (length), "d" (advice)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

int tsfi_memfd_create(const char *name, unsigned int flags) {
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (SYS_memfd_create), "D" (name), "S" (flags)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

int tsfi_sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask) {
    long ret;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (SYS_sched_setaffinity), "D" (pid), "S" (cpusetsize), "d" (mask)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

int tsfi_futex(uint32_t *uaddr, int op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3) {
    long ret;
    register const struct timespec *r10 __asm__("r10") = timeout;
    register uint32_t *r8 __asm__("r8") = uaddr2;
    register uint32_t r9 __asm__("r9") = val3;
    __asm__ volatile (
        "syscall\n\t"
        : "=a" (ret)
        : "a" (SYS_futex), "D" (uaddr), "S" (op), "d" (val), "r" (r10), "r" (r8), "r" (r9)
        : "rcx", "r11", "memory"
    );
    return (int)ret;
}

#include "tsfi_k0rn_ops.h"
#include "tsfi_wave15.h"

#define DNA_SCALE 16384.0f

K0RnStream* tsfi_k0rn_load(const char *name) {
    char path[1024];
    snprintf(path, sizeof(path), "assets/%s.k0rn", name);
    int fd = open(path, O_RDONLY);
    if (fd == -1) return NULL;

    K0RnStream *s = (K0RnStream*)lau_malloc_wired(sizeof(K0RnStream));
    if (read(fd, s, sizeof(K0RnStream)) != sizeof(K0RnStream)) {
        close(fd); lau_free(s); return NULL;
    }

    s->ops = (K0RnOp*)lau_memalign(512, sizeof(K0RnOp) * s->stream_capacity);
    if (read(fd, s->ops, sizeof(K0RnOp) * s->op_count) != (ssize_t)(sizeof(K0RnOp) * s->op_count)) {
        close(fd); lau_free(s->ops); lau_free(s); return NULL;
    }

    close(fd);
    return s;
}

bool tsfi_k0rn_save(const char *name, const K0RnStream *s) {
    char path[1024];
    snprintf(path, sizeof(path), "assets/%s.k0rn", name);
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) return false;

    if (write(fd, s, sizeof(K0RnStream)) != sizeof(K0RnStream)) {
        close(fd); return false;
    }
    if (write(fd, s->ops, sizeof(K0RnOp) * s->op_count) != (ssize_t)(sizeof(K0RnOp) * s->op_count)) {
        close(fd); return false;
    }
    close(fd);
    return true;
}

size_t tsfi_io_grep(const uint8_t* buffer, size_t buffer_size, const char* pattern, size_t pattern_len, size_t* match_indices, size_t max_matches) {
    if (!buffer || buffer_size == 0 || !pattern || pattern_len == 0 || !match_indices || max_matches == 0) return 0;
    if (buffer_size < pattern_len) return 0;

    size_t match_count = 0;
    for (size_t i = 0; i <= buffer_size - pattern_len; i++) {
        if (memcmp(buffer + i, pattern, pattern_len) == 0) {
            match_indices[match_count++] = i;
            if (match_count >= max_matches) break;
        }
    }
    return match_count;
}

size_t tsfi_io_grep_context(const uint8_t* buffer, size_t buffer_size, const char* pattern, size_t pattern_len,
                            size_t* match_starts, size_t* match_ends, size_t max_matches, int before_lines, int after_lines) {
    (void)before_lines; (void)after_lines;
    size_t match_indices[max_matches];
    size_t count = tsfi_io_grep(buffer, buffer_size, pattern, pattern_len, match_indices, max_matches);
    
    for (size_t i = 0; i < count; i++) {
        size_t start = match_indices[i];
        while (start > 0 && buffer[start-1] != '\n') start--;
        size_t end = match_indices[i] + pattern_len;
        while (end < buffer_size && buffer[end] != '\n') end++;
        match_starts[i] = start;
        match_ends[i] = end;
    }
    return count;
}

size_t tsfi_io_grep_masked(const uint8_t* buffer, size_t buffer_size, const char* pattern, const uint8_t* ignore_mask,
                           size_t pattern_len, size_t* match_indices, size_t max_matches) {
    size_t count = 0;
    for (size_t i = 0; i <= buffer_size - pattern_len; i++) {
        bool match = true;
        for (size_t j = 0; j < pattern_len; j++) {
            if (ignore_mask[j]) continue;
            if (buffer[i + j] != (uint8_t)pattern[j]) {
                match = false; break;
            }
        }
        if (match) {
            match_indices[count++] = i;
            if (count >= max_matches) break;
        }
    }
    return count;
}

size_t tsfi_vram_grep(const LauVRAM* vram, const char* pattern, size_t pattern_len, size_t* match_row, size_t* match_col, size_t max_matches) {
    size_t match_count = 0;
    for (int r = 0; r < LAU_VRAM_ROWS; r++) {
        for (int c = 0; c <= LAU_VRAM_COLS - (int)pattern_len; c++) {
            bool match = true;
            for (size_t i = 0; i < pattern_len; i++) {
                if (vram->grid[r][c + i].character != (uint8_t)pattern[i]) {
                    match = false; break;
                }
            }
            if (match) {
                match_row[match_count] = r;
                match_col[match_count] = c;
                match_count++;
                if (match_count >= max_matches) return match_count;
            }
        }
    }
    return match_count;
}

size_t tsfi_vram_grep_masked(const LauVRAM* vram, const char* pattern, const uint8_t* ignore_mask, size_t pattern_len, size_t* match_row, size_t* match_col, size_t max_matches) {
    size_t match_count = 0;
    for (int r = 0; r < LAU_VRAM_ROWS; r++) {
        for (int c = 0; c <= LAU_VRAM_COLS - (int)pattern_len; c++) {
            bool match = true;
            for (size_t i = 0; i < pattern_len; i++) {
                if (ignore_mask[i] == 0 && vram->grid[r][c + i].character != (uint8_t)pattern[i]) {
                    match = false; break;
                }
            }
            if (match) {
                match_row[match_count] = r;
                match_col[match_count] = c;
                match_count++;
                if (match_count >= max_matches) return match_count;
            }
        }
    }
    return match_count;
}

int tsfi_persist_manifold(const char *path, const void *manifold, size_t size) {
    if (!path || !manifold) return -1;
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) return -1;
    ssize_t w = write(fd, manifold, (size_t)size);
    if (close(fd) != 0) return -1;
    return (w == (ssize_t)size) ? 0 : -1;
}

int tsfi_restore_manifold(const char *path, void *manifold, size_t size) {
    if (!path || !manifold) return -1;
    int fd = open(path, O_RDONLY);
    if (fd == -1) return -1;
    ssize_t r = read(fd, manifold, (size_t)size);
    if (close(fd) != 0) return -1;
    return (r == (ssize_t)size) ? 0 : -1;
}

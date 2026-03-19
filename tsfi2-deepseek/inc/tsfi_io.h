#ifndef TSFI_IO_H
#define TSFI_IO_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "tsfi_io_types.h"
#include "tsfi_k0rn_ops.h"
#include "lau_vram.h"

// Native AVX-512 Grep Prototypes
size_t tsfi_io_grep(const uint8_t* buffer, size_t buffer_size, const char* pattern, size_t pattern_len, size_t* match_indices, size_t max_matches);
size_t tsfi_io_grep_context(const uint8_t* buffer, size_t buffer_size, const char* pattern, size_t pattern_len,
                            size_t* match_starts, size_t* match_ends, size_t max_matches, int before_lines, int after_lines);
size_t tsfi_io_grep_masked(const uint8_t* buffer, size_t buffer_size, const char* pattern, const uint8_t* ignore_mask,
                           size_t pattern_len, size_t* match_indices, size_t max_matches);

// VRAM Grep (Vulkan Bridge)
size_t tsfi_vram_grep(const LauVRAM* vram, const char* pattern, size_t pattern_len, size_t* match_row, size_t* match_col, size_t max_matches);
size_t tsfi_vram_grep_masked(const LauVRAM* vram, const char* pattern, const uint8_t* ignore_mask, size_t pattern_len, size_t* match_row, size_t* match_col, size_t max_matches);

// Standard IO Wrappers (Consistent formatting)
extern int g_tsfi_verbose;
#define TSFI_DEBUG_LOG(fmt, ...) do { if (g_tsfi_verbose) tsfi_io_printf(stdout, fmt, ##__VA_ARGS__); } while (0)
#define TSFI_DEBUG_FPRINTF(stream, fmt, ...) do { if (g_tsfi_verbose) { fprintf(stream, fmt, ##__VA_ARGS__); fflush(stream); } } while (0)

int tsfi_io_printf(FILE* stream, const char* format, ...);
void tsfi_io_flush(FILE* stream);
void tsfi_io_hex_dump(FILE *stream, const char *tag, const void *ptr, size_t size);

// File Operations
void* tsfi_io_map_file(const char* path, size_t* out_size);
size_t tsfi_io_read_vectorized(const char* path, void* dest, size_t max_size);

// Shared Memory & Memory Management (TSFi Native)
int tsfi_shm_open(const char *name, int oflag, mode_t mode);
int tsfi_shm_unlink(const char *name);
int tsfi_sync_manifold_to_shm(const char *disk_path, const char *shm_name, size_t size);
int tsfi_persist_manifold(const char *path, const void *manifold, size_t size);
int tsfi_restore_manifold(const char *path, void *manifold, size_t size);
int tsfi_mprotect(void *addr, size_t len, int prot);
int tsfi_madvise(void *addr, size_t length, int advice);
int tsfi_memfd_create(const char *name, unsigned int flags);

// Scheduling & Synchronization (Hardware Alignment)
int tsfi_sched_setaffinity(pid_t pid, size_t cpusetsize, const void *mask);
int tsfi_futex(uint32_t *uaddr, int op, uint32_t val, const struct timespec *timeout, uint32_t *uaddr2, uint32_t val3);

// Assembly Thunk I/O (Bypasses select/poll)
int tsfi_io_wait_wavefront(void **addrs, uint32_t *masks, int count, int timeout_ms);

// Universal Hybrid Wait (FDs + Memory)
int tsfi_io_wait_universal(int *fds, int fd_count, void **mem_addrs, uint32_t *mem_masks, int mem_count, int timeout_ms);
int tsfi_monitor(int *fds, int fd_count, void **mem_addrs, uint32_t *mem_masks, int mem_count, int timeout_ms);

// Thread-safe binary write
size_t tsfi_io_write(FILE *stream, const void *ptr, size_t size);

// K0Rn Persistence API (Implemented in src/tsfi_io.c)
K0RnStream* tsfi_k0rn_load(const char *name);
bool tsfi_k0rn_save(const char *name, const K0RnStream *s);

typedef struct TSFiHelmholtzSVDAG TSFiHelmholtzSVDAG;

// Infinite Journal API
void tsfi_journal_init(const char *path);
void tsfi_journal_append(TsfiWavelet *W);
void tsfi_journal_want_to_remember(TsfiWavelet *W, float priority, const char *reason);
void tsfi_journal_anchor_fracture_memory(void *manifold_shm, uint64_t svdag_id, const TSFiHelmholtzSVDAG *dag);
void tsfi_journal_recover(TsfiWaveletArena *arena);
void tsfi_journal_teardown(void);

#endif // TSFI_IO_H

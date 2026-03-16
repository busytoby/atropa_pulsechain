#include "lau_registry.h"
#include "lau_telemetry.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_io.h"
#include <stdatomic.h>
#include <stddef.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <nmmintrin.h>
#include <immintrin.h>
#include <errno.h>

extern _Atomic size_t g_active_allocs;
extern _Atomic size_t g_active_bytes;

_Atomic int g_init_in_progress = 0;
_Atomic int g_teardown_in_progress = 0;

LauMetadata *g_head = NULL;
static _Atomic int g_lock = 0;
static LauTelemetryState *g_telem = NULL;
static LauRegistryManifold *g_local_manifold = NULL;

static inline void lau_spin_lock(_Atomic int *lock) {
    while (atomic_exchange_explicit(lock, 1, memory_order_acquire)) {
        __builtin_ia32_pause();
    }
}

static inline void lau_spin_unlock(_Atomic int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);
}

static void report_event(uint64_t ptr, uint64_t size, uint32_t type, uint32_t flags);

__attribute__((constructor))
void lau_registry_init_telemetry(void) {
    static bool in_init = false;
    if (in_init || g_local_manifold) return;
    in_init = true;
    
    setenv("TSFI_LEAK_TRACKING_IN_PROGRESS", "0", 1);
    atomic_store_explicit(&g_init_in_progress, 1, memory_order_relaxed);

    g_local_manifold = (LauRegistryManifold*)lau_memalign_wired(64, sizeof(LauRegistryManifold));
    if (g_local_manifold) {
        memset(g_local_manifold, 0, sizeof(LauRegistryManifold));
        atomic_fetch_sub(&g_active_allocs, 1);
        atomic_fetch_sub(&g_active_bytes, sizeof(LauRegistryManifold));
    }

    atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
    in_init = false;

    const char *id = getenv("TSFI_TELEMETRY_ID");
    if (!id) return;
    char name[256];
    snprintf(name, sizeof(name), "/tsfi_telem_%s", id);
    int fd = shm_open(name, O_RDWR | O_CREAT, 0666);
    if (fd == -1) return;
    if (ftruncate(fd, sizeof(LauTelemetryState)) == -1) { close(fd); return; }
    g_telem = mmap(NULL, sizeof(LauTelemetryState), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
}

static inline uint64_t get_time_ns_fast() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

static void lau_telemetry_integrity_update(LauTelemetryState *s) {
    if (!s) return;
    atomic_fetch_add(&s->sequence_number, 1);
    uint64_t crc = 0;
    crc = _mm_crc32_u64(crc, s->magic);
    crc = _mm_crc32_u64(crc, s->pid);
    crc = _mm_crc32_u64(crc, atomic_load(&s->total_allocs));
    crc = _mm_crc32_u64(crc, atomic_load(&s->total_frees));
    crc = _mm_crc32_u64(crc, atomic_load(&s->active_allocs));
    crc = _mm_crc32_u64(crc, atomic_load(&s->active_bytes));
    crc = _mm_crc32_u64(crc, atomic_load(&s->peak_bytes));
    crc = _mm_crc32_u64(crc, atomic_load(&s->exec_steps));
    crc = _mm_crc32_u64(crc, atomic_load(&s->exec_last_ts));
    uint64_t *dir_ptr = (uint64_t*)s->last_directive_str;
    crc = _mm_crc32_u64(crc, *dir_ptr);
    crc = _mm_crc32_u64(crc, (uint64_t)atomic_load(&s->recip_symmetry));
    atomic_store(&s->header_crc, (uint32_t)crc);
}

static void report_event(uint64_t ptr, uint64_t size, uint32_t type, uint32_t flags) {
    if (!g_telem) return;
    if (type == 3) {
        atomic_fetch_sub(&g_telem->active_allocs, 1);
        atomic_fetch_sub(&g_telem->active_bytes, size);
        atomic_fetch_add(&g_telem->total_frees, 1);
    } else {
        atomic_fetch_add(&g_telem->total_allocs, 1);
        atomic_fetch_add(&g_telem->active_allocs, 1);
        uint64_t curr = atomic_fetch_add(&g_telem->active_bytes, size) + size;
        uint64_t peak = atomic_load(&g_telem->peak_bytes);
        while (curr > peak && !atomic_compare_exchange_weak(&g_telem->peak_bytes, &peak, curr));
    }
    uint32_t head = atomic_fetch_add(&g_telem->event_head, 1);
    uint32_t idx = head % LAU_TELEM_RING_SIZE;
    LauTelemetryEvent *e = &g_telem->events[idx];
    e->timestamp = get_time_ns_fast();
    e->ptr_addr = ptr;
    e->size = size;
    e->type = type;
    e->flags = flags;
    lau_telemetry_integrity_update(g_telem);
}

void lau_registry_report_event(uint64_t ptr, uint64_t size, uint32_t type, uint32_t flags) {
    report_event(ptr, size, type, flags);
}

static void manifold_insert(LauRegistryManifold *m, LauMetadata *md) {
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
    if (!m || !md) return;
    uint32_t count = atomic_load(&m->count);
    if (count < LAU_REGISTRY_CAPACITY) {
        LauMetadata *slot = &m->slots[count];
        slot->actual_start = md->actual_start;
        slot->alloc_size = md->alloc_size;
        slot->alloc_file = md->alloc_file;
        slot->probe_latency = md->probe_latency;
        slot->physical_tier = md->physical_tier;
        slot->current_prot = md->current_prot;
        slot->next = NULL; slot->prev = NULL; 
        m->search_index[count] = (uintptr_t)md->actual_start;
        atomic_fetch_add(&m->count, 1);
    }
}

static void manifold_remove(LauRegistryManifold *m, void *ptr) {
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
    if (!m || !ptr) return;
    uint32_t count = atomic_load(&m->count);
    uintptr_t target = (uintptr_t)ptr;
    for (uint32_t i = 0; i < count; i++) {
        if (m->search_index[i] == target) {
            uint32_t last = atomic_fetch_sub(&m->count, 1) - 1;
            if (i != last) {
                m->slots[i] = m->slots[last];
                m->search_index[i] = m->search_index[last];
            }
            break;
        }
    }
}

static void safe_update_ptr(LauMetadata **ptr_in_node, LauMetadata *val, uint8_t current_prot) {
    if (current_prot == PROT_READ || current_prot == (PROT_READ|PROT_EXEC)) {
        if (tsfi_mprotect((void*)((uintptr_t)ptr_in_node & ~0xFFFULL), 4096, PROT_READ | PROT_WRITE) != 0) {
            fprintf(stderr, "[LAU_REGISTRY] FATAL: Failed to unseal metadata at %p (errno=%d)\n", (void*)ptr_in_node, errno);
            abort();
        }
        *ptr_in_node = val;
        if (tsfi_mprotect((void*)((uintptr_t)ptr_in_node & ~0xFFFULL), 4096, current_prot) != 0) {
            fprintf(stderr, "[LAU_REGISTRY] FATAL: Failed to re-seal metadata at %p\n", (void*)ptr_in_node);
            abort();
        }
    } else {
        *ptr_in_node = val;
    }
}

void lau_registry_insert(LauMetadata *m) {
    lau_spin_lock(&g_lock);
    m->next = g_head;
    m->prev = NULL;
    if (g_head) {
        safe_update_ptr(&g_head->prev, m, g_head->current_prot);
    }
    g_head = m;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw) manifold_insert(&fw->manifold, m);
    else if (g_local_manifold) manifold_insert(g_local_manifold, m);
    lau_spin_unlock(&g_lock);
    size_t size = m->alloc_size & 0x007FFFFFFFFFFFFFULL;
    uint32_t flags = (m->alloc_size & (1ULL << 55)) ? LAU_TELEM_FLAG_SEALED : 0;
    report_event((uint64_t)m->actual_start, size, 1, flags);
}

void tsfi_registry_backfill_manifold(void) {
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (!fw) return;
    lau_spin_lock(&g_lock);
    LauMetadata *curr = g_head;
    while (curr) {
        manifold_insert(&fw->manifold, curr);
        curr = curr->next;
    }
    if (g_local_manifold) atomic_store(&g_local_manifold->count, 0);
    lau_spin_unlock(&g_lock);
}

void lau_registry_remove(LauMetadata *m) {
    if (!m) return;
    lau_spin_lock(&g_lock);
    LauMetadata *curr = g_head;
    while (curr) {
        if (curr->actual_start == m->actual_start) break;
        curr = curr->next;
    }
    if (curr) {
        if (curr->prev) safe_update_ptr(&curr->prev->next, curr->next, curr->prev->current_prot);
        else g_head = curr->next;
        if (curr->next) safe_update_ptr(&curr->next->prev, curr->prev, curr->next->current_prot);
    }
    if (g_local_manifold) manifold_remove(g_local_manifold, m->actual_start);
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw) manifold_remove(&fw->manifold, m->actual_start);
    lau_spin_unlock(&g_lock);
    size_t size = m->alloc_size & 0x007FFFFFFFFFFFFFULL;
    uint32_t flags = (m->alloc_size & (1ULL << 55)) ? LAU_TELEM_FLAG_SEALED : 0;
    report_event((uint64_t)m->actual_start, size, 3, flags);
}

LauMetadata* lau_registry_find(void *payload) {
    if (!payload) return NULL;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw) {
        LauMetadata *m = tsfi_registry_scan_zmm(&fw->manifold, payload);
        if (m) return m;
    }
    if (g_local_manifold) {
        LauMetadata *m = tsfi_registry_scan_zmm(g_local_manifold, payload);
        if (m) return m;
    }
    lau_spin_lock(&g_lock);
    LauMetadata *curr = g_head;
    while (curr) {
        size_t size = curr->alloc_size & 0x007FFFFFFFFFFFFFULL;
        uintptr_t start = (uintptr_t)curr->actual_start;
        uintptr_t p = (uintptr_t)payload;
        if (p >= start && p < start + size) {
            lau_spin_unlock(&g_lock);
            return curr;
        }
        curr = curr->next;
    }
    lau_spin_unlock(&g_lock);
    return NULL;
}

LauMetadata* lau_registry_find_locked(void *payload) {
    if (!payload) return NULL;
    LauMetadata *curr = g_head;
    while (curr) {
        if (curr->actual_start == payload) return curr;
        curr = curr->next;
    }
    return NULL;
}

void lau_registry_remove_locked(LauMetadata *m) {
    if (!m) return;
    if (m->prev) safe_update_ptr(&m->prev->next, m->next, m->prev->current_prot);
    if (m->next) safe_update_ptr(&m->next->prev, m->prev, m->next->current_prot);
    if (g_head == m) g_head = m->next;
}

uint32_t lau_registry_get_count(void) {
    LauMetadata *curr;
    uint32_t active = 0;
    const char *tracking = getenv("TSFI_LEAK_TRACKING_IN_PROGRESS");
    bool filter = (tracking && tracking[0] == '1');
    lau_spin_lock(&g_lock);
    curr = g_head;
    while (curr) {
        bool skip = false;
        if (filter) {
            if (curr->alloc_file && strcmp(curr->alloc_file, "PRE_START") == 0) skip = true;
        }
        if (!skip) active++;
        curr = curr->next;
    }
    lau_spin_unlock(&g_lock);
    return active;
}

void lau_telemetry_record_exec(const char *directive) {
    if (!g_telem) return;
    atomic_fetch_add(&g_telem->exec_steps, 1);
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint64_t now = (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
    atomic_store(&g_telem->exec_last_ts, now);
    if (directive) {
        char *dest = (char*)g_telem->last_directive_str;
        size_t i;
        for (i = 0; i < 63 && directive[i] != '\0'; i++) dest[i] = directive[i];
        dest[i] = '\0';
    }
    lau_telemetry_integrity_update(g_telem);
}

void lau_registry_lock(void) { lau_spin_lock(&g_lock); }
void lau_registry_unlock(void) { lau_spin_unlock(&g_lock); }
LauTelemetryState* lau_telemetry_get_state(void) { return g_telem; }

LauMetadata* tsfi_registry_scan_zmm(LauRegistryManifold *m, void *ptr) {
    if (!m || !ptr) return NULL;
    uint32_t count = atomic_load(&m->count);
    if (count == 0 || count > LAU_REGISTRY_CAPACITY) return NULL;
    uintptr_t target = (uintptr_t)ptr;
    __m512i v_target = _mm512_set1_epi64(target);
    for (uint32_t i = 0; i + 7 < count; i += 8) {
        __m512i v_index = _mm512_loadu_si512(&m->search_index[i]);
        __mmask8 mask = _mm512_cmpeq_epu64_mask(v_index, v_target);
        if (mask) { LauMetadata *res = &m->slots[i + __builtin_ctz(mask)]; if (res->actual_start) return res; }
    }
    for (uint32_t j = 0; j < count; j++) {
        uintptr_t start = m->search_index[j];
        size_t size = m->slots[j].alloc_size & 0x007FFFFFFFFFFFFFULL;
        if (start != 0 && target >= start && target < start + size) return &m->slots[j];
    }
    return NULL;
}

void lau_registry_teardown(void) {
    atomic_store(&g_teardown_in_progress, 1);
    if (g_local_manifold) {
        void *ptr = g_local_manifold;
        lau_spin_lock(&g_lock);
        LauMetadata *m = lau_registry_find_locked(ptr);
        if (m) lau_registry_remove_locked(m);
        lau_spin_unlock(&g_lock);
        
        g_local_manifold = NULL;
        munmap(ptr, sizeof(LauRegistryManifold));
        atomic_fetch_sub(&g_active_allocs, 1);
        atomic_fetch_sub(&g_active_bytes, sizeof(LauRegistryManifold));
    }
}

void* lau_registry_get_local_manifold(void) { return g_local_manifold; }
LauMetadata* lau_registry_get_head(void) { return g_head; }

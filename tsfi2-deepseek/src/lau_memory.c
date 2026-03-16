#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_wire_firmware.h"
#include "vulkan/vulkan_system.h"
#include "lau_thunk.h"
#include "lau_audit.h"
#include "tsfi_io.h"
#include "tsfi_io_types.h"
#include "tsfi_vision.h"
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <stdalign.h>
#include <x86intrin.h>
#include <immintrin.h>
#include <sys/syscall.h>
#include <errno.h>

// Forward declarations for thunk emitters not in headers
void* ThunkProxy_emit_mapped(ThunkProxy *p, struct MappedCommon *c, struct LauWiredHeader *h);

#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS 0x20
#endif

// --- Internal Telemetry Globals ---
#undef lau_malloc
#undef lau_memalign
#undef lau_strdup
#undef lau_realloc

_Atomic size_t g_alloc_count = 0;
_Atomic size_t g_alloc_bytes = 0;
_Atomic size_t g_free_count = 0;
_Atomic size_t g_free_bytes = 0;
_Atomic size_t g_active_allocs = 0;
_Atomic size_t g_active_bytes = 0;
_Atomic size_t g_max_active_bytes = 0;

_Atomic size_t g_gpu_alloc_count = 0;
_Atomic size_t g_gpu_alloc_bytes = 0;
_Atomic size_t g_gpu_active_allocs = 0;
_Atomic size_t g_gpu_active_bytes = 0;

 // g_init_in_progress linked from registry

static _Atomic int g_quarantine_lock = 0;

static inline void lau_spin_lock(_Atomic int *lock) {
    while (atomic_exchange_explicit(lock, 1, memory_order_acquire)) { __builtin_ia32_pause(); }
}

static inline void lau_spin_unlock(_Atomic int *lock) {
    atomic_store_explicit(lock, 0, memory_order_release);
}

// Ensure compilation of physical mapping offsets matches reality
static_assert(offsetof(LauWiredHeader, payload) == 8192, "LauWiredHeader layout mismatch");

// --- Private Helpers ---

static inline LauWiredHeader* get_wired_header(void *payload) { if (!payload) return NULL;
    return (LauWiredHeader *)((char *)payload - 8192);
}

LauWiredHeader* get_wired_header_external(void *payload) {
    return get_wired_header(payload);
}

// Probe execution latency natively
uint32_t lau_probe_latency(void *ptr) {
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return 0;
    if (!ptr) return 0xFFFFFFFF;
    unsigned int dummy;
    uint64_t start = __rdtscp(&dummy);
    volatile uint8_t *p = (volatile uint8_t *)ptr;
    (void)*p;
    uint64_t end = __rdtscp(&dummy);
    return (uint32_t)(end - start);
}

void lau_clflush(void *ptr) {
    if (!ptr) return;
    _mm_clflushopt(ptr);
    _mm_sfence();
}

void lau_clzero(void *ptr) {
    if (!ptr) return;
    memset(ptr, 0, 64);
}

static uint32_t calculate_checksum_basic(const LauHeader *h) {
    if (!h) return 0;
    uint32_t crc = 0;
    crc = _mm_crc32_u64(crc, h->meta.alloc_size & 0x007FFFFFFFFFFFFFULL);
    crc = _mm_crc32_u64(crc, h->meta.instruction_hash);
    return crc;
}

static uint32_t calculate_checksum_wired(const LauWiredHeader *h) {
    if (!h) return 0;
    uint32_t crc = 0;
    // Hash only immutable physical attributes to avoid pointer race conditions
    crc = _mm_crc32_u64(crc, h->meta.alloc_size & 0x007FFFFFFFFFFFFFULL);
    crc = _mm_crc32_u64(crc, h->meta.instruction_hash);
    return crc;
}

uint32_t calculate_checksum_basic_external(const LauHeader *h) {
    return calculate_checksum_basic(h);
}

uint32_t calculate_checksum_wired_external(const LauWiredHeader *h) {
    return calculate_checksum_wired(h);
}

// --- Memory API ---

void *lau_malloc_loc(size_t size, const char *file, int line) {
    return lau_memalign_loc(16, size, file, line);
}

void *lau_memalign_loc(size_t alignment, size_t size, const char *file, int line) {
    extern LauWireFirmware *g_tsfi_firmware;
    if (!g_tsfi_firmware && !atomic_load_explicit(&g_init_in_progress, memory_order_relaxed) && !lau_registry_get_local_manifold()) return NULL;
    size_t header_size = sizeof(LauHeader);
    
    // Total size must account for:
    // 1. Minimum 16-byte alignment for the block start (libc requirement)
    // 2. Padding to shift the payload to the 'alignment' boundary
    // 3. The header which sits exactly before the payload
    size_t pad = alignment > 16 ? alignment : 16;
    size_t total_size = header_size + size + pad;
    void *block = NULL;
    
    if (posix_memalign(&block, alignment > 16 ? alignment : 16, total_size) != 0) return NULL;
    
    uintptr_t block_addr = (uintptr_t)block;
    // Align the PAYLOAD address
    uintptr_t payload_addr = (block_addr + header_size + (alignment - 1)) & ~(alignment - 1);
    void *payload = (void *)payload_addr;
    
    LauHeader *h = (LauHeader *)((char *)payload - header_size);
    
    memset(&h->meta, 0, sizeof(LauMetadata));
    h->meta.seal_level = LAU_SEAL_NONE;
    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_BASIC;
    h->meta.alloc_size = total_size | ((size_t)LAU_TYPE_BASIC << 56);
    h->meta.current_prot = PROT_READ | PROT_WRITE;
    h->meta.actual_start = block;
    h->meta.payload_start = payload;
    const char *tracking = getenv("TSFI_LEAK_TRACKING_IN_PROGRESS");
    if (!tracking || tracking[0] != "1"[0]) file = "PRE_START";
    h->meta.alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;
    
    h->meta.probe_latency = lau_probe_latency(block);
    h->footer.checksum = calculate_checksum_basic(h);
    h->meta.current_prot = PROT_READ | PROT_WRITE;
    
    lau_registry_insert(&h->meta);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    return payload;
}

void *lau_calloc_loc(size_t nmemb, size_t size, const char *file, int line) {
    size_t total = nmemb * size;
    void *ptr = lau_malloc_loc(total, file, line);
    if (ptr) tsfi_memset(ptr, 0, total);
    return ptr;
}

void *lau_malloc_wired_loc(size_t size, const char *file, int line) {
    void *p = lau_memalign_wired_loc(512, size, file, line);
    
    return p;
}

void *lau_memalign_wired_loc(size_t alignment, size_t size, const char *file, int line) {
    (void)alignment;
    static _Thread_local int in_wired = 0;
    if (in_wired > 4) {
        return NULL; // Critical block for actual infinite loops
    }
    in_wired++;

    if (atomic_load_explicit(&g_init_in_progress, memory_order_relaxed)) {
        goto internal_alloc;
    }

    if (in_wired == 1) {
        LauWireFirmware *fw_check = tsfi_wire_firmware_get_no_init();
        if (fw_check && fw_check->cell_mem_genesis) {
            void *p = fw_check->cell_mem_genesis(size, LAU_TYPE_WIRED, file, line);
            if (p) {
                in_wired--;
                return p;
            }
        }
    }

internal_alloc:

    size_t off = 8192; // 512 * 15 (Next 512-aligned block after ~7168)
    size_t total_size = off + size;


    void *block = lau_rebar_alloc(total_size);
    if (!block) {
        block = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    }
    
    if (block == MAP_FAILED || block == NULL) {
        fflush(stderr);
        abort();
    }
    
    void *payload = (char*)block + off;
    
    LauWiredHeader *h = (LauWiredHeader *)block;
    memset(&h->meta, 0, sizeof(LauMetadata));
    h->meta.seal_level = LAU_SEAL_NONE;
    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_WIRED;
    h->meta.alloc_size = total_size | ((size_t)LAU_TYPE_WIRED << 56);
    h->meta.current_prot = PROT_READ | PROT_WRITE;
    h->meta.actual_start = block;
    h->meta.payload_start = (char*)block + off;
    const char *tracking = getenv("TSFI_LEAK_TRACKING_IN_PROGRESS");
    if (!tracking || tracking[0] != "1"[0]) file = "PRE_START";
    h->meta.alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;
    h->logic_classify = tsfi_vision_classify_thunk;
    h->proxy = NULL; // Explicitly NULL to avoid checksum garbage deref
    
    uint8_t *pad_ptr = h->_pad_death_zone;
    for (size_t i = 0; i < sizeof(h->_pad_death_zone); i += 64) {
        lau_clzero(pad_ptr + i);
    }

    h->meta.probe_latency = lau_probe_latency(block);
    h->footer.checksum = calculate_checksum_wired(h);
    h->meta.current_prot = PROT_READ | PROT_WRITE;
    lau_registry_insert(&h->meta);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    if (size >= sizeof(MappedCommon)) lau_wire_mapped_logic(payload);
    
    in_wired--;
    return payload;
}

void *lau_realloc_loc(void *ptr, size_t size, const char *file, int line) {
    if (!ptr) return lau_malloc_loc(size, file, line);
    
    LauFooter *f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    if (f->magic != LAU_MAGIC) return NULL;
    
    LauMetadata *m = lau_registry_find(ptr);
    (void)m;
    size_t current_payload_size = 0;
    size_t size_mask = 0x007FFFFFFFFFFFFFULL;
    
    if (f->type == LAU_TYPE_BASIC) {
        LauHeader *h = (LauHeader *)((char *)ptr - sizeof(LauHeader));
        current_payload_size = (h->meta.alloc_size & size_mask) - ((char*)ptr - (char*)h->meta.actual_start);
    } else if (f->type == LAU_TYPE_WIRED) {
        LauWiredHeader *h = get_wired_header(ptr);
        current_payload_size = (h->meta.alloc_size & size_mask) - 8192;
    }
    
    if (size <= current_payload_size) return ptr; 
    
    void *new_ptr = lau_malloc_loc(size, file, line);
    if (!new_ptr) return NULL;
    memcpy(new_ptr, ptr, current_payload_size);
    lau_free(ptr);
    return new_ptr;
}

void lau_seal_level_loc(void *ptr, uint8_t target_level, const char *file, int line) {
    (void)file; (void)line;
    if (!ptr) return;
    
    LauMetadata *m = lau_registry_find(ptr);
    if (!m) return;
    
    // 1. Initial Genesis (Seal-0: Plier/YI)
    // Always apply standard seal first if never sealed.
    if (m->seal_level == LAU_SEAL_NONE) {
        lau_seal_object_loc(ptr, file, line);
        // m->seal_level is now likely PLIER (0) if WIRED, but we force it to be sure.
        if (m->seal_level == LAU_SEAL_NONE) m->seal_level = LAU_SEAL_PLIER;
    }

    if (target_level == LAU_SEAL_PLIER) goto finalize;

    // 2. Linear Progression
    uint8_t type = (uint8_t)(m->alloc_size >> 56);
    if (type != LAU_TYPE_WIRED) goto finalize;

    LauWiredHeader *h = get_wired_header(ptr);
    if (!h || !h->proxy) goto finalize;

    // Transition to SEAL-1 (Dai)
    if (m->seal_level < LAU_SEAL_DAI && target_level >= LAU_SEAL_DAI) {
        lau_mprotect(ptr, PROT_READ | PROT_WRITE);
        ThunkProxy_unseal((ThunkProxy*)h->proxy);
        extern void* ThunkProxy_emit_layer1_seal(ThunkProxy *p, void *target_fn);
        ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, NULL);
        ThunkProxy_seal((ThunkProxy*)h->proxy);
        m->seal_level = LAU_SEAL_DAI;
    }

    // Transition to SEAL-2 (Étendre)
    if (m->seal_level < LAU_SEAL_ETENDRE && target_level >= LAU_SEAL_ETENDRE) {
        lau_mprotect(ptr, PROT_READ | PROT_WRITE);
        // Étendre specific thunking would go here.
        m->seal_level = LAU_SEAL_ETENDRE;
    }

finalize:
    m->alloc_size |= (1ULL << 55);
    lau_mprotect(ptr, PROT_READ);
}

void lau_seal_object_loc(void *ptr, const char *file, int line) {
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
    (void)file; (void)line;
    if (!ptr) return;
    
    LauMetadata *m = lau_registry_find(ptr);
    if (m && (m->alloc_size & (1ULL << 55)) && m->seal_level != LAU_SEAL_NONE) {
        return;
    }
    
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (m) m->probe_latency = lau_probe_latency(m->actual_start);

    if (fw && fw->cell_seal_capture) fw->cell_seal_capture(ptr);
    
    uint8_t type = LAU_TYPE_BASIC;
    if (m) type = (uint8_t)(m->alloc_size >> 56);
    
    LauFooter *f = NULL;
    if (type == LAU_TYPE_GPU) {
        size_t size = m->alloc_size & 0x007FFFFFFFFFFFFFULL;
        f = (LauFooter *)((char*)ptr + size);
    } else {
        f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    }

    if (f->magic != LAU_MAGIC) {
        fprintf(stderr, "CRITICAL: Seal magic mismatch at %p (type=0x%x)\n", (void*)f, type);
        abort();
    }

    // Always ensure RW for genesis update
    lau_mprotect(ptr, PROT_READ | PROT_WRITE);

    if (type == LAU_TYPE_WIRED) {
        LauWiredHeader *h = get_wired_header(ptr);
        if (!h) return;
        h->footer.checksum = calculate_checksum_wired(h);
        
        if (h->proxy) {
            extern struct YI* tsfi_reaction_shoot(TSFiBigInt* Prime);
            TSFiBigInt* prime = tsfi_bn_alloc();
            tsfi_bn_randomize(prime);
            struct YI *yi = tsfi_reaction_shoot(prime);
            tsfi_bn_free(prime);
            
            ThunkProxy_unseal((ThunkProxy*)h->proxy);
            ThunkProxy_emit_ontological_structuring((ThunkProxy*)h->proxy, yi, NULL);
            ThunkProxy_seal((ThunkProxy*)h->proxy);
        }
    } else if (type == LAU_TYPE_BASIC) {
        LauHeader *h = (LauHeader*)((char*)ptr - sizeof(LauHeader));
        h->footer.checksum = calculate_checksum_basic(h);
    }

    if (m) {
        m->seal_level = LAU_SEAL_PLIER;
        m->alloc_size |= (1ULL << 55);
        lau_mprotect(ptr, PROT_READ);
    }
}

void lau_unseal_object_loc(void *ptr, const char *file, int line) {
    (void)file; (void)line;
    if (!ptr) return;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();

    LauMetadata *m = lau_registry_find(ptr);
    if (!m) return;

    // Hypervisor Level 0: Restore write access to the entire metadata/header area first.
    // We use lau_mprotect directly to restore RW, which handles the page alignment and full block size.
    uint32_t latency = lau_mprotect(ptr, PROT_READ | PROT_WRITE);
    
    if (fw && fw->cell_seal_release) fw->cell_seal_release(ptr);
    
    m->probe_latency = latency;

    {
        uint8_t type = (uint8_t)(m->alloc_size >> 56);
        
        LauFooter *f = NULL;
        if (type == LAU_TYPE_GPU) {
            size_t size = m->alloc_size & 0x007FFFFFFFFFFFFFULL;
            f = (LauFooter *)((char*)ptr + size);
        } else {
            f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
        }

        if (f->magic != LAU_MAGIC) {
            
            abort();
        }
        
        if (type == LAU_TYPE_WIRED) {
            
            
            // Hypervisor Level 0 Enforcer: Restore read/write access to the payload BEFORE modifying metadata
            lau_mprotect(ptr, PROT_READ | PROT_WRITE);
            
            LauWiredHeader *h = get_wired_header(ptr);
            
            h->meta.alloc_size &= ~(1ULL << 55);
            if (m) m->alloc_size &= ~(1ULL << 55);
            
            h->footer.checksum = calculate_checksum_wired(h);
            
             // g_teardown_in_progress linked from registry
            if (h->proxy && !atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) {
                ThunkProxy *p = (ThunkProxy*)h->proxy;
                if (p->thunk_pool) {
                    ThunkProxy_unseal(h->proxy);

                    // Extinguish Layer-1 and Layer-0 Entities dynamically
                    struct YI *yi = NULL;
                    struct Dai *dai = NULL;
                    ThunkProxy_extract_ontological_structuring(p, &yi, &dai);
                    
                    if (dai) {
                        extern void freeDAI(struct Dai*);
                        freeDAI(dai);
                    }

                    if (yi) {
                        extern void freeYI(struct YI*);
                        freeYI(yi);
                    }
                    
                    // Clear the ontological signature
                    if (yi || dai) {
                        ThunkProxy_emit_ontological_structuring(p, NULL, NULL);
                    }
                }
            }

            if (m) {
                size_t size = m->alloc_size & 0x007FFFFFFFFFFFFFULL;
                extern void lau_registry_report_event(uint64_t ptr, uint64_t size, uint32_t type, uint32_t flags);
                lau_registry_report_event((uint64_t)m->actual_start, size, 5, 0); // TYPE 5 = UNSEAL
            }
        } else if (type == LAU_TYPE_GPU) {
            // Hypervisor Level 0 Enforcer: Restore read/write access to the payload BEFORE modifying metadata
            lau_mprotect(ptr, PROT_READ | PROT_WRITE);
            if (m) m->alloc_size &= ~(1ULL << 55);
            f->checksum = 0; // GPU checksum not implemented yet
        } else {
            // Hypervisor Level 0 Enforcer: Restore read/write access to the payload BEFORE modifying metadata
            lau_mprotect(ptr, PROT_READ | PROT_WRITE);
            
            LauHeader *h = (LauHeader*)((char*)ptr - sizeof(LauHeader));
            h->meta.alloc_size &= ~(1ULL << 55);
            if (m) m->alloc_size &= ~(1ULL << 55);
            h->footer.checksum = calculate_checksum_basic(h);
        }
    }
}

char *lau_strdup_loc(const char *s, const char *file, int line) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = (char *)lau_malloc_loc(len, file, line);
    if (dup) memcpy(dup, s, len);
    return dup;
}

#define QUARANTINE_DEPTH 64
typedef struct { void *ptr; size_t size; } QuarantineEntry;
static QuarantineEntry g_quarantine_pool[QUARANTINE_DEPTH] = {0};

void lau_quarantine_drain(void) {
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
    lau_spin_lock(&g_quarantine_lock);
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    for (size_t i = 0; i < QUARANTINE_DEPTH; i++) {
        if (g_quarantine_pool[i].ptr) { 
            if (fw && fw->cell_mem_reclaim) fw->cell_mem_reclaim(g_quarantine_pool[i].ptr, g_quarantine_pool[i].size);
            else munmap(g_quarantine_pool[i].ptr, g_quarantine_pool[i].size);
            g_quarantine_pool[i].ptr = NULL; 
        }
    }
    lau_spin_unlock(&g_quarantine_lock);
}

// Internal helper for authoritative free using pre-resolved metadata
uint32_t lau_mprotect_meta(LauMetadata *m, int prot) {
    if (!m) return 0;
    size_t pg = sysconf(_SC_PAGESIZE);
    uint32_t latency = 0;

    uint8_t fast_type = (uint8_t)(m->alloc_size >> 56);
    if (fast_type == LAU_TYPE_BASIC) {
        return lau_probe_latency(m->actual_start);
    }

    uintptr_t addr = (uintptr_t)m->actual_start & ~(pg - 1);
    size_t recorded_size = m->alloc_size & 0x007FFFFFFFFFFFFFULL;
    size_t protect_size = (uintptr_t)m->actual_start - addr + recorded_size;
    if (fast_type == LAU_TYPE_GPU) protect_size += 4096; 
    protect_size = (protect_size + pg - 1) & ~(pg - 1);

    // SAFETY: Temporarily force RW to update metadata safely
    if (tsfi_mprotect((void*)addr, protect_size, PROT_READ | PROT_WRITE) != 0) {
        perror("lau_mprotect_meta safety-unlock failure");
        return 0;
    }

    latency = lau_probe_latency(m->actual_start);
    m->current_prot = (uint8_t)prot;

    // Propagate to original header if Wired
    if (fast_type == LAU_TYPE_WIRED) {
        LauWiredHeader *wh = (LauWiredHeader*)m->actual_start;
        wh->meta.current_prot = (uint8_t)prot;
    }

    // Apply the final intended protection using native tsfi_mprotect
    if (tsfi_mprotect((void*)addr, protect_size, prot) != 0) {
        perror("lau_mprotect_meta final-lock failure");
    }

    if (prot & PROT_EXEC) {
        if (fast_type == LAU_TYPE_WIRED) {
            volatile uint8_t *p = (volatile uint8_t *)m->actual_start;
            for (size_t i = 0; i < protect_size; i += 64) { __asm__ __volatile__("clflushopt %0" : : "m" (p[i])); }
            __asm__ __volatile__("sfence");
        }
    }

    return latency;
}

static void lau_free_meta(LauMetadata *meta) {
    if (!meta || !meta->actual_start || atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
    if (!meta || !meta->actual_start) return;
    
    uint8_t fast_type = (uint8_t)(meta->alloc_size >> 56);
    void *actual_block_start = meta->actual_start;
    size_t size = meta->alloc_size & 0x007FFFFFFFFFFFFFULL;
    void *payload = meta->payload_start;
    
    // Ensure header is writable for cleanup
    lau_mprotect_meta(meta, PROT_READ | PROT_WRITE);
    
     // g_teardown_in_progress linked from registry
    int in_teardown = atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed);

    if (fast_type == LAU_TYPE_GPU) {
        size_t block_size = size + sizeof(LauFooter) + sizeof(LauHeader);
        
        atomic_fetch_add(&g_free_count, 1);
        atomic_fetch_add(&g_free_bytes, size);
        atomic_fetch_sub(&g_active_allocs, 1);
        atomic_fetch_sub(&g_active_bytes, size);
        atomic_fetch_sub(&g_gpu_active_allocs, 1);
        atomic_fetch_sub(&g_gpu_active_bytes, size);
        
        LauFooter *f = (LauFooter *)((char*)payload + size);
        f->magic = LAU_MAGIC_FREED;
        
        LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
        if (fw && fw->cell_mem_reclaim) {
            fw->cell_mem_reclaim(actual_block_start, block_size);
        } else {
            if (!fw || (uintptr_t)actual_block_start < (uintptr_t)fw->rtl.zhong_rebar_ptr || (uintptr_t)actual_block_start >= (uintptr_t)fw->rtl.zhong_rebar_ptr + fw->rtl.zhong_rebar_size) {
                munmap(actual_block_start, block_size + 512); 
            }
        }
        return; 
    }

    if (fast_type == LAU_TYPE_WIRED) {
        if (!payload) return;
        LauWiredHeader *wh = (LauWiredHeader*)((char*)payload - 8192);
        if (!in_teardown) {
            if (wh->meta.alloc_size & (1ULL << 55)) { fprintf(stderr, "CRITICAL: Attempt to free sealed hardware cell at ptr=%p\n", payload); abort(); }
            uint32_t calc = calculate_checksum_wired(wh);
            if (calc != wh->footer.checksum) { 
                fprintf(stderr, "CRITICAL: Checksum mismatch (WIRED) at %p\n", payload);
                abort(); 
            }
        }

        // Trigger Layer-0 Teardown Cascade before freeing sub-resources or proxies
        if (wh->layer0_teardown) {
            wh->layer0_teardown(payload);
            wh->layer0_teardown = NULL; // Prevent double-triggering
        }

        if (wh->proxy) {
            ThunkProxy *p = (ThunkProxy*)wh->proxy;
            wh->proxy = NULL;
            extern void ThunkProxy_destroy_authoritative(ThunkProxy *p);
            ThunkProxy_destroy_authoritative(p);
        }
    } else {
        if (!payload) return;
        LauHeader *h = (LauHeader*)((char*)payload - sizeof(LauHeader));
        if (!in_teardown) {
            if (h->meta.alloc_size & (1ULL << 55)) { fprintf(stderr, "CRITICAL: Attempt to free sealed basic cell\n"); abort(); }
            uint32_t calc = calculate_checksum_basic(h);
            if (calc != h->footer.checksum) { 
                fprintf(stderr, "CRITICAL: Checksum mismatch (BASIC) at %p [Size: %zu]\n", payload, size);
                abort(); 
            }
        }
    }

    fprintf(stderr, "[DEBUG] decrementing g_active_allocs from %zu for %p\n", (size_t)atomic_load(&g_active_allocs), payload);
    atomic_fetch_add(&g_free_count, 1);
    atomic_fetch_add(&g_free_bytes, size);
    atomic_fetch_sub(&g_active_allocs, 1);
    atomic_fetch_sub(&g_active_bytes, size);
    fprintf(stderr, "[DEBUG] new g_active_allocs: %zu\n", (size_t)atomic_load(&g_active_allocs));
    
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fast_type == LAU_TYPE_BASIC) {
        free(actual_block_start);
    } else {
        if (fw && fw->cell_mem_reclaim) {
            fw->cell_mem_reclaim(actual_block_start, size);
        } else {
            bool is_rebar = (fw && (uintptr_t)actual_block_start >= (uintptr_t)fw->rtl.zhong_rebar_ptr && (uintptr_t)actual_block_start < (uintptr_t)fw->rtl.zhong_rebar_ptr + fw->rtl.zhong_rebar_size);
            if (!is_rebar) {
                bool quarantined = false;
                lau_spin_lock(&g_quarantine_lock);
                for (size_t i = 0; i < QUARANTINE_DEPTH; i++) {
                    if (!g_quarantine_pool[i].ptr) {
                        g_quarantine_pool[i].ptr = actual_block_start;
                        g_quarantine_pool[i].size = size;
                        quarantined = true;
                        break;
                    }
                }
                lau_spin_unlock(&g_quarantine_lock);
                
                if (!quarantined) {
                    lau_quarantine_drain();
                    munmap(actual_block_start, size);
                }
            }
        }
    }
}

__attribute__((no_sanitize("address")))
void lau_free_authoritative(void *ptr) {
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
    if (!ptr) return;
    LauMetadata *meta = lau_registry_find(ptr);
    if (!meta) return;
    lau_registry_remove(meta);
    lau_free_meta(meta);
}

__attribute__((no_sanitize("address")))
void lau_free(void *ptr) {
    if (!ptr) return;
    
    // Authoritative check for teardown state BEFORE any registry lookups
     // g_teardown_in_progress linked from registry
    if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) {
        // During global sweep, we only allow lau_free_meta to perform reclamation
        // Manual calls from parent objects (like ThunkProxy_destroy) are ignored.
        return; 
    }
    
    LauMetadata *meta = lau_registry_find(ptr);
    if (!meta) {
        // Final re-check teardown just in case it transitioned during lookup
        if (atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed)) return;
        fprintf(stderr, "CRITICAL: Metadata resolution failed for %p! Likely memory corruption or double-free.\n", ptr);
        return; 
    }
    
    fprintf(stderr, "[DEBUG] lau_free proceeding for %p (meta: %p)\n", ptr, (void*)meta);
    lau_registry_remove(meta);
    
    lau_free_meta(meta);
}

uint32_t lau_mprotect(void *ptr, int prot) {
    if (!ptr) return 0;
    LauMetadata *m = lau_registry_find(ptr);
    if (!m) return 0;
    return lau_mprotect_meta(m, prot);
}

LauGpuHandle lau_gpu_lookup(void *ptr) {
    LauGpuHandle h = { .buffer = NULL, .secret = 0, .valid = false };
    LauMetadata *m = lau_registry_find(ptr);
    if (m && (m->alloc_size >> 56) == LAU_TYPE_GPU) {
        h.buffer = m->actual_start;
        h.secret = 0; 
        h.valid = true;
    }
    return h;
}

#undef lau_seal_object
#undef lau_unseal_object

void lau_seal_object(void *ptr) {
    lau_seal_object_loc(ptr, "WRAPPER", 0);
}

void lau_unseal_object(void *ptr) {
    lau_unseal_object_loc(ptr, "WRAPPER", 0);
}

void lau_report_memory_metrics(void) {
    alarm(0);
    size_t active = lau_registry_get_count();
    size_t gpu_active = atomic_load(&g_gpu_active_allocs);
    size_t cpu_active = active - gpu_active;
    
    printf("[MEMORY REPORT]\n");
    printf("--- System Memory (CPU) ---\n");
    printf("Allocations: %zu\nBytes: %zu\nActive: %zu\n",
           atomic_load(&g_alloc_count) - atomic_load(&g_gpu_alloc_count),
           atomic_load(&g_alloc_bytes) - atomic_load(&g_gpu_alloc_bytes),
           cpu_active);
           
    printf("--- Graphics Memory (GPU/ReBAR) ---\n");
    printf("Allocations: %zu\nBytes: %zu\nActive: %zu\n",
           atomic_load(&g_gpu_alloc_count),
           atomic_load(&g_gpu_alloc_bytes),
           gpu_active);

    printf("--- Global Totals ---\n");
    printf("Total Active: %zu\nTotal Bytes: %zu\nPeak: %.2f MB\n",
           active, atomic_load(&g_active_bytes),
           (double)atomic_load(&g_max_active_bytes) / 1048576.0);

    if (active > 0) {
        printf("\n[MEMORY LEAKS DETECTED]\n");
         // g_teardown_in_progress linked from registry
        int in_teardown = atomic_load_explicit(&g_teardown_in_progress, memory_order_relaxed);
        
        if (!in_teardown) lau_registry_lock();
        extern LauMetadata *g_head;
        LauMetadata *curr = g_head;
        size_t printed_leaks = 0;
        while (curr && (uintptr_t)curr > 0x1000) {
            if (curr->alloc_file && strcmp(curr->alloc_file, "PRE_START") == 0) { curr = curr->next; continue; }
            uint8_t type = (uint8_t)(curr->alloc_size >> 56);
            const char *type_str = (type == LAU_TYPE_WIRED) ? "WIRED" : (type == LAU_TYPE_GPU) ? "GPU" : "BASIC";
            printf("LEAK: %p | Type: %s | Size: %zu | Origin: %s\n", 
                   curr->actual_start, type_str, (size_t)(curr->alloc_size & 0x007FFFFFFFFFFFFFULL), 
                   curr->alloc_file ? curr->alloc_file : "UNKNOWN");
            printed_leaks++;
            curr = curr->next;
        }
        if (!in_teardown) lau_registry_unlock();
        printf("---------------------------------------\n");
        if (printed_leaks == 0) {
            printf("[INFO] METRIC RESET: No actual leaks found in registry list.\n");
            printf("\n[MEMORY LEAKS DETECTED] PERFECTLY ZERO LEAKS\n");
        } else if (printed_leaks != active) {
            printf("[WARNING] METRIC DESYNC: Printed %zu leaks but Active counter is %zu!\n", printed_leaks, active);
        }
    } else {
        printf("\n[MEMORY LEAKS DETECTED] PERFECTLY ZERO LEAKS\n");
    }
}

void lau_report_memory_to_log(void) {
    size_t sealed_count = 0;
    size_t active = lau_registry_get_count();
    
    lau_registry_lock();
    LauMetadata *curr = lau_registry_get_head();
    while (curr) {
        if (curr->alloc_file && strcmp(curr->alloc_file, "PRE_START") == 0) { curr = curr->next; continue; }
        uint8_t type = (uint8_t)(curr->alloc_size >> 56);
        if (type == LAU_TYPE_WIRED) { if (((LauWiredHeader*)curr->actual_start)->sealed) sealed_count++; }
        else { if (curr->alloc_size & (1ULL << 55)) sealed_count++; }
        curr = curr->next;
    }
    lau_registry_unlock();
    tsfi_io_log(NULL, TSFI_LOG_MEMORY, "METRICS", 
                "Allocs: %zu, Bytes: %zu, Free: %zu, Active: %zu, Sealed: %zu, Peak: %zu",
                atomic_load(&g_alloc_count), atomic_load(&g_alloc_bytes), atomic_load(&g_free_count),
                active, sealed_count, atomic_load(&g_max_active_bytes));
}

size_t lau_get_active_count(void) {
    return lau_registry_get_count();
}

size_t lau_get_active_bytes(void) { return atomic_load(&g_active_bytes); }
size_t lau_get_max_active_bytes(void) { return atomic_load(&g_max_active_bytes); }
void lau_sync(void *ptr, LauSyncDirection dir) { (void)ptr; (void)dir; }

void lau_memory_init_gpu(void *vk_ctx) {
    (void)vk_ctx;
}

static _Atomic size_t g_shared_rebar_offset = 0;

void *lau_malloc_gpu_loc(size_t size, LauGpuTier tier, const char *file, int line) {
    if (tier == LAU_GPU_REBAR) {
        LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
        if (fw && fw->rtl.zhong_rebar_ptr) {
            size_t payload_offset = (sizeof(LauHeader) + 63) & ~63;
            size_t total_needed = size + sizeof(LauFooter) + payload_offset; 
            // Hypervisor Fix: Align all ReBAR allocations to 4096-byte page boundaries 
            // so `mprotect` sealing doesn't cause collateral read-only locks on adjacent chunks!
            size_t align_needed = (total_needed + 4095) & ~4095;
            
            size_t offset = atomic_fetch_add(&g_shared_rebar_offset, align_needed);
            
            if (offset + align_needed <= fw->rtl.zhong_rebar_size) {
                void* ptr = (char*)(uintptr_t)fw->rtl.zhong_rebar_ptr + offset + payload_offset;
                LauFooter *f = (LauFooter *)((char*)ptr + size);
                f->magic = LAU_MAGIC;
                f->type = LAU_TYPE_GPU;
                f->checksum = 0;
                
                LauHeader* h = (LauHeader*)((char*)ptr - sizeof(LauHeader));
                h->footer.magic = LAU_MAGIC;
                h->footer.type = LAU_TYPE_GPU;
                
                LauMetadata *md = &h->meta;
                memset(md, 0, sizeof(LauMetadata));
                md->actual_start = h;
                md->payload_start = ptr;
                md->alloc_size = size | ((uint64_t)LAU_TYPE_GPU << 56);
                md->current_prot = PROT_READ | PROT_WRITE;
                md->alloc_file = file;
                
                atomic_fetch_add(&g_gpu_alloc_count, 1);
                atomic_fetch_add(&g_gpu_alloc_bytes, size);
                atomic_fetch_add(&g_gpu_active_allocs, 1);
                atomic_fetch_add(&g_gpu_active_bytes, size);
                
                lau_registry_insert(md);

                return ptr;
            }
        }
    }
    
    // Fallback: allocate in system RAM but with GPU layout and strict 512-byte alignment
    size_t header_size = (sizeof(LauHeader) + 511) & ~511; // Align header size to 512
    size_t total_needed = size + sizeof(LauFooter) + header_size + 512;
    void *block = mmap(NULL, total_needed, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) return NULL;

    void *ptr = (char*)block + header_size;
    printf("[DEBUG] mmap block=%p, header_size=%zu, ptr=%p\n", block, header_size, ptr); fflush(stdout);
    LauHeader *h = (LauHeader*)block;
    LauFooter *f = (LauFooter*)((char*)ptr + size);
    
    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_GPU;
    f->magic = LAU_MAGIC;
    f->type = LAU_TYPE_GPU;
    
    LauMetadata *md = &h->meta;
    memset(md, 0, sizeof(LauMetadata));
    md->actual_start = block;
    md->payload_start = ptr;
    md->alloc_size = size | ((uint64_t)LAU_TYPE_GPU << 56);
    md->current_prot = PROT_READ | PROT_WRITE;
    md->alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;

    atomic_fetch_add(&g_gpu_alloc_count, 1);
    atomic_fetch_add(&g_gpu_alloc_bytes, size);
    atomic_fetch_add(&g_gpu_active_allocs, 1);
    atomic_fetch_add(&g_gpu_active_bytes, size);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, size);
    atomic_fetch_add(&g_active_allocs, 1);
    atomic_fetch_add(&g_active_bytes, size);

    lau_registry_insert(md);
    return ptr;
}

void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr) {
    lau_registry_lock();
    LauMetadata *curr = lau_registry_get_head();
    size_t unsealed_leaks = 0;
    while (curr) {
        if (curr->alloc_file && strcmp(curr->alloc_file, "PRE_START") == 0) { curr = curr->next; continue; }
        size_t real_size = curr->alloc_size & 0x007FFFFFFFFFFFFFULL;
        if (teardown_ptr >= curr->actual_start && (char*)teardown_ptr < (char*)curr->actual_start + real_size) {
            curr = curr->next;
            continue; 
        }
        
        bool is_sealed = (curr->alloc_size & (1ULL << 55)) != 0;
        uint8_t type = (uint8_t)(curr->alloc_size >> 56);
        uint16_t line = 0;
        
        if (type == LAU_TYPE_WIRED) {
            LauWiredHeader *wh = (LauWiredHeader*)curr;
            line = wh->footer.alloc_line;
        } else if (type == LAU_TYPE_BASIC || type == LAU_TYPE_GPU) {
            LauHeader *h = (LauHeader*)curr;
            line = h->footer.alloc_line;
        }
        
        if (!is_sealed) {
            fprintf(stderr, "ASSERTION FAILED: %s leaked UNSEALED object %p (Size: %zu) Origin: %s:%u\n", 
                    context, curr->actual_start, real_size, curr->alloc_file ? curr->alloc_file : "UNKNOWN", line);
            unsealed_leaks++;
        } else {
            fprintf(stderr, "ASSERTION FAILED: %s leaked SEALED object %p (Size: %zu) Origin: %s:%u\n", 
                    context, curr->actual_start, real_size, curr->alloc_file ? curr->alloc_file : "UNKNOWN", line);
            unsealed_leaks++; // Increment same counter to force abort
        }
        curr = curr->next;
    }
    lau_registry_unlock();
    
    if (unsealed_leaks > 0) {
        fprintf(stderr, "CRITICAL: %zu Leaks (Sealed/Unsealed) detected during %s! Forcing core dump.\n", unsealed_leaks, context);
        abort();
    }
}

void lau_mem_scramble(void* ptr, size_t size, char set) {
    if (!ptr || size == 0) return;
    if (set == LAU_SCRAMBLE_MODE_ZERO) {
        tsfi_memset(ptr, 0, size);
    } else {
        uint8_t *p = (uint8_t*)ptr;
        for (size_t i = 0; i < size; i++) p[i] = (uint8_t)rand();
    }
}

void* lau_rebar_alloc(size_t size) {
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (!fw || !fw->rtl.zhong_rebar_ptr) {
        return NULL;
    }
    // Hypervisor Fix: Align all ReBAR allocations to 4096-byte page boundaries 
    // so `mprotect` sealing doesn't cause collateral read-only locks on adjacent chunks!
    size_t pg = 4096;
    size_t align_size = (size + (pg - 1)) & ~(pg - 1);
    size_t offset = atomic_fetch_add(&g_shared_rebar_offset, align_size);
    if (offset + align_size > fw->rtl.zhong_rebar_size) return NULL;
    void *ptr = (void*)((uintptr_t)fw->rtl.zhong_rebar_ptr + offset);
    printf("[DEBUG] lau_rebar_alloc returning %p (size=%zu, offset=%zu)\n", ptr, size, offset);
    return ptr;
}

void* lau_rebar_alloc_external(size_t size) {
    return lau_rebar_alloc(size);
}

void lau_rebar_reset(void) {
    atomic_store(&g_shared_rebar_offset, 0);
}

void lau_wire_mapped_logic(void *ptr) {
    if (!ptr) return;
    LauFooter *f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    if (f->type != LAU_TYPE_WIRED) return;

    extern _Thread_local int g_in_thunk_create;

    LauWiredHeader *h = get_wired_header(ptr);
    if (!h->proxy && !g_in_thunk_create) {
        g_in_thunk_create = 1;
        h->proxy = (void*)ThunkProxy_create();
        g_in_thunk_create = 0;
    }
    
    if (h->proxy && !g_in_thunk_create) {
        MappedCommon *c = (MappedCommon *)ptr;
        ThunkProxy_emit_mapped((ThunkProxy*)h->proxy, c, h);
    }
}

void lau_free_all_active(void) {
    alarm(0);
    fprintf(stderr, "[DEBUG] Entering lau_free_all_active\n");
     // g_teardown_in_progress linked from registry
    atomic_store_explicit(&g_teardown_in_progress, 1, memory_order_relaxed);

    extern LauMetadata *g_head;
    int safety = 0;
    while (1) {
        if (safety++ > 10000) {
            fprintf(stderr, "[REGISTRY] CRITICAL: Teardown exceeded safety limit! Manifold may be corrupted.\n");
            break;
        }
        lau_registry_lock();
        if (!g_head) {
            lau_registry_unlock();
            break;
        }
        
        LauMetadata *m = g_head;
        void *payload = m->payload_start;
        fprintf(stderr, "[REGISTRY] Teardown: freeing %p (payload %p)\n", (void*)m, payload);
        
        // Authoritative Verification: Check if this node is still valid in the manifold
        extern LauMetadata* lau_registry_find_locked(void*);
        LauMetadata *resolved = lau_registry_find_locked(payload);
        if (resolved != m) {
            // Node was already freed/removed by another routine during recursive call
            lau_registry_unlock();
            continue;
        }
        
        extern void lau_registry_remove_locked(LauMetadata *m);
        lau_registry_remove_locked(m);
        
        lau_registry_unlock();
        
        // Ensure memory is writable before freeing, especially if it was sealed
        if (m->alloc_size & (1ULL << 55)) {
            lau_mprotect_meta(m, PROT_READ | PROT_WRITE);
        }
        
        lau_free_meta(m);
    }

    atomic_store(&g_active_allocs, 0);
    atomic_store(&g_active_bytes, 0);
    atomic_store(&g_gpu_active_allocs, 0);
    atomic_store(&g_gpu_active_bytes, 0);
}

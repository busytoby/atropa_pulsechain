#include "lau_memory.h"
#include "lau_registry.h"
#include "tsfi_log.h"

// Undefine macros
#undef lau_malloc
#undef lau_malloc_wired
#undef lau_memalign
#undef lau_strdup
#undef lau_realloc

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/random.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>
#include <stdatomic.h>
#include <errno.h>
#include <pthread.h>

void abort(void);

static _Atomic size_t g_alloc_count = 0;
static _Atomic size_t g_alloc_bytes = 0;
static _Atomic size_t g_free_count = 0;
static _Atomic size_t g_free_bytes = 0;
static _Atomic size_t g_active_allocs = 0;
static _Atomic size_t g_active_bytes = 0;
static _Atomic size_t g_max_active_bytes = 0; // New: Peak usage tracking

// --- Helper: Checksum ---
static uint32_t calculate_checksum_basic(const LauHeader *h) {
    uint32_t sum = 0;
    size_t size = h->meta.alloc_size & 0x00FFFFFFFFFFFFFF;
    sum ^= (uint32_t)size;
    sum ^= (uint32_t)(uintptr_t)h->meta.actual_start;
    sum ^= (uint32_t)(uintptr_t)h->meta.alloc_file;
    sum ^= (uint32_t)h->footer.alloc_line;
    return (sum << 1) | (sum >> 31);
}

static uint32_t calculate_checksum_wired(const LauWiredHeader *h) {
    uint32_t sum = 0;
    size_t size = h->meta.alloc_size & 0x00FFFFFFFFFFFFFF;
    sum ^= (uint32_t)size;
    sum ^= (uint32_t)(uintptr_t)h->meta.actual_start;
    sum ^= (uint32_t)(uintptr_t)h->meta.alloc_file;
    
    if (h->sealed) {
        sum ^= (uint32_t)(uintptr_t)h->proxy;
        sum ^= (uint32_t)(uintptr_t)h->logic_epoch;
        sum ^= (uint32_t)(uintptr_t)h->logic_state;
        sum ^= (uint32_t)(uintptr_t)h->logic_directive;
        sum ^= (uint32_t)(uintptr_t)h->logic_scramble;
        sum ^= (uint32_t)(uintptr_t)h->logic_provenance;
    }
    
    return (sum << 1) | (sum >> 31);
}

// --- Scramble ---
void lau_mem_scramble(void* ptr, size_t size, char set) {
    if (!ptr || size == 0) return;
    if (set != LAU_SCRAMBLE_MODE_RANDOM) {
        memset(ptr, set, size);
        return;
    }
    
    uint64_t* dptr = (uint64_t*)ptr;
    size_t len = size / sizeof(uint64_t);
    size_t rem = size % sizeof(uint64_t);

    for (size_t i = 0; i < len; i++) {
        uint64_t r;
        while (getrandom(&r, sizeof(r), 0) < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[LAU_MEMORY] FATAL: Entropy failure!\n");
            abort();
        }
        dptr[i] = r;
    }

    if (rem > 0) {
        uint64_t r;
        while (getrandom(&r, sizeof(r), 0) < 0) {
            if (errno == EINTR) continue;
            fprintf(stderr, "[LAU_MEMORY] FATAL: Entropy failure!\n");
            abort();
        }
        uint8_t* bptr = (uint8_t*)(dptr + len);
        for (size_t i = 0; i < rem; i++) {
            bptr[i] = (uint8_t)(r >> (i * 8)); // Overwrite (XOR not needed for free'd mem)
        }
    }
}

// --- BASIC MALLOC ---
__attribute__((visibility("default"))) void *lau_malloc_loc(size_t size, const char *file, int line) {
    if (size > SIZE_MAX - sizeof(LauHeader)) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Integer overflow!\n");
        abort();
    }
    size_t total_size = sizeof(LauHeader) + size;
    void *block = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) return NULL;
    
    LauHeader *h = (LauHeader *)block;
    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_BASIC;
    h->meta.alloc_size = total_size | ((size_t)LAU_TYPE_BASIC << 56);
    h->meta.actual_start = block;
    h->meta.alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;
    
    h->footer.checksum = calculate_checksum_basic(h);
    
    lau_registry_insert(&h->meta);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    
    // Update Peak
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    return (void *)(h + 1);
}

// --- WIRED MALLOC ---
void *lau_malloc_wired_loc(size_t size, const char *file, int line) {
    if (size > SIZE_MAX - sizeof(LauWiredHeader)) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Integer overflow!\n");
        abort();
    }
    size_t total_size = sizeof(LauWiredHeader) + size;
    void *block = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) return NULL;
    
    LauWiredHeader *h = (LauWiredHeader *)block;
    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_WIRED;
    h->meta.alloc_size = total_size | ((size_t)LAU_TYPE_WIRED << 56);
    h->meta.actual_start = block;
    h->meta.alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;
    
    h->footer.checksum = calculate_checksum_wired(h);
    
    lau_registry_insert(&h->meta);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    
    // Update Peak
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    return (void *)(h + 1);
}

// --- MEMALIGN WIRED ---
void *lau_memalign_wired_loc(size_t alignment, size_t size, const char *file, int line) {
    size_t offset = sizeof(LauWiredHeader);
    
    if (offset % alignment != 0) {
        offset = ((offset / alignment) + 1) * alignment;
    }
    
    if (size > SIZE_MAX - offset) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Integer overflow!\n");
        abort();
    }
    
    size_t total_size = offset + size;

    if (alignment > 4096) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Alignment > 4096 not supported by internal mmap allocator\n");
        abort();
    }

    void *block = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) return NULL;
    
    void *payload = (char *)block + offset;
    LauWiredHeader *h = (LauWiredHeader *)((char *)payload - sizeof(LauWiredHeader));
    
    if ((void*)h < block) abort();

    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_WIRED;
    h->meta.alloc_size = total_size | ((size_t)LAU_TYPE_WIRED << 56);
    h->meta.actual_start = block;
    h->meta.alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;
    
    h->footer.checksum = calculate_checksum_wired(h);
    
    lau_registry_insert(&h->meta);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    
    // Update Peak
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    return payload;
}

// --- MEMALIGN (Basic Only) ---
void *lau_memalign_loc(size_t alignment, size_t size, const char *file, int line) {
    size_t offset = sizeof(LauHeader);
    if (offset % alignment != 0) {
        offset = ((offset / alignment) + 1) * alignment;
    }
    if (size > SIZE_MAX - offset) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Integer overflow!\n");
        abort();
    }
    
    size_t total_size = offset + size;

    if (alignment > 4096) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Alignment > 4096 not supported by internal mmap allocator\n");
        abort();
    }

    void *block = mmap(NULL, total_size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (block == MAP_FAILED) return NULL;
    
    void *payload = (char *)block + offset;
    LauHeader *h = (LauHeader *)((char *)payload - sizeof(LauHeader));
    
    if ((void*)h < block) abort();

    h->footer.magic = LAU_MAGIC;
    h->footer.type = LAU_TYPE_BASIC;
    h->meta.alloc_size = total_size | ((size_t)LAU_TYPE_BASIC << 56);
    h->meta.actual_start = block;
    h->meta.alloc_file = file;
    h->footer.alloc_line = (uint16_t)line;
    h->footer.checksum = calculate_checksum_basic(h);
    
    lau_registry_insert(&h->meta);
    
    atomic_fetch_add(&g_alloc_count, 1);
    atomic_fetch_add(&g_alloc_bytes, total_size);
    atomic_fetch_add(&g_active_allocs, 1);
    size_t new_active = atomic_fetch_add(&g_active_bytes, total_size) + total_size;
    
    // Update Peak
    size_t current_max = atomic_load(&g_max_active_bytes);
    while (new_active > current_max && !atomic_compare_exchange_weak(&g_max_active_bytes, &current_max, new_active));
    
    return payload;
}

void *lau_realloc_loc(void *ptr, size_t size, const char *file, int line) {
    if (!ptr) return lau_malloc_loc(size, file, line);
    if (size == 0) {
        lau_free(ptr);
        return NULL;
    }

    LauFooter *f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    size_t current_payload_size = 0;
    size_t size_mask = 0x00FFFFFFFFFFFFFF;

    if (f->type == LAU_TYPE_BASIC) {
        LauHeader *h = (LauHeader *)((char *)ptr - sizeof(LauHeader));
        current_payload_size = (h->meta.alloc_size & size_mask) - sizeof(LauHeader);
    } else if (f->type == LAU_TYPE_WIRED) {
        LauWiredHeader *h = (LauWiredHeader *)((char *)ptr - sizeof(LauWiredHeader));
        current_payload_size = (h->meta.alloc_size & size_mask) - sizeof(LauWiredHeader);
    } else {
        return NULL;
    }

    if (size <= current_payload_size) return ptr;

    void *new_ptr = lau_malloc_loc(size, file, line);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, current_payload_size);
    lau_free(ptr);
    return new_ptr;
}

void lau_seal_object(void *ptr) {
    if (!ptr) return;
    // Check Footer
    LauFooter *f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    
    if (f->magic != LAU_MAGIC) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Cannot seal invalid object!\n");
        abort();
    }
    
    if (f->type == LAU_TYPE_WIRED) {
        LauWiredHeader *h = (LauWiredHeader *)((char *)ptr - sizeof(LauWiredHeader));
        h->sealed = true;
        h->footer.checksum = calculate_checksum_wired(h);
    } else {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Cannot seal Basic object!\n");
        abort();
    }
}

__attribute__((visibility("default"))) char *lau_strdup_loc(const char *s, const char *file, int line) {
    if (!s) return NULL;
    size_t len = strlen(s) + 1;
    char *dup = (char *)lau_malloc_loc(len, file, line);
    if (dup) memcpy(dup, s, len);
    return dup;
}

// --- Quarantine (Delayed Free) ---
// Keeps recently freed pointers alive ("quarantined") to reliably detect double frees
// before the underlying system allocator reuses the address.
#define QUARANTINE_DEPTH 64

typedef struct {
    void *ptr;
    size_t size;
} QuarantineEntry;

static QuarantineEntry g_quarantine_pool[QUARANTINE_DEPTH] = {0};
static size_t g_quarantine_idx = 0;
static pthread_mutex_t g_quarantine_lock = PTHREAD_MUTEX_INITIALIZER;

__attribute__((visibility("default"))) void lau_free(void *ptr) {
    if (!ptr) return;
    
    // Footer is always -8 bytes from ptr
    LauFooter *f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    
    // Check Magic
    if (f->magic == (uint8_t)LAU_MAGIC_FREED) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Double free detected!\n");
        abort();
    }
    if (f->magic != (uint8_t)LAU_MAGIC) {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Heap corruption (Magic: 0x%x)!\n", f->magic);
        abort();
    }
    
    LauMetadata *meta = NULL;
    void *header_start = NULL;
    size_t header_size = 0;

    if (f->type == LAU_TYPE_BASIC) {
        LauHeader *h = (LauHeader *)((char *)ptr - sizeof(LauHeader));
        if (f->checksum != calculate_checksum_basic(h)) {
            fprintf(stderr, "[LAU_MEMORY] FATAL: Checksum Mismatch (Basic)!\n");
            abort();
        }
        meta = &h->meta;
        header_start = h;
        header_size = sizeof(LauHeader);
    } else if (f->type == LAU_TYPE_WIRED) {
        LauWiredHeader *h = (LauWiredHeader *)((char *)ptr - sizeof(LauWiredHeader));
        if (f->checksum != calculate_checksum_wired(h)) {
            fprintf(stderr, "[LAU_MEMORY] FATAL: Checksum Mismatch (Wired)!\n");
            abort();
        }
        
        if (h->resonance_as_status) {
            void *p = h->resonance_as_status;
            h->resonance_as_status = NULL;
            lau_free(p);
        }
        if (h->proxy) {
            ThunkProxy *p = h->proxy;
            h->proxy = NULL;
            if (p->thunk_pool) {
                mprotect(p->thunk_pool, p->pool_size, PROT_READ | PROT_WRITE);
                lau_free(p->thunk_pool);
            }
            lau_free(p);
        }
        
        meta = &h->meta;
        header_start = h;
        header_size = sizeof(LauWiredHeader);
    } else {
        fprintf(stderr, "[LAU_MEMORY] FATAL: Unknown Header Type! (Magic: 0x%x, Type: 0x%x)\n", f->magic, f->type);
        abort();
    }

    size_t packed_size = meta->alloc_size;
    size_t real_size = packed_size & 0x00FFFFFFFFFFFFFF;
    size_t header_offset = (char*)ptr - (char*)meta->actual_start;
    size_t payload_size = real_size - header_offset;

    lau_registry_remove(meta);

    atomic_fetch_add(&g_free_count, 1);
    atomic_fetch_add(&g_free_bytes, real_size);
    atomic_fetch_sub(&g_active_allocs, 1);
    atomic_fetch_sub(&g_active_bytes, real_size);

    lau_mem_scramble(ptr, payload_size, LAU_SCRAMBLE_MODE_RANDOM);
    
    // Mark as freed in the footer
    f->magic = (uint8_t)LAU_MAGIC_FREED;

    void *raw_block = meta->actual_start;
    // We only zero the Metadata part to keep the Footer (with FREED magic) intact
    memset(header_start, 0, header_size - sizeof(LauFooter));
    
    // Quarantine Logic: Delay actual free
    pthread_mutex_lock(&g_quarantine_lock);
    QuarantineEntry old_entry = g_quarantine_pool[g_quarantine_idx];
    
    g_quarantine_pool[g_quarantine_idx].ptr = raw_block;
    g_quarantine_pool[g_quarantine_idx].size = real_size; // Store the size!

    g_quarantine_idx = (g_quarantine_idx + 1) % QUARANTINE_DEPTH;
    pthread_mutex_unlock(&g_quarantine_lock);

    if (old_entry.ptr) {
        munmap(old_entry.ptr, old_entry.size);
    }
}

__attribute__((visibility("default"))) void lau_quarantine_drain(void) {
    pthread_mutex_lock(&g_quarantine_lock);
    for (size_t i = 0; i < QUARANTINE_DEPTH; i++) {
        if (g_quarantine_pool[i].ptr) {
            munmap(g_quarantine_pool[i].ptr, g_quarantine_pool[i].size);
            g_quarantine_pool[i].ptr = NULL;
            g_quarantine_pool[i].size = 0;
        }
    }
    pthread_mutex_unlock(&g_quarantine_lock);
}

void lau_report_memory_metrics(void) {

    tsfi_log(LOG_INFO, "[MEMORY REPORT]\n");

    tsfi_log(LOG_INFO, "Allocations: %zu\n", atomic_load(&g_alloc_count));

    tsfi_log(LOG_INFO, "Total Bytes Allocated: %zu\n", atomic_load(&g_alloc_bytes));

    tsfi_log(LOG_INFO, "Frees: %zu\n", atomic_load(&g_free_count));

    tsfi_log(LOG_INFO, "Total Bytes Freed: %zu\n", atomic_load(&g_free_bytes));

    tsfi_log(LOG_INFO, "Current Active Allocations: %zu\n", atomic_load(&g_active_allocs));

    tsfi_log(LOG_INFO, "Current Active Bytes: %zu\n", atomic_load(&g_active_bytes));

    tsfi_log(LOG_INFO, "Maximum Active Memory: %.2f MB\n", (double)atomic_load(&g_max_active_bytes) / 1048576.0); // New MB Report



    size_t active = atomic_load(&g_active_allocs);

    if (active > 0) {

        tsfi_log(LOG_DEBUG, "--- ACTIVE ALLOCATIONS (AUDIT) ---\n");

        lau_registry_lock();

        LauMetadata *curr = lau_registry_get_head();

        while (curr) {

            size_t raw_size = curr->alloc_size;

            size_t size = raw_size & 0x00FFFFFFFFFFFFFF;

            uint8_t type = (uint8_t)(raw_size >> 56);

            

            int line = 0;

            if (type == LAU_TYPE_BASIC) {

                LauHeader *h = (LauHeader*)curr;

                line = h->footer.alloc_line;

            } else if (type == LAU_TYPE_WIRED) {

                LauWiredHeader *h = (LauWiredHeader*)curr;

                line = h->footer.alloc_line;

            }

            

            tsfi_log(LOG_DEBUG, "[AUDIT] File: %s:%d | Size: %zu bytes\n", 

                   curr->alloc_file ? curr->alloc_file : "UNKNOWN", line, size);

            

            curr = curr->next;

        }

        lau_registry_unlock();

        tsfi_log(LOG_DEBUG, "----------------------------\n");

    }

}



void lau_report_memory_to_log(void) {
    FILE *f = fopen("lau_audit.log", "a");
    if (!f) return;

    time_t now;
    time(&now);
    char *date = ctime(&now);
    date[strcspn(date, "\n")] = 0;

    fprintf(f, "\n[%s] [MEMORY REPORT]\n", date);
    fprintf(f, "Allocations: %zu\n", atomic_load(&g_alloc_count));
    fprintf(f, "Total Bytes Allocated: %zu\n", atomic_load(&g_alloc_bytes));
    fprintf(f, "Frees: %zu\n", atomic_load(&g_free_count));
    fprintf(f, "Total Bytes Freed: %zu\n", atomic_load(&g_free_bytes));
    fprintf(f, "Current Active Allocations: %zu\n", atomic_load(&g_active_allocs));
    fprintf(f, "Current Active Bytes: %zu\n", atomic_load(&g_active_bytes));
    fprintf(f, "Maximum Active Memory: %.2f MB\n", (double)atomic_load(&g_max_active_bytes) / 1048576.0); // New MB Report

    size_t active = atomic_load(&g_active_allocs);
    if (active > 0) {
        fprintf(f, "--- ACTIVE ALLOCATIONS (AUDIT) ---\n");
        lau_registry_lock();
        LauMetadata *curr = lau_registry_get_head();
        while (curr) {
            size_t raw_size = curr->alloc_size;
            size_t size = raw_size & 0x00FFFFFFFFFFFFFF;
            uint8_t type = (uint8_t)(raw_size >> 56);
            
            int line = 0;
            if (type == LAU_TYPE_BASIC) {
                LauHeader *h = (LauHeader*)curr;
                line = h->footer.alloc_line;
            } else if (type == LAU_TYPE_WIRED) {
                LauWiredHeader *h = (LauWiredHeader*)curr;
                line = h->footer.alloc_line;
            }
            
            fprintf(f, "[AUDIT] File: %s:%d | Size: %zu bytes\n", 
                   curr->alloc_file ? curr->alloc_file : "UNKNOWN", 
                   line, 
                   size);
            curr = curr->next;
        }
        lau_registry_unlock();
        fprintf(f, "----------------------------\n");
    }
    fclose(f);
}

size_t lau_get_active_count(void) {
    return atomic_load(&g_active_allocs);
}

__attribute__((visibility("default"))) size_t lau_get_max_active_bytes(void) {
    return atomic_load(&g_max_active_bytes);
}

// Hausdorff Operator
__attribute__((visibility("default"))) bool lau_verify_hausdorff_property(void *ptr) {
    if (!ptr) return false;
    LauFooter *f = (LauFooter *)((char*)ptr - sizeof(LauFooter));
    
    if (f->magic != LAU_MAGIC) return false;
    
    if (f->type == LAU_TYPE_BASIC) {
        LauHeader *h = (LauHeader *)((char *)ptr - sizeof(LauHeader));
        return f->checksum == calculate_checksum_basic(h);
    } else if (f->type == LAU_TYPE_WIRED) {
        LauWiredHeader *h = (LauWiredHeader *)((char *)ptr - sizeof(LauWiredHeader));
        return f->checksum == calculate_checksum_wired(h);
    }
    return false;
}
#ifndef LAU_MEMORY_H
#define LAU_MEMORY_H

#include "tsfi_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#define LAU_MAGIC 0x42
#define LAU_MAGIC_FREED 0xDE

typedef enum {
    LAU_TYPE_BASIC = 0x10,
    LAU_TYPE_WIRED = 0x20,
    LAU_TYPE_GPU   = 0x30
} LauHeaderType;

typedef enum {
    LAU_GPU_NATIVE,
    LAU_GPU_REBAR,
    LAU_GPU_DEVICE_LOCAL
} LauGpuTier;

typedef enum {
    LAU_SYNC_CPU_WRITE_GPU_READ = 0, // Flush CPU WC -> GPU L2
    LAU_SYNC_GPU_WRITE_CPU_READ = 1, // GPU L2 -> Invalidate CPU Cache (if needed)
    LAU_SYNC_GPU_L1_FLUSH       = 2  // Force L1 -> L2 writeback
} LauSyncDirection;

// Packed Footer (8 bytes)
// Layout: [Magic(1)][Type(1)][Line(2)][Checksum(4)]
typedef struct {
    uint8_t magic;
    uint8_t type;
    uint16_t alloc_line;
    uint32_t checksum;
} LauFooter;

// Basic Header (48 bytes, 16-byte aligned)
typedef struct {
    LauMetadata meta;
    LauFooter footer;
} LauHeader;

static_assert(offsetof(LauHeader, meta) == 0, "LauHeader meta offset mismatch");

// Simulated Register Bank (Private to WGP)
typedef struct {
    uint32_t sgpr[1024]; // Scalar Registers (e.g. Hilbert Coefficients)
    float    vgpr[256]; // Vector Registers (Simulation of local store)
} LauRegisterBank;

// Occupancy Manifest: Real-time hardware pressure tracking
typedef struct {
    _Atomic int32_t wgp_active_waves[64]; // Track up to 64 WGPs
    _Atomic int32_t ace_pressure[8];      // Track up to 8 ACEs (Async Compute Engines)
    uint32_t last_update_ts;
    uint32_t pad[7];
} LauOccupancyManifest;

// Wired Header (Full Logic)
// Meta(40) + Fields + Footer(8) -> Must align to 16
struct ThunkProxy;
typedef struct LauWiredHeader {
    LauMetadata meta;
    
    // Register Bank (Large Alignment)
    LauRegisterBank regs __attribute__((aligned(64)));

    // Sub-system context mappings
    void *proxy;

    // Direct Mappings
    int version;
    int system_id;
    char *resonance_as_status; 
    int counter;
    bool sealed;
    bool ftw; 
    bool is_autonomous_excuse_active;
    double confidence_technical;
    double confidence_process;
    
    // Hardware Telemetry
    LauOccupancyManifest occupancy;

    // Execution Context Pointers (Thunk Callbacks)
    void (*logic_epoch)(int*);
    bool (*logic_state)(void*);
    void (*logic_directive)(int*, char*);
    void (*logic_scramble)(void*);
    void (*logic_provenance)(void*);
    void (*logic_hilbert)(void*, float, float, float*);
    void (*logic_hilbert_batch)(void*, const float*, float*, int);
    int  (*logic_classify)(void*);

    // Physical Padding to anchor topography at 8192
    uint8_t _pad_death_zone[1024] __attribute__((aligned(64)));
    void (*layer0_teardown)(void*);
    int schema_count;
    const ThunkSignature* schema;
    uint8_t _pad_align[1504]; // Shifted to reach 8192
    LauFooter footer;
    uint8_t payload[]; // Anchored at 8192
    } LauWiredHeader;


    static_assert(offsetof(LauWiredHeader, footer) == 8184, "LauWiredHeader footer offset mismatch");
    static_assert(offsetof(LauWiredHeader, payload) == 8192, "LauWiredHeader layout mismatch");

typedef LauWiredHeader LauSystemHeader;

// --- Reflection Binding Macro ---
#define TSFI_BIND_SCHEMA(obj_ptr, schema_array, count) do { \
    LauWiredHeader* _h = (LauWiredHeader*)((char*)(obj_ptr) - 8192); \
    _h->schema = (schema_array); \
    _h->schema_count = (count); \
} while(0)


// --- GPU VRAM Handles ---
typedef struct {
    void* buffer; // VkBuffer (cast to void* to avoid including vulkan.h here)
    size_t secret;
    bool valid;
} LauGpuHandle;

// --- Memory API ---
void lau_memory_init_gpu(void *vk_ctx);
LauGpuHandle lau_gpu_lookup(void *ptr);
void *lau_malloc_loc(size_t size, const char *file, int line);
void *lau_memalign_loc(size_t alignment, size_t size, const char *file, int line);
void *lau_calloc_loc(size_t nmemb, size_t size, const char *file, int line);
char *lau_strdup_loc(const char *s, const char *file, int line);
void *lau_realloc_loc(void *ptr, size_t size, const char *file, int line);
void *lau_malloc_wired_loc(size_t size, const char *file, int line);
void *lau_memalign_wired_loc(size_t alignment, size_t size, const char *file, int line);
void *lau_malloc_gpu_loc(size_t size, LauGpuTier tier, const char *file, int line);

#define lau_malloc(size) lau_malloc_loc(size, __FILE__, __LINE__)
#define lau_memalign(alignment, size) lau_memalign_loc(alignment, size, __FILE__, __LINE__)
#define lau_calloc(nmemb, size) lau_calloc_loc(nmemb, size, __FILE__, __LINE__)
#define lau_strdup(s) lau_strdup_loc(s, __FILE__, __LINE__)
#define lau_realloc(ptr, size) lau_realloc_loc(ptr, size, __FILE__, __LINE__)
#define lau_malloc_wired(size) lau_malloc_wired_loc(size, __FILE__, __LINE__)
#define lau_memalign_wired(alignment, size) lau_memalign_wired_loc(alignment, size, __FILE__, __LINE__)
#define lau_malloc_gpu(size, tier) lau_malloc_gpu_loc(size, tier, __FILE__, __LINE__)

void* tsfi_memset(void *s, int c, size_t n);
#define lau_memset tsfi_memset

void lau_free(void *ptr);
void lau_free_authoritative(void *ptr);
void lau_free_all_active(void);
uint32_t lau_mprotect(void *ptr, int prot); // New: Returns gathered latency
uint32_t lau_mprotect_meta(LauMetadata *m, int prot);
void lau_sync(void *ptr, LauSyncDirection dir); // New
void lau_quarantine_drain(void); // New
// Sealing & Integrity Matrix
#define LAU_SEAL_PLIER   0
#define LAU_SEAL_YI      0
#define LAU_SEAL_DAI     1
#define LAU_SEAL_ETENDRE 2

void lau_seal_object_loc(void *ptr, const char *file, int line);
void lau_seal_level_loc(void *ptr, uint8_t level, const char *file, int line);
void lau_unseal_object_loc(void *ptr, const char *file, int line);

#define lau_seal_object(ptr) lau_seal_object_loc(ptr, __FILE__, __LINE__)
#define lau_seal_level(ptr, lvl) lau_seal_level_loc(ptr, lvl, __FILE__, __LINE__)
#define lau_unseal_object(ptr) lau_unseal_object_loc(ptr, __FILE__, __LINE__)

void* lau_rebar_alloc(size_t size);
void* lau_rebar_alloc_external(size_t size);

void lau_report_memory_metrics(void);
void lau_report_memory_to_log(void); // New: Write to lau_audit.log
#define LAU_SCRAMBLE_MODE_ZERO   0
#define LAU_SCRAMBLE_MODE_RANDOM 1

void lau_mem_scramble(void* ptr, size_t size, char set);

size_t lau_get_active_count(void); // New: Accessor for tracking
size_t lau_get_active_bytes(void); // New: Accessor for tracking
size_t lau_get_max_active_bytes(void); // New: High water mark

// Physical Environment Emulation
uint32_t lau_probe_latency(void *ptr);
void lau_clflush(void *ptr);
void lau_clzero(void *ptr);

void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);

void lau_wire_mapped_logic(void *ptr);

#endif // LAU_MEMORY_H
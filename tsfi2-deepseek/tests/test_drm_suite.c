#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "lau_memory.h"
#include <fcntl.h>

// --- DRM VA MAPPING MOCK ---
#define AMDGPU_VA_OP_MAP 1
#define AMDGPU_VA_OP_UNMAP 2

struct drm_amdgpu_gem_va {
    uint32_t handle;
    uint32_t _pad;
    uint32_t operation;
    uint32_t flags;
    uint64_t va_address;
    uint64_t offset_in_bo;
    uint64_t map_size;
};

bool mock_drm_ioctl_amdgpu_gem_va(struct drm_amdgpu_gem_va *args) {
    if (args->handle == 0) return false;
    if (args->va_address == 0) return false;
    if (args->map_size == 0) return false;
    
    if (args->operation == AMDGPU_VA_OP_MAP) {
        return true;
    } else if (args->operation == AMDGPU_VA_OP_UNMAP) {
        return true;
    }
    return false;
}

void test_drm_bo_va_mapping() {
    printf("[TEST] Verifying DRM BO Virtual Address (VA) Mapping Boundary...\n");

    struct drm_amdgpu_gem_va va_args = {0};
    va_args.handle = 42; // Simulated BO handle
    va_args.operation = AMDGPU_VA_OP_MAP;
    va_args.flags = 0; // Read/Write
    va_args.va_address = 0x800000000000ULL; // BDA Pointer Space
    va_args.offset_in_bo = 0;
    va_args.map_size = 1024 * 1024; // 1MB

    bool success = mock_drm_ioctl_amdgpu_gem_va(&va_args);
    assert(success == true);
    printf("  [PASS] DRM GEM VA Map payload validated (BDA: %llx).\n", (unsigned long long)va_args.va_address);

    va_args.operation = AMDGPU_VA_OP_UNMAP;
    success = mock_drm_ioctl_amdgpu_gem_va(&va_args);
    assert(success == true);
    printf("  [PASS] DRM GEM VA Unmap payload validated.\n");
}

// --- DRM CTX MOCK ---
#define AMDGPU_CTX_OP_ALLOC_CTX 1
#define AMDGPU_CTX_OP_FREE_CTX  2
#define AMDGPU_CTX_OP_QUERY_STATE 3

#define AMDGPU_CTX_PRIORITY_HIGH 1

struct drm_amdgpu_ctx_in {
    uint32_t op;
    uint32_t flags;
    uint32_t ctx_id;
    int32_t priority;
};

struct drm_amdgpu_ctx_out {
    union {
        uint32_t alloc_ctx_id;
        struct {
            uint32_t hangs;
            uint32_t reset_status;
        } state;
    };
};

union drm_amdgpu_ctx {
    struct drm_amdgpu_ctx_in in;
    struct drm_amdgpu_ctx_out out;
};

bool mock_drm_ioctl_amdgpu_ctx(union drm_amdgpu_ctx *args) {
    if (args->in.op == AMDGPU_CTX_OP_ALLOC_CTX) {
        if (args->in.priority == AMDGPU_CTX_PRIORITY_HIGH) {
            args->out.alloc_ctx_id = 99; // High Priority Context
            return true;
        }
        args->out.alloc_ctx_id = 100;
        return true;
    } else if (args->in.op == AMDGPU_CTX_OP_FREE_CTX) {
        if (args->in.ctx_id == 0) return false;
        return true;
    } else if (args->in.op == AMDGPU_CTX_OP_QUERY_STATE) {
        if (args->in.ctx_id == 0) return false;
        args->out.state.hangs = 0;
        args->out.state.reset_status = 0; // 0 = AMDGPU_CTX_NO_RESET
        return true;
    }
    return false;
}

void test_drm_amdgpu_ctx() {
    printf("[TEST] Verifying AMDGPU Context Creation Boundary...\n");

    union drm_amdgpu_ctx args = {0};
    args.in.op = AMDGPU_CTX_OP_ALLOC_CTX;
    args.in.priority = AMDGPU_CTX_PRIORITY_HIGH;

    bool success = mock_drm_ioctl_amdgpu_ctx(&args);
    assert(success == true);
    assert(args.out.alloc_ctx_id == 99);
    printf("  [PASS] DRM Context allocated with HIGH PRIORITY (ID: %u).\n", args.out.alloc_ctx_id);

    args.in.op = AMDGPU_CTX_OP_QUERY_STATE;
    args.in.ctx_id = 99;
    success = mock_drm_ioctl_amdgpu_ctx(&args);
    assert(success == true);
    assert(args.out.state.hangs == 0);
    assert(args.out.state.reset_status == 0);
    printf("  [PASS] DRM Context queried successfully (0 Hangs, 0 Resets).\n");

    args.in.op = AMDGPU_CTX_OP_FREE_CTX;
    args.in.ctx_id = 99;
    success = mock_drm_ioctl_amdgpu_ctx(&args);
    assert(success == true);
    printf("  [PASS] DRM Context freed (ID: %u).\n", args.in.ctx_id);
}

// --- DRM BO ALLOCATION MOCK ---
#define AMDGPU_GEM_DOMAIN_VRAM 0x2
#define AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED (1 << 0)

struct drm_amdgpu_gem_create_in {
    uint64_t bo_size;
    uint64_t alignment;
    uint64_t domains;
    uint64_t domain_flags;
};

struct drm_amdgpu_gem_create_out {
    uint32_t handle;
};

union drm_amdgpu_gem_create {
    struct drm_amdgpu_gem_create_in in;
    struct drm_amdgpu_gem_create_out out;
};

struct drm_amdgpu_gem_mmap_in {
    uint32_t handle;
};

struct drm_amdgpu_gem_mmap_out {
    uint64_t addr_ptr;
};

union drm_amdgpu_gem_mmap {
    struct drm_amdgpu_gem_mmap_in in;
    struct drm_amdgpu_gem_mmap_out out;
};

bool mock_drm_ioctl_amdgpu_gem_create(union drm_amdgpu_gem_create *args) {
    if (args->in.bo_size == 0) return false;
    if ((args->in.domains & AMDGPU_GEM_DOMAIN_VRAM) == 0) return false;
    args->out.handle = 42;
    return true;
}

bool mock_drm_ioctl_amdgpu_gem_mmap(union drm_amdgpu_gem_mmap *args) {
    if (args->in.handle == 0) return false;
    args->out.addr_ptr = 0x100000000ULL;
    return true;
}

void test_drm_bo_allocation() {
    printf("[TEST] Verifying DRM BO Allocation Boundary...\n");

    union drm_amdgpu_gem_create create_args = {0};
    create_args.in.bo_size = 1024 * 1024; // 1MB
    create_args.in.alignment = 4096;
    create_args.in.domains = AMDGPU_GEM_DOMAIN_VRAM;
    create_args.in.domain_flags = AMDGPU_GEM_CREATE_CPU_ACCESS_REQUIRED;

    bool success = mock_drm_ioctl_amdgpu_gem_create(&create_args);
    assert(success == true);
    assert(create_args.out.handle == 42);
    printf("  [PASS] DRM GEM Create payload validated. Handle: %u\n", create_args.out.handle);

    union drm_amdgpu_gem_mmap mmap_args = {0};
    mmap_args.in.handle = create_args.out.handle;

    success = mock_drm_ioctl_amdgpu_gem_mmap(&mmap_args);
    assert(success == true);
    assert(mmap_args.out.addr_ptr == 0x100000000ULL);
    printf("  [PASS] DRM GEM Mmap payload validated. Mmap Offset: %llx\n", (unsigned long long)mmap_args.out.addr_ptr);
}

// --- DRM COMMAND SUBMISSION MOCK ---
struct drm_amdgpu_cs_chunk {
    uint32_t chunk_id;
    uint32_t length_dw;
    uint64_t chunk_data;
};

struct drm_amdgpu_cs {
    uint64_t ctx_id;
    uint32_t bo_list_handle;
    uint32_t num_chunks;
    uint64_t chunks;
};

#define AMDGPU_CHUNK_ID_IB 0x01

bool mock_drm_ioctl_amdgpu_cs(struct drm_amdgpu_cs *cs) {
    if (cs == NULL) return false;
    if (cs->num_chunks == 0) return false;
    
    struct drm_amdgpu_cs_chunk *chunks = (struct drm_amdgpu_cs_chunk *)(uintptr_t)cs->chunks;
    if (chunks[0].chunk_id != AMDGPU_CHUNK_ID_IB) return false;
    if (chunks[0].length_dw == 0) return false;
    if (chunks[0].chunk_data == 0) return false;
    return true;
}

void test_drm_command_submission() {
    printf("[TEST] Verifying DRM Command Submission Boundary...\n");

    uint32_t* mock_ib = (uint32_t*)lau_malloc_wired(16 * sizeof(uint32_t));
    assert(mock_ib != NULL);
    mock_ib[0] = 0xC0031500; // PM4 Dispatch
    
    struct drm_amdgpu_cs_chunk* ib_chunk = (struct drm_amdgpu_cs_chunk*)lau_malloc_wired(sizeof(struct drm_amdgpu_cs_chunk));
    assert(ib_chunk != NULL);
    ib_chunk->chunk_id = AMDGPU_CHUNK_ID_IB;
    ib_chunk->length_dw = 16;
    ib_chunk->chunk_data = (uint64_t)(uintptr_t)mock_ib;

    struct drm_amdgpu_cs cs_payload;
    cs_payload.ctx_id = 1;
    cs_payload.bo_list_handle = 2;
    cs_payload.num_chunks = 1;
    cs_payload.chunks = (uint64_t)(uintptr_t)ib_chunk;

    bool success = mock_drm_ioctl_amdgpu_cs(&cs_payload);
    assert(success == true);

    printf("  [PASS] DRM amdgpu_cs payload structure validated.\n");

    lau_free(ib_chunk);
    lau_free(mock_ib);
}

// --- DRM DMABUF EXPORT/IMPORT MOCK ---
struct drm_prime_handle {
    uint32_t handle;
    uint32_t flags;
    int32_t  fd;
};

#define DRM_RDWR O_RDWR

bool mock_drm_ioctl_prime_handle_to_fd(struct drm_prime_handle *args) {
    if (args->handle == 0) return false;
    args->fd = 42; 
    return true;
}

bool mock_drm_ioctl_prime_fd_to_handle(struct drm_prime_handle *args) {
    if (args->fd <= 0) return false;
    args->handle = 99; 
    return true;
}

void test_drm_dmabuf_export_import() {
    printf("[TEST] Verifying Direct DRM DMABUF Export/Import Boundary...\n");

    struct drm_prime_handle export_args = {0};
    export_args.handle = 7; 
    export_args.flags = DRM_RDWR;
    bool success = mock_drm_ioctl_prime_handle_to_fd(&export_args);
    assert(success == true);
    printf("  [PASS] DRM PRIME Export payload validated. DMABUF FD: %d\n", export_args.fd);

    struct drm_prime_handle import_args = {0};
    import_args.fd = export_args.fd; 
    success = mock_drm_ioctl_prime_fd_to_handle(&import_args);
    assert(success == true);
    printf("  [PASS] DRM PRIME Import payload validated. New BO Handle: %u\n", import_args.handle);
}

// --- DRM SYNCOBJ (TIMELINE SEMAPHORE) MOCK ---
struct drm_amdgpu_syncobj_create {
    uint32_t handle;
    uint32_t flags;
};

struct drm_amdgpu_syncobj_timeline_wait {
    uint64_t handles;
    uint64_t points;
    uint64_t timeout_nsec;
    uint32_t count_handles;
    uint32_t flags;
    uint32_t first_signaled;
    uint32_t pad;
};

#define DRM_AMDGPU_SYNCOBJ_WAIT_FLAGS_WAIT_ALL (1 << 0)
#define DRM_AMDGPU_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT (1 << 1)

bool mock_drm_ioctl_amdgpu_syncobj_create(struct drm_amdgpu_syncobj_create *args) {
    if (args == NULL) return false;
    args->handle = 77;
    return true;
}

bool mock_drm_ioctl_amdgpu_syncobj_timeline_wait(struct drm_amdgpu_syncobj_timeline_wait *args) {
    if (args->count_handles == 0) return false;
    if (args->points == 0) return false;
    args->first_signaled = 0;
    return true;
}

void test_amdgpu_syncobj() {
    printf("[TEST] Verifying AMDGPU SyncObj (Timeline Semaphore) Boundary...\n");
    struct drm_amdgpu_syncobj_create create_args = {0};
    assert(mock_drm_ioctl_amdgpu_syncobj_create(&create_args) == true);
    uint32_t syncobj_handle = create_args.handle;
    uint64_t timeline_point = 42;
    struct drm_amdgpu_syncobj_timeline_wait wait_args = {0};
    wait_args.handles = (uint64_t)(uintptr_t)&syncobj_handle;
    wait_args.points = (uint64_t)(uintptr_t)&timeline_point;
    wait_args.timeout_nsec = 1000000000ULL; wait_args.count_handles = 1; wait_args.flags = DRM_AMDGPU_SYNCOBJ_WAIT_FLAGS_WAIT_ALL;
    assert(mock_drm_ioctl_amdgpu_syncobj_timeline_wait(&wait_args) == true);
    printf("  [PASS] DRM Timeline SyncObj wait payload validated (Point: %llu).\n", (unsigned long long)timeline_point);
}

int main() {
    alarm(5);
    printf("=== TSFi Unified DRM Subsystem Verification ===\n");
    test_drm_bo_va_mapping();
    test_drm_amdgpu_ctx();
    test_drm_bo_allocation();
    test_drm_command_submission();
    test_drm_dmabuf_export_import();
    test_amdgpu_syncobj();
    
    printf("\nALL DRM BOUNDARY TESTS PASSED\n");

    extern void tsfi_dl_thunks_teardown(void);
    tsfi_dl_thunks_teardown();
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}

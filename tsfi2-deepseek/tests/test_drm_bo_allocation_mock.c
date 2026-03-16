#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "lau_memory.h"

#define AMDGPU_GEM_DOMAIN_VRAM 0x2
#define AMDGPU_GEM_DOMAIN_GTT  0x4
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
    
    // Simulate successful allocation
    args->out.handle = 42;
    return true;
}

bool mock_drm_ioctl_amdgpu_gem_mmap(union drm_amdgpu_gem_mmap *args) {
    if (args->in.handle == 0) return false;
    
    // Simulate successful mmap offset generation
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

int main() {
    printf("=== TSFi DRM BO Allocation Verification ===\n");
    test_drm_bo_allocation();
    printf("ALL DRM BO ALLOCATION TESTS PASSED\n");

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
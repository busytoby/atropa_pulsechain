#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "lau_memory.h"

#define AMDGPU_CTX_OP_ALLOC_CTX 1
#define AMDGPU_CTX_OP_FREE_CTX  2
#define AMDGPU_CTX_OP_QUERY_STATE 3
#define AMDGPU_CTX_OP_QUERY_STATE2 4

#define AMDGPU_CTX_PRIORITY_NORMAL 0
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

int main() {
    printf("=== TSFi AMDGPU Context Verification ===\n");
    test_drm_amdgpu_ctx();
    printf("ALL AMDGPU CONTEXT TESTS PASSED\n");

    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

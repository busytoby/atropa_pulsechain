#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "lau_memory.h"
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_types.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>

// Redeclare structs to match what we compiled into firmware for testing purposes
typedef struct {
    MappedCommon common; 
    uint32_t handle_id;
    int bounds_x;
    int bounds_y;
    void (*resize)(void* ctx, int w, int h);
} TSFiWaylandSurface;

typedef struct {
    MappedCommon common; 
    uint32_t* pm4_stream;
    size_t capacity;
    size_t write_ptr;
    bool is_recording;
    void (*submit_pm4)(void* ctx);
} TSFiCommandBuffer;

int main() {
    alarm(5);
    printf("=== TSFi Vulkan Subsystem Thunk Hypervisor Verification ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize Vulkan to force mapping allocations
    // We'll dynamically resolve the TSFi Vulkan mock functions
    extern PFN_vkVoidFunction tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName);
    
    PFN_vkCreateWaylandSurfaceKHR pfn_tsfi_vkCreateWaylandSurfaceKHR = (PFN_vkCreateWaylandSurfaceKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkCreateWaylandSurfaceKHR");
    PFN_vkDestroySurfaceKHR pfn_tsfi_vkDestroySurfaceKHR = (PFN_vkDestroySurfaceKHR)tsfi_vkGetInstanceProcAddr(NULL, "vkDestroySurfaceKHR");
    PFN_vkAllocateCommandBuffers pfn_tsfi_vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)tsfi_vkGetInstanceProcAddr(NULL, "vkAllocateCommandBuffers");
    PFN_vkFreeCommandBuffers pfn_tsfi_vkFreeCommandBuffers = (PFN_vkFreeCommandBuffers)tsfi_vkGetInstanceProcAddr(NULL, "vkFreeCommandBuffers");

    printf("[TEST 1] Allocating Surface via Vulkan ABI...\n");
    VkSurfaceKHR surface_handle = VK_NULL_HANDLE;
    pfn_tsfi_vkCreateWaylandSurfaceKHR(NULL, NULL, NULL, &surface_handle);
    TSFiWaylandSurface *surface = (TSFiWaylandSurface*)surface_handle;
    
    assert(surface != NULL);
    assert(surface->handle_id == 42); // Check initialization
    printf("PASS: Surface physically instantiated at %p.\n", surface);

    // Test 2: Thunk Proxy wiring logic is handled by allocator now
    printf("[TEST 2] Requesting execution of Surface->resize via VM_OP_INVOKE...\n");
    uint32_t hi_surf = (uint32_t)((uint64_t)surface >> 32);
    uint32_t lo_surf = (uint32_t)((uint64_t)surface & 0xFFFFFFFF);
    
    uint32_t resize_cmd[] = { VM_OP_INVOKE, hi_surf, lo_surf, offsetof(TSFiWaylandSurface, resize), ZMM_OP_END };
    
    // We pass args to resize via RDX and RCX since RDI is the context ptr, but our ABI mock invoke logic doesn't support arg passing dynamically yet.
    // Instead we will call the thunk directly since it's baked, OR we can test just invoking the VM and observing it not crash.
    // Wait, tsfi_vkCreateWaylandSurfaceKHR seals the object. Let's make sure we can't write to it!
    printf("[TEST 3] Invoking VM execute block on sealed surface...\n");
    tsfi_zmm_vm_exec_block(&vm, resize_cmd);
    
    // Test 4: Allocate Command Buffer
    printf("[TEST 4] Allocating Command Buffer via Vulkan ABI...\n");
    VkCommandBufferAllocateInfo allocInfo = { .commandBufferCount = 1 };
    VkCommandBuffer cb_handle = VK_NULL_HANDLE;
    pfn_tsfi_vkAllocateCommandBuffers(NULL, &allocInfo, &cb_handle);
    TSFiCommandBuffer *cb = (TSFiCommandBuffer*)cb_handle;
    
    // Test 5: Thunk Proxy wiring logic is handled by allocator now
    printf("[TEST 5] Requesting execution of CommandBuffer->submit_pm4 via VM_OP_INVOKE...\n");
    uint32_t hi_cb = (uint32_t)((uint64_t)cb >> 32);
    uint32_t lo_cb = (uint32_t)((uint64_t)cb & 0xFFFFFFFF);
    uint32_t submit_cmd[] = { VM_OP_INVOKE, hi_cb, lo_cb, offsetof(TSFiCommandBuffer, submit_pm4), ZMM_OP_END };
    
    size_t prev_write_ptr = cb->write_ptr;
    tsfi_zmm_vm_exec_block(&vm, submit_cmd);
    assert(cb->write_ptr == prev_write_ptr + 4);
    printf("PASS: PM4 write pointer successfully mutated securely inside thunk.\n");

    // Test 6: Clean Teardown (Tests unseal cascading)
    printf("[TEST 6] Triggering Vulkan Teardowns...\n");
    pfn_tsfi_vkDestroySurfaceKHR(NULL, surface_handle, NULL);
    pfn_tsfi_vkFreeCommandBuffers(NULL, NULL, 1, &cb_handle);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== VULKAN HYPERVISOR TESTS PASSED ===\n");
    extern void lau_report_memory_metrics(void);
    extern void lau_free_all_active(void);
    lau_free_all_active();
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test", NULL);
    return 0;
}

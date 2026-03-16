#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_manifest.h"
#include "lau_memory.h"
#include "vulkan_main.h"

// Test: Vulkan MCP ReBAR Integration (The Zhong Path)
// Verifies that the Tool Manifold resides in Device-Local Host-Visible memory.

typedef struct {
    VulkanContext* vk;
    TSFiHelmholtzManifold* rebar_manifold;
} VulkanMcpContext;

extern VulkanMcpContext* tsfi_vulkan_mcp_init();
extern void tsfi_vulkan_mcp_populate_tools(VulkanMcpContext* ctx);
extern void tsfi_vulkan_mcp_destroy(VulkanMcpContext* ctx);

int main() {
    printf("[TEST] Vulkan MCP ReBAR Verification...\n");

    VulkanMcpContext* ctx = tsfi_vulkan_mcp_init();
    if (!ctx) return 1;

    tsfi_vulkan_mcp_populate_tools(ctx);

    // Verify Address resides in ReBAR
    void* manifold_addr = ctx->rebar_manifold->lattice;
    LauGpuHandle h = lau_gpu_lookup(manifold_addr);

    if (h.valid) {
        printf("  [PASS] Manifold addr %p mapped to GPU Buffer %p (Secret: %zu)\n", 
               manifold_addr, h.buffer, h.offset);
    } else {
        printf("  [FAIL] Manifold addr %p is NOT in ReBAR pool.\n", manifold_addr);
        tsfi_vulkan_mcp_destroy(ctx);
        return 1;
    }

    // Verify Tool 0 (wave512.run) Presence in ReBAR
    uint32_t secret = ctx->rebar_manifold->lattice[0].secret_provenance & TSFI_SECRET_MASK;
    if (secret == 0xA01) {
        printf("  [PASS] Tool 0 Secret 0xA01 verified in ReBAR lattice.\n");
    } else {
        printf("  [FAIL] Tool 0 Secret mismatch (found 0x%X).\n", secret);
        tsfi_vulkan_mcp_destroy(ctx);
        return 1;
    }

    tsfi_vulkan_mcp_destroy(ctx);
    printf("[TEST] Vulkan MCP ReBAR Complete.\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
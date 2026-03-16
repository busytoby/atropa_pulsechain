#include "tsfi_manifest.h"
#include "tsfi_zmm_vm.h"
#include "lau_memory.h"
#include "vulkan_main.h"
#include "vulkan_render.h"
#include "vulkan_init.h"
#include "tsfi_wiring.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- LLM-MCP Vulkan Integration: The Zhong/Zhao Path ---

typedef struct {
    VulkanContext* vk;
    TSFiHelmholtzManifold* rebar_manifold;
} VulkanMcpContext;

static void manifold_noop() {}
static void manifold_directive(int* cnt, char* d) { 
    if (cnt) (*cnt)++;
    printf("[MANIFOLD] Directive: %s\n", d); 
}

VulkanMcpContext* tsfi_vulkan_mcp_init() {
    printf("[VULKAN_MCP] Initializing ReBAR Manifold...\n");
    
    VulkanMcpContext* ctx = (VulkanMcpContext*)lau_malloc(sizeof(VulkanMcpContext));
    if (!ctx) return NULL;

    // Use the existing init_vulkan() which establishes the context
    ctx->vk = init_vulkan();
    if (!ctx->vk) {
        printf("[VULKAN_MCP] ERROR: Failed to create Vulkan context.\n");
        lau_free(ctx);
        return NULL;
    }

    // Explicitly initialize ReBAR pool
    init_rebar_pool(ctx->vk);

    // Link lau_memory to this specific context
    lau_memory_init_gpu(ctx->vk);

    // Allocate the Manifold as a Wired System Object using the Mapped Struct paradigm
    ctx->rebar_manifold = (TSFiHelmholtzManifold*)lau_malloc_wired(sizeof(TSFiHelmholtzManifold));
    if (!ctx->rebar_manifold) return NULL;

    // Perform Bijective Wiring
    LauSystemHeader *h = (LauSystemHeader *)((char *)ctx->rebar_manifold - offsetof(LauWiredHeader, payload));
    
    TSFiLogicTable manifold_logic = {
        .logic_epoch = (void*)manifold_noop,
        .logic_state = (void*)manifold_noop,
        .logic_directive = manifold_directive,
        .logic_scramble = (void*)manifold_noop,
        .logic_provenance = (void*)manifold_noop
    };
    
    lau_wire_system((WaveSystem*)ctx->rebar_manifold, h, &manifold_logic);

    // Set Status Metadata
    h->resonance_as_status = lau_strdup("FHFD_ACTIVE_MAPPED");

    ctx->rebar_manifold->capacity = 64;
    ctx->rebar_manifold->count = 0;
    ctx->rebar_manifold->centrifugal_force = 1;

    // Bind function pointers for the "Clutch" engagement
    ctx->rebar_manifold->resolve_phi = (void*)tsfi_phi_resolve;
    ctx->rebar_manifold->crossover   = tsfi_phi_crossover;
    
    // Use lau_malloc_gpu to place the lattice in ReBAR (The Zhong Path)
    ctx->rebar_manifold->lattice = (TSFiPhiAtom*)lau_malloc_gpu(ctx->rebar_manifold->capacity * sizeof(TSFiPhiAtom), LAU_GPU_REBAR);
    
    if (!ctx->rebar_manifold->lattice) {
        printf("[VULKAN_MCP] ERROR: ReBAR allocation failed.\n");
        return NULL;
    }

    memset(ctx->rebar_manifold->lattice, 0, ctx->rebar_manifold->capacity * sizeof(TSFiPhiAtom));
    
    printf("[VULKAN_MCP] Manifold established in Device-Local Host-Visible Memory.\n");
    return ctx;
}

void tsfi_vulkan_mcp_populate_tools(VulkanMcpContext* ctx) {
    if (!ctx || !ctx->rebar_manifold) return;

    // Tool 0: wave512.run
    uint32_t secret_run = 0xA01;
    TSFiPhiAtom* a0 = &ctx->rebar_manifold->lattice[0];
    a0->secret_provenance = secret_run;
    for(int j=0; j<16; j++) a0->state_vector[j] = 0x11111111;
    a0->state_vector[15] = (a0->state_vector[15] & ~TSFI_SECRET_MASK) | secret_run;
    a0->svdag_node_id = 0x1000; // Bijective Geometry Pointer
    ctx->rebar_manifold->count++;

    // Tool 1: shell.read_file
    uint32_t secret_read = 0xA02;
    TSFiPhiAtom* a1 = &ctx->rebar_manifold->lattice[1];
    a1->secret_provenance = secret_read;
    for(int j=0; j<16; j++) a1->state_vector[j] = 0x22222222;
    a1->state_vector[15] = (a1->state_vector[15] & ~TSFI_SECRET_MASK) | secret_read;
    a1->svdag_node_id = 0x2000; // Bijective Geometry Pointer
    ctx->rebar_manifold->count++;

    printf("[VULKAN_MCP] %zu tools mapped to ReBAR lattice.\n", ctx->rebar_manifold->count);
}

void tsfi_vulkan_mcp_destroy(VulkanMcpContext* ctx) {
    if (!ctx) return;
    if (ctx->rebar_manifold) {
        if (ctx->rebar_manifold->lattice) lau_free(ctx->rebar_manifold->lattice);
        lau_free(ctx->rebar_manifold);
    }
    if (ctx->vk) cleanup_vulkan(ctx->vk);
    lau_free(ctx);
}

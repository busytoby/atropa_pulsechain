#ifndef VULKAN_SYSTEM_H
#define VULKAN_SYSTEM_H

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <wayland-client.h>
#include <vulkan/vulkan.h>
#include "window_inc/vulkan_struct.h"
#include "tsfi_types.h"
#include "tsfi_staging.h"
#include "window_inc/tsfi_input.h"

#include "xdg-shell-client-protocol.h"
#include "linux-dmabuf-v1-client-protocol.h"
#include "linux-explicit-synchronization-unstable-v1-client-protocol.h"
#include "lau_telemetry.h"
#include "lau_wire_log.h"
#include "lau_wire_mcp.h"
#include "vulkan_backend.h"

// --- Vulkan System Definition ---
DEFINE_MAPPED_STRUCT(VulkanSystem, 
    int system_id;
    VulkanBackendType backend_type;
    LauTelemetryState *telem;
    char *current_directive;
    double current_intensity;
    char *provenance_sig;

    struct wl_display *display;
    struct wl_registry *registry;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    
    struct xdg_wm_base *xdg_wm_base;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;

    struct wl_pointer *pointer;
    struct wl_keyboard *keyboard;
    struct wl_surface *surface;
    struct zwp_linux_dmabuf_v1 *dmabuf;
    struct wl_buffer *dma_buffer;
    VkImage dma_image;
    VkDeviceMemory dma_mem;
    
    // DMABUF Feedback State
    struct {
        bool feedback_received;
        uint32_t main_device_id_maj;
        uint32_t main_device_id_min;
        bool format_supported; // XRGB8888
    } dmabuf_state;

    // Fallback Upload Resources
    VkBuffer staging_vk_buffer;
    VkDeviceMemory staging_vk_memory;

    VulkanContext *vk;
    bool running;
    bool edit_mode;
    bool input_bar_visible;
    bool text_forward;
    bool text_right_align;
    int mouse_x, mouse_y;
    bool mouse_down;
    
    int width, height;
    bool maximized;
    bool fullscreen;
    bool minimized;
    bool external_render_enabled; // If true, skip clearing paint_buffer
    uint32_t last_key_scancode;   // Last pressed key (for Logic triggers)
    void *vty_render_thunk;
    
    TSFiInputState input_state;
    StagingBuffer *paint_buffer;
    bool is_ab4h;

    // --- Unified Communication Manifold ---
    LauMasterWavefront wavefront;

    // --- Neural Context (Gathered Data) ---
    char     neural_intent[256];   // Gemini's current stated goal
    char     neural_insight[1024]; // Gemini's last significant analysis
    uint64_t neural_epoch;         // Epoch of last gathering

    // --- Compute Architecture ---
    VkDescriptorSetLayout compute_desc_layout;
    VkPipelineLayout      compute_pipeline_layout;
    VkPipeline            compute_pipeline;
    VkDescriptorPool      compute_desc_pool;
    VkDescriptorSet       compute_desc_set;
    
    VkImage               compute_target_image; // The K0Rn Target
    VkDeviceMemory        compute_target_memory;
    VkImageView           compute_target_view;

    struct wl_data_device_manager *data_device_manager;
    struct wl_data_device *data_device;
    char *clipboard_buffer;

    // --- Zhong Architecture (Physical Context) ---
    uint64_t *gpu_sem_addr;     // Timeline Counter Pointer
    uint64_t gpu_sem_target;    // Last logical target
)

#endif

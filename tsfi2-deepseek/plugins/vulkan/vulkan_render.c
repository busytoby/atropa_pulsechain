#include <time.h>
#include "tsfi_hotloader.h"
#include "vulkan_render.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "vulkan_init.h"
#include "tsfi_vision.h"
#include "tsfi_wire_firmware.h"
#include "window_inc/vulkan_config.h"
#include <nmmintrin.h>

static const uint32_t *custom_frag_spv = NULL;
static size_t custom_frag_size = 0;
static VkPipeline custom_frag_pipeline = VK_NULL_HANDLE;
static VkPipelineLayout custom_frag_layout = VK_NULL_HANDLE;

static uint32_t calculate_telem_crc(const LauTelemetryState *s) {
    uint64_t crc = 0;
    crc = _mm_crc32_u64(crc, s->magic);
    crc = _mm_crc32_u64(crc, s->pid);
    crc = _mm_crc32_u64(crc, s->total_allocs);
    crc = _mm_crc32_u64(crc, s->total_frees);
    crc = _mm_crc32_u64(crc, s->active_allocs);
    crc = _mm_crc32_u64(crc, s->active_bytes);
    crc = _mm_crc32_u64(crc, s->peak_bytes);
    crc = _mm_crc32_u64(crc, s->exec_steps);
    crc = _mm_crc32_u64(crc, s->exec_last_ts);
    uint64_t *dir_ptr = (uint64_t*)s->last_directive_str;
    crc = _mm_crc32_u64(crc, *dir_ptr);
    crc = _mm_crc32_u64(crc, (uint64_t)s->recip_symmetry);
    return (uint32_t)crc;
}

void init_logical_pipeline(VulkanContext *vk, const uint32_t *spv_code, size_t spv_size) {
    if (!vk || !spv_code) return;

    printf("[DEBUG] Context: %p, Device: %p, CreateProc: %p\n", (void*)vk, (void*)vk->device, (void*)vk->vkCreateShaderModule);

    if (!vk->vkCreateShaderModule) {
        printf("[TSFI_VULKAN] ERROR: vkCreateShaderModule proc not loaded.\n");
        return;
    }

    VkShaderModuleCreateInfo moduleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spv_size,
        .pCode = spv_code
    };

    VkShaderModule logicalPipelineThunk;
    if (vk->vkCreateShaderModule(vk->device, &moduleInfo, NULL, &logicalPipelineThunk) != VK_SUCCESS) {
        printf("[TSFI_VULKAN] ERROR: Failed to create Logical Pipeline Thunk.\n");
        return;
    }

    // Pipeline Layout
    VkPipelineLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 0,
        .pSetLayouts = NULL,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = NULL
    };

    if (vk->vkCreatePipelineLayout(vk->device, &layoutInfo, NULL, &vk->compute_pipeline_layout) != VK_SUCCESS) {
        printf("[TSFI_VULKAN] ERROR: Failed to create pipeline layout.\n");
        vk->vkDestroyShaderModule(vk->device, logicalPipelineThunk, NULL);
        return;
    }

    // Compute Pipeline
    VkPipelineShaderStageCreateInfo stageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = logicalPipelineThunk,
        .pName = "main"
    };

    VkComputePipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = stageInfo,
        .layout = vk->compute_pipeline_layout
    };

    if (vk->vkCreateComputePipelines(vk->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vk->compute_pipeline) != VK_SUCCESS) {
        printf("[TSFI_VULKAN] ERROR: Failed to create compute pipeline.\n");
    }

    vk->vkDestroyShaderModule(vk->device, logicalPipelineThunk, NULL);
}

void init_terminal_pipeline(VulkanContext *vk, const uint32_t *spv_code, size_t spv_size) {
    if (!vk || !spv_code) return;

    VkShaderModuleCreateInfo moduleInfo = {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spv_size,
        .pCode = spv_code
    };

    VkShaderModule terminalShader;
    if (vk->vkCreateShaderModule(vk->device, &moduleInfo, NULL, &terminalShader) != VK_SUCCESS) {
        printf("[TSFI_VULKAN] ERROR: Failed to create Terminal Shader Module.\n");
        return;
    }

    // Descriptor Set Layout for Terminal Resources
    VkDescriptorSetLayoutBinding bindings[5] = {
        {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}, // VRAM
        {1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}, // UnicodeMap
        {2, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}, // Registry
        {3, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}, // DNA
        {4, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}  // OutImage
    };

    VkDescriptorSetLayoutCreateInfo dslInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 5,
        .pBindings = bindings
    };

    VkDescriptorSetLayout dsl;
    if (vk->vkCreateDescriptorSetLayout(vk->device, &dslInfo, NULL, &dsl) != VK_SUCCESS) {
        vk->vkDestroyShaderModule(vk->device, terminalShader, NULL);
        return;
    }

    // Pipeline Layout with push constants and descriptor set
    VkPushConstantRange pushConstantRange = {
        .stageFlags = VK_SHADER_STAGE_COMPUTE_BIT,
        .offset = 0,
        .size = 16 // 4 uints
    };

    VkPipelineLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &dsl,
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    if (vk->vkCreatePipelineLayout(vk->device, &layoutInfo, NULL, &vk->compute_pipeline_layout) != VK_SUCCESS) {
        vk->vkDestroyDescriptorSetLayout(vk->device, dsl, NULL);
        vk->vkDestroyShaderModule(vk->device, terminalShader, NULL);
        return;
    }

    VkPipelineShaderStageCreateInfo stageInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_COMPUTE_BIT,
        .module = terminalShader,
        .pName = "main"
    };

    VkComputePipelineCreateInfo pipelineInfo = {
        .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
        .stage = stageInfo,
        .layout = vk->compute_pipeline_layout
    };

    if (vk->vkCreateComputePipelines(vk->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &vk->compute_pipeline) != VK_SUCCESS) {
        printf("[TSFI_VULKAN] ERROR: Failed to create compute pipeline.\n");
    }

    vk->vkDestroyShaderModule(vk->device, terminalShader, NULL);
    // Note: DSL is leaked here for brevity in prototype, should be stored in VulkanContext
}

void dispatch_terminal_shader(VulkanSystem *s) {
    if (!s || !s->vk || !s->vk->compute_pipeline) return;
    VulkanContext *vk = s->vk;

    // In a real implementation, we would create a descriptor set and update it.
    // For this architectural prototype, we assume the resources are already bound
    // or we use Buffer Device Address (BDA) which the shader is ready for.
    // We will use Push Constants to pass the BDA pointers to the shader.

    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (!fw) return;

    // Use established 10x18 metrics for the terminal monitor
    uint32_t char_w = 10;
    uint32_t char_h = 18;

    uint32_t pushData[4] = { s->width, s->height, char_w, char_h };
    vk->vkCmdPushConstants(vk->command_buffers[vk->currentFrame], vk->compute_pipeline_layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, 16, pushData);

    vk->vkCmdBindPipeline(vk->command_buffers[vk->currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, vk->compute_pipeline);
    
    // Dispatch: enough wavefronts to cover the screen
    uint32_t groupCountX = (s->width + 7) / 8;
    uint32_t groupCountY = (s->height + 7) / 8;
    vk->vkCmdDispatch(vk->command_buffers[vk->currentFrame], groupCountX, groupCountY, 1);
}

// Implement the high-level init_swapchain from vulkan_render.h
bool init_swapchain(VulkanSystem *s) {
    if (!s || !s->vk) return false;
    // Delegate to low-level init
    init_vk_swapchain(s->vk, s->width, s->height);
    return (s->vk->swapchain != NULL);
}

void recreate_swapchain(VulkanSystem *s) {
    if (!s || !s->vk || !s->vk->device) return;
    s->vk->vkDeviceWaitIdle(s->vk->device);
    printf("[VULKAN] Recreating Swapchain to %dx%d...\n", s->width, s->height);
    init_vk_swapchain(s->vk, s->width, s->height);
}

#include "tsfi_broadcaster.h"

void draw_broadcaster_BASE(VulkanSystem *s, LauBroadcaster *lb) {
    if (!s || !s->paint_buffer || !lb) return;

    // Use 9px width, 10px height per char for a 160x60 BASE
    int char_w = 8; // Compact
    int char_h = 10;
    int BASE_w = BROADCASTER_COLS * char_w + 20;
    int BASE_h = BROADCASTER_ROWS * char_h + 20;
    
    // Check for "total darkness" or initial render
    static bool first_render = true;
    if (first_render) {
        // Background: Fully Opaque Dark Navy (Clears the area)
        draw_rounded_rect(s->paint_buffer, 5, 95, BASE_w, BASE_h, 5, 0xFF050515);
        first_render = false;
        lb->is_dirty = true;
        for (int y = 0; y < BROADCASTER_ROWS; y++) {
            for (int x = 0; x < BROADCASTER_COLS; x++) lb->char_BASE_dirty[y][x] = true;
        }
    }

    if (!lb->is_dirty) return;

    for (int y = 0; y < BROADCASTER_ROWS; y++) {
        for (int x = 0; x < BROADCASTER_COLS; x++) {
            if (!lb->char_BASE_dirty[y][x]) continue;

            // Clear the cell background before redrawing
            int px = 15 + (x * char_w);
            int py = 105 + (y * char_h);
            staging_blend_over_avx512(s->paint_buffer, px, py, char_w, char_h, 0xFF050515);

            uint32_t codepoint = lb->char_BASE[y][x];
            if (codepoint != 0 && codepoint != ' ') {
                uint32_t color = 0xFF00DD00; // Terminal Green
                if (y == lb->cursor_y) color = 0xFF00FFFF; // Cyan cursor row
                draw_debug_codepoint(s->paint_buffer, px, py, codepoint, color);
            }
            lb->char_BASE_dirty[y][x] = false;
        }
    }
    
    // Draw cursor block
    if (lb->cursor_x < BROADCASTER_COLS && lb->cursor_y < BROADCASTER_ROWS) {
        staging_blend_over_avx512(s->paint_buffer, 15 + lb->cursor_x * char_w, 105 + lb->cursor_y * char_h, char_w, char_h - 2, 0xFFFFFFFF);
    }
    lb->is_dirty = false;
}

#include "tsfi_wire_firmware.h"

void draw_gemini_monitor(VulkanSystem *s) {
    if (!s || !s->paint_buffer) return;

    LauWireFirmware *fw = tsfi_wire_firmware_get();
    if (!fw) return;

    // Use proportional font metrics for the 14px Truetype font
    int char_w = 10;
    int char_h = 18;
    int mon_x = 10;
    int mon_y = 400; // Position below the main sim viewport
    int mon_w = 120 * char_w + 20; // Fit 120 chars wide
    int mon_h = 24 * char_h + 20;  // Fit 24 lines

    // Draw background
    draw_rounded_rect(s->paint_buffer, mon_x - 5, mon_y - 5, mon_w, mon_h, 5, 0xFF000000);
    draw_debug_text(s->paint_buffer, mon_x, mon_y - 20, "GEMINI NEURAL MONITOR (LIVE WAVEFRONT)", 0xFF00FFFF, true);

    // Static ANSI 16-color palette mapping
    static const uint32_t palette[16] = {
        0xFF000000, 0xFFCC0000, 0xFF00CC00, 0xFFCCCC00, // Black, Red, Green, Yellow
        0xFF0000CC, 0xFFCC00CC, 0xFF00CCCC, 0xFFCCCCCC, // Blue, Magenta, Cyan, White
        0xFF555555, 0xFFFF5555, 0xFF55FF55, 0xFFFFFF55, // Br Black, Br Red, Br Green, Br Yellow
        0xFF5555FF, 0xFFFF55FF, 0xFF55FFFF, 0xFFFFFFFF  // Br Blue, Br Magenta, Br Cyan, Br White
    };

    // Render the active VRAM region
    // The VRAM is 160x60, but our viewport is 120x24.
    // We will render the bottom 24 rows of the VRAM.
    int start_y = LAU_VRAM_ROWS - 24;
    if (start_y < 0) start_y = 0;

    for (int y = 0; y < 24; y++) {
        int vram_y = start_y + y;
        if (vram_y >= LAU_VRAM_ROWS) break;
        
        for (int x = 0; x < 120; x++) {
            if (x >= LAU_VRAM_COLS) break;
            
            LauVRAMCell cell = fw->vram.grid[vram_y][x];
            if (cell.character > 32) {
                uint32_t fg = palette[cell.fg_color & 0xF];
                if (cell.attributes & 1) fg = palette[(cell.fg_color & 0x7) + 8]; // Bold
                
                int px = mon_x + 5 + (x * char_w);
                int py = mon_y + 5 + (y * char_h);
                draw_debug_codepoint(s->paint_buffer, px, py, cell.character, fg);
            }
        }
    }
    
    // Draw cursor
    int cy = fw->vram.cursor_y - start_y;
    int cx = fw->vram.cursor_x;
    if (cy >= 0 && cy < 24 && cx >= 0 && cx < 120) {
        staging_blend_over_avx512(s->paint_buffer, mon_x + 5 + cx * char_w, mon_y + 5 + cy * char_h, char_w, char_h - 2, 0xFFFFFFFF);
    }
}

void draw_ui_elements(VulkanSystem *s) {
    if (!s || !s->paint_buffer) return;
    if (s->disable_ui_overlay) return;

    // Clear telemetry/cockpit area (top part of the screen)
    staging_blend_over_avx512(s->paint_buffer, 0, 0, s->paint_buffer->width, 90, 0xFF000000);

    // Check if we have a linked broadcaster in the Helmholtz List
    // For Phase 1, we look for a global or specific linkage.
    // In vtube --test, LauBroadcaster is allocated.
    extern LauBroadcaster* get_active_broadcaster(void);
    LauBroadcaster *lb = get_active_broadcaster();
    if (lb) {
        draw_broadcaster_BASE(s, lb);
    }

    draw_gemini_monitor(s);

    if (s->telem) {
        char buf[512];
        uint32_t current_crc = calculate_telem_crc(s->telem);
        bool valid = (current_crc == s->telem->header_crc);
        
        uint32_t color = valid ? 0xFF00FF00 : 0xFF0000FF; // Green if valid, Red if torn

        if (!valid) {
            draw_debug_text(s->paint_buffer, 10, 10, "TELEMETRY TORN (WAITING...)", color, true);
            return;
        }

        // 1. Cockpit Stats
        snprintf(buf, sizeof(buf), "DIRECTIVE : %s", s->telem->last_directive_str);
        draw_debug_text(s->paint_buffer, 10, 10, buf, color, true);

        snprintf(buf, sizeof(buf), "SYMMETRY  : %.2f%%", s->telem->recip_symmetry * 100.0f);
        draw_debug_text(s->paint_buffer, 10, 25, buf, color, true);

        snprintf(buf, sizeof(buf), "INTENSITY : %.2f", s->telem->current_intensity);
        draw_debug_text(s->paint_buffer, 10, 40, buf, color, true);

        // 2. Vision Analysis (Direct Probe)
        uint8_t max_val = tsfi_vision_max_value_raw(s->paint_buffer->data, s->paint_buffer->size);
        snprintf(buf, 256, "VISION MAX: %u", max_val);
        draw_debug_text(s->paint_buffer, 10, 70, buf, 0xFF00FFFF, true);

        if (max_val < 20) {
            draw_debug_text(s->paint_buffer, 10, 85, "VISION_ALERT: SCREEN TOO DARK", 0xFFFF0000, true);
        }

    } else {
        draw_debug_text(s->paint_buffer, 10, 10, "COCKPIT LINK : OFFLINE", 0xFFFF0000, true);
        
        // Even if cockpit is offline, verify BASE content
        uint8_t BASE_max = tsfi_vision_max_value_raw(s->paint_buffer->data, s->paint_buffer->size);
        if (BASE_max < 20) {
             staging_blend_over_avx512(s->paint_buffer, 0, 0, 50, 50, 0xFFFF0000); // Red corner if dark
        }
    }
    
    // Draw live Vulkan knowledge graph of LP connections
    extern void draw_vulkan_knowledge_graph(VulkanSystem *s);
    draw_vulkan_knowledge_graph(s);
}

static VkBuffer g_staging_vk_buffer = VK_NULL_HANDLE;
static VkDeviceMemory g_staging_vk_memory = VK_NULL_HANDLE;
static size_t g_staging_vk_buffer_allocated_size = 0;
static void* g_staging_vk_persistent_map = NULL;

bool init_staging_vk_buffer(VulkanSystem *s, size_t size) {
    if (!s || !s->vk) return false;
    VulkanContext *vk = s->vk;

    VkExternalMemoryBufferCreateInfo extBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_BUFFER_CREATE_INFO,
        .pNext = NULL,
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
    };

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = &extBufferInfo,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    if (vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &g_staging_vk_buffer) != VK_SUCCESS) return false;

    VkMemoryRequirements memReqs;
    vk->vkGetBufferMemoryRequirements(vk->device, g_staging_vk_buffer, &memReqs);

    VkExportMemoryAllocateInfo exportAllocInfo = {
        .sType = VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO,
        .pNext = NULL,
        .handleTypes = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
    };

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = &exportAllocInfo,
        .allocationSize = memReqs.size
    };

    // Find host visible memory
    bool found = false;
    for (uint32_t i = 0; i < vk->memory_properties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) && 
            (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) &&
            (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
            allocInfo.memoryTypeIndex = i;
            found = true;
            break;
        }
    }
    if (!found) return false;

    if (vk->vkAllocateMemory(vk->device, &allocInfo, NULL, &g_staging_vk_memory) != VK_SUCCESS) return false;
    vk->vkBindBufferMemory(vk->device, g_staging_vk_buffer, g_staging_vk_memory, 0);

    // Persistently map the coherent memory to save call overhead on render iterations (Zero-copy)
    if (vk->vkMapMemory(vk->device, g_staging_vk_memory, 0, size, 0, &g_staging_vk_persistent_map) != VK_SUCCESS) {
        g_staging_vk_persistent_map = NULL;
        return false;
    }

    if (vk->vkGetMemoryFdKHR) {
        VkMemoryGetFdInfoKHR fdInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
            .pNext = NULL,
            .memory = g_staging_vk_memory,
            .handleType = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT
        };
        
        int fd = -1;
        if (vk->vkGetMemoryFdKHR(vk->device, &fdInfo, &fd) == VK_SUCCESS) {
            close(fd);
        }
    }

    return true;
}

void upload_staging_to_image(VulkanSystem *s, StagingBuffer *sb, VkImage target, VkCommandBuffer cmd) {
    if (!s || !s->vk || !sb || !target || !cmd) return;
    VulkanContext *vk = s->vk;

    size_t bytes_per_pixel = (vk->swapchainFormat == VK_FORMAT_R16G16B16A16_SFLOAT) ? 8 : 4;
    size_t dest_size = s->width * s->height * bytes_per_pixel;

    if (!g_staging_vk_buffer || g_staging_vk_buffer_allocated_size != dest_size) {
        if (g_staging_vk_buffer != VK_NULL_HANDLE) {
            vk->vkDestroyBuffer(vk->device, g_staging_vk_buffer, NULL);
            g_staging_vk_buffer = VK_NULL_HANDLE;
        }
        if (g_staging_vk_memory != VK_NULL_HANDLE) {
            vk->vkFreeMemory(vk->device, g_staging_vk_memory, NULL);
            g_staging_vk_memory = VK_NULL_HANDLE;
        }
        if (!init_staging_vk_buffer(s, dest_size)) return;
        g_staging_vk_buffer_allocated_size = dest_size;
    }

    // 1. Copy CPU data to Staging VkBuffer via persistent mapping (Zero-copy)
    void* mapped = g_staging_vk_persistent_map;
    if (!mapped) return;
    
    if (vk->swapchainFormat == VK_FORMAT_R16G16B16A16_SFLOAT) {
        typedef struct { uint16_t r, g, b, a; } AB4HPixelLocal;
        AB4HPixelLocal *src = (AB4HPixelLocal*)sb->data;
        AB4HPixelLocal *dst = (AB4HPixelLocal*)mapped;
        int dest_w = s->width;
        int dest_h = s->height;
        for (int y = 0; y < dest_h; y++) {
            int src_y = (y * (int)sb->height) / dest_h;
            if (src_y >= (int)sb->height) src_y = (int)sb->height - 1;
            for (int x = 0; x < dest_w; x++) {
                int src_x = (x * (int)sb->width) / dest_w;
                if (src_x >= (int)sb->width) src_x = (int)sb->width - 1;
                dst[y * dest_w + x] = src[src_y * sb->width + src_x];
            }
        }
    } else if (sb->stride == sb->width * 8) {
        typedef struct { uint16_t r, g, b, a; } AB4HPixelLocal;
        AB4HPixelLocal *src = (AB4HPixelLocal*)sb->data;
        uint32_t *dst = (uint32_t*)mapped;
        
        int dest_w = s->width;
        int dest_h = s->height;
        for (int y = 0; y < dest_h; y++) {
            int src_y = (y * (int)sb->height) / dest_h;
            if (src_y >= (int)sb->height) src_y = (int)sb->height - 1;
            for (int x = 0; x < dest_w; x++) {
                int src_x = (x * (int)sb->width) / dest_w;
                if (src_x >= (int)sb->width) src_x = (int)sb->width - 1;
                
                size_t src_idx = src_y * sb->width + src_x;
                
                // Half-to-float helper inline
                uint16_t h_r = src[src_idx].r;
                uint32_t sign = (h_r & 0x8000) << 16;
                uint32_t exponent = (h_r & 0x7c00) >> 10;
                uint32_t mantissa = (h_r & 0x03ff) << 13;
                if (exponent == 0) {
                    if (mantissa != 0) {
                        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
                        exponent++; exponent += 127 - 15;
                    }
                } else if (exponent == 31) exponent = 255;
                else exponent += 127 - 15;
                union { uint32_t u; float f; } u_r = { sign | (exponent << 23) | mantissa };
                float fr = (exponent == 0 && mantissa == 0) ? 0.0f : u_r.f;

                uint16_t h_g = src[src_idx].g;
                sign = (h_g & 0x8000) << 16;
                exponent = (h_g & 0x7c00) >> 10;
                mantissa = (h_g & 0x03ff) << 13;
                if (exponent == 0) {
                    if (mantissa != 0) {
                        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
                        exponent++; exponent += 127 - 15;
                    }
                } else if (exponent == 31) exponent = 255;
                else exponent += 127 - 15;
                union { uint32_t u; float f; } u_g = { sign | (exponent << 23) | mantissa };
                float fg = (exponent == 0 && mantissa == 0) ? 0.0f : u_g.f;

                uint16_t h_b = src[src_idx].b;
                sign = (h_b & 0x8000) << 16;
                exponent = (h_b & 0x7c00) >> 10;
                mantissa = (h_b & 0x03ff) << 13;
                if (exponent == 0) {
                    if (mantissa != 0) {
                        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
                        exponent++; exponent += 127 - 15;
                    }
                } else if (exponent == 31) exponent = 255;
                else exponent += 127 - 15;
                union { uint32_t u; float f; } u_b = { sign | (exponent << 23) | mantissa };
                float fb = (exponent == 0 && mantissa == 0) ? 0.0f : u_b.f;

                uint16_t h_a = src[src_idx].a;
                sign = (h_a & 0x8000) << 16;
                exponent = (h_a & 0x7c00) >> 10;
                mantissa = (h_a & 0x03ff) << 13;
                if (exponent == 0) {
                    if (mantissa != 0) {
                        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
                        exponent++; exponent += 127 - 15;
                    }
                } else if (exponent == 31) exponent = 255;
                else exponent += 127 - 15;
                union { uint32_t u; float f; } u_a = { sign | (exponent << 23) | mantissa };
                float fa = (exponent == 0 && mantissa == 0) ? 0.0f : u_a.f;

                // Clamping to standard range
                float max_val = 1.0f;
                float min_val = 0.0f;
                float cl_r = fr < min_val ? min_val : (fr > max_val ? max_val : fr);
                float cl_g = fg < min_val ? min_val : (fg > max_val ? max_val : fg);
                float cl_b = fb < min_val ? min_val : (fb > max_val ? max_val : fb);
                float cl_a = fa < min_val ? min_val : (fa > max_val ? max_val : fa);

                uint8_t r = (uint8_t)(cl_r * 255.0f);
                uint8_t g = (uint8_t)(cl_g * 255.0f);
                uint8_t b = (uint8_t)(cl_b * 255.0f);
                uint8_t a = (uint8_t)(cl_a * 255.0f);

                // B8G8R8A8 little-endian layout (B, G, R, A)
                dst[y * dest_w + x] = ((uint32_t)a << 24) | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
            }
        }
    } else {
        memcpy(mapped, sb->data, sb->size);
    }

    // 2. Transition image to TRANSFER_DST_OPTIMAL
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = target,
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 },
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT
    };
    vk->vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

    // 3. Copy Buffer to Image
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
        .imageOffset = {0, 0, 0},
        .imageExtent = {(uint32_t)s->width, (uint32_t)s->height, 1}
    };
    vk->vkCmdCopyBufferToImage(cmd, g_staging_vk_buffer, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // 4. Transition to PRESENT_SRC_KHR
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = 0;
    vk->vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);
}

void draw_frame(VulkanSystem *s) {
    if (!s || !s->vk) return;
    static int frame_count = 0;
    if (frame_count % 60 == 0) {
        printf("[VULKAN] Rendering Frame %d...\n", frame_count);
    }
    frame_count++;

    VulkanContext *vk = s->vk;
    if (!vk->swapchain) return;

    // Monitor frame timing for dynamic presentation mode toggling
    static struct timespec prev_time = {0};
    struct timespec curr_time;
    clock_gettime(CLOCK_MONOTONIC, &curr_time);
    float frame_time_ms = 0.0f;
    if (prev_time.tv_sec != 0) {
        frame_time_ms = (float)(curr_time.tv_sec - prev_time.tv_sec) * 1000.0f +
                        (float)(curr_time.tv_nsec - prev_time.tv_nsec) / 1000000.0f;
    }
    prev_time = curr_time;

    static int low_time_cnt = 0;
    static int high_time_cnt = 0;
    if (frame_time_ms > 0.0f && !vk->is_leased) {
        if (frame_time_ms < 14.0f) {
            low_time_cnt++;
            high_time_cnt = 0;
        } else if (frame_time_ms > 16.0f) {
            high_time_cnt++;
            low_time_cnt = 0;
        } else {
            low_time_cnt = 0;
            high_time_cnt = 0;
        }

        if (low_time_cnt > 30 && vk->currentPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) {
            printf("\n[VULKAN] Pacing: Frame time is %.2f ms. Switching to MAILBOX present mode...\n", frame_time_ms);
            vk->desiredPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            recreate_swapchain(s);
            low_time_cnt = 0;
        } else if (high_time_cnt > 30 && vk->currentPresentMode != VK_PRESENT_MODE_FIFO_KHR) {
            printf("\n[VULKAN] Pacing: Frame time is %.2f ms. Reverting to FIFO present mode...\n", frame_time_ms);
            vk->desiredPresentMode = VK_PRESENT_MODE_FIFO_KHR;
            recreate_swapchain(s);
            high_time_cnt = 0;
        }
    }

    vk->vkWaitForFences(vk->device, 1, &vk->inFlightFences[vk->currentFrame], VK_TRUE, UINT64_MAX);
    vk->vkResetFences(vk->device, 1, &vk->inFlightFences[vk->currentFrame]);

    uint32_t imageIndex;
    VkResult res = vk->vkAcquireNextImageKHR(vk->device, vk->swapchain, UINT64_MAX, vk->imageAvailableSemaphores[vk->currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (res != VK_SUCCESS && res != VK_SUBOPTIMAL_KHR) {
        printf("[VULKAN] vkAcquireNextImageKHR failed with status: %d\n", res);
        if (res == VK_ERROR_OUT_OF_DATE_KHR) {
            recreate_swapchain(s);
        }
        return;
    }

    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vk->vkBeginCommandBuffer(vk->command_buffers[vk->currentFrame], &beginInfo);

    // 1. Reset and Start Queries
    if (vk->query_pool_perf) {
        vk->vkCmdResetQueryPool(vk->command_buffers[vk->currentFrame], vk->query_pool_perf, 0, 2);
        vk->vkCmdWriteTimestamp(vk->command_buffers[vk->currentFrame], VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, vk->query_pool_perf, 0);
    }
    if (vk->query_pool_stats) {
        vk->vkCmdResetQueryPool(vk->command_buffers[vk->currentFrame], vk->query_pool_stats, 0, 1);
        vk->vkCmdBeginQuery(vk->command_buffers[vk->currentFrame], vk->query_pool_stats, 0, 0);
    }

    // Update UI Elements (CPU Side)
    draw_ui_elements(s);

    // 1. Transition Image Layout for Dynamic Rendering (COLOR_ATTACHMENT_OPTIMAL)
    VkImageMemoryBarrier2 barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .image = vk->swapchainImages[imageIndex],
        .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
    };

    VkDependencyInfo depInfo = {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &barrier
    };
    vk->vkCmdPipelineBarrier2(vk->command_buffers[vk->currentFrame], &depInfo);

    // 2. Begin Dynamic Rendering Session for Plane 71 Promotion
    VkRenderingAttachmentInfo colorAttachment = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = vk->swapchainImageViews[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue = { .color = { .float32 = { 0.0f, 0.1f, 0.2f, 1.0f } } } // TSFi Blue
    };

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .flags = VK_RENDERING_CONTENTS_SECONDARY_COMMAND_BUFFERS_BIT,
        .renderArea = { {0, 0}, vk->swapchainImages[imageIndex] ? (VkExtent2D){(uint32_t)s->width, (uint32_t)s->height} : (VkExtent2D){0, 0} }, // Target swapchain resolution
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment
    };

    vk->vkCmdBeginRendering(vk->command_buffers[vk->currentFrame], &renderingInfo);

    // If custom fragment (compute) shader is loaded and we are in 320x200 bitmap mode, execute it directly
    if (custom_frag_spv && s->width == 320 && s->height == 200) {
        if (custom_frag_pipeline == VK_NULL_HANDLE) {
            VkShaderModuleCreateInfo moduleInfo = {
                .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
                .codeSize = custom_frag_size,
                .pCode = custom_frag_spv
            };
            VkShaderModule mod;
            if (vk->vkCreateShaderModule(vk->device, &moduleInfo, NULL, &mod) == VK_SUCCESS) {
                VkDescriptorSetLayoutBinding bindings[2] = {
                    {0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}, // Input Buffer
                    {1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,  1, VK_SHADER_STAGE_COMPUTE_BIT, NULL}  // Output Image
                };
                VkDescriptorSetLayoutCreateInfo dslInfo = {
                    .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
                    .bindingCount = 2,
                    .pBindings = bindings
                };
                VkDescriptorSetLayout dsl;
                if (vk->vkCreateDescriptorSetLayout(vk->device, &dslInfo, NULL, &dsl) == VK_SUCCESS) {
                    VkPipelineLayoutCreateInfo layoutInfo = {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
                        .setLayoutCount = 1,
                        .pSetLayouts = &dsl
                    };
                    if (vk->vkCreatePipelineLayout(vk->device, &layoutInfo, NULL, &custom_frag_layout) == VK_SUCCESS) {
                        VkPipelineShaderStageCreateInfo stageInfo = {
                            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                            .stage = VK_SHADER_STAGE_COMPUTE_BIT,
                            .module = mod,
                            .pName = "main"
                        };
                        VkComputePipelineCreateInfo pipelineInfo = {
                            .sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
                            .stage = stageInfo,
                            .layout = custom_frag_layout
                        };
                        vk->vkCreateComputePipelines(vk->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &custom_frag_pipeline);
                    }
                }
                vk->vkDestroyShaderModule(vk->device, mod, NULL);
            }
        }
        if (custom_frag_pipeline != VK_NULL_HANDLE) {
            vk->vkCmdBindPipeline(vk->command_buffers[vk->currentFrame], VK_PIPELINE_BIND_POINT_COMPUTE, custom_frag_pipeline);
            vk->vkCmdDispatch(vk->command_buffers[vk->currentFrame], 40, 25, 1);
        }
    }

    // --- SECONDARY COMMAND BUFFER DISPATCH (Plane 71 Promotion) ---
    // In a production TSFi environment, this recording is performed by a Zhao worker thread.
    VkCommandBufferInheritanceRenderingInfoKHR inheritanceRenderingInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDERING_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = (VkFormat[]){ vk->swapchainFormat },
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

    VkCommandBufferInheritanceInfo inheritanceInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
        .pNext = &inheritanceRenderingInfo
    };

    VkCommandBufferBeginInfo secondaryBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
        .pInheritanceInfo = &inheritanceInfo
    };

    vk->vkBeginCommandBuffer(vk->secondary_command_buffers[vk->currentFrame], &secondaryBeginInfo);
    
    // (Secondary Recording: Apply High-Quality Post-Processing directly to scanout if possible)
    if (s->is_ab4h) {
        extern void tsfi_bloom_thunk_ab4h(uint16_t* pixels, int width, int height, float threshold, float intensity);
        if (tsfi_bloom_thunk_ab4h) {
            tsfi_bloom_thunk_ab4h((uint16_t*)s->paint_buffer->data, s->paint_buffer->width, s->paint_buffer->height, 0.85f, 0.4f);
        }
    } else {
        if (tsfi_bloom_thunk) {
            tsfi_bloom_thunk((uint32_t*)s->paint_buffer->data, s->paint_buffer->width, s->paint_buffer->height, 0.85f, 0.4f);
        }
    }
    
    vk->vkEndCommandBuffer(vk->secondary_command_buffers[vk->currentFrame]);

    // Merge Secondary Work into Primary Path
    vk->vkCmdExecuteCommands(vk->command_buffers[vk->currentFrame], 1, &vk->secondary_command_buffers[vk->currentFrame]);

    vk->vkCmdEndRendering(vk->command_buffers[vk->currentFrame]);

    // 2. End and Capture Queries
    if (vk->query_pool_stats) {
        vk->vkCmdEndQuery(vk->command_buffers[vk->currentFrame], vk->query_pool_stats, 0);
    }
    if (vk->query_pool_perf) {
        vk->vkCmdWriteTimestamp(vk->command_buffers[vk->currentFrame], VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, vk->query_pool_perf, 1);
        
        VkBufferMemoryBarrier2 rebarBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
            .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
            .buffer = vk->rebar_buffer,
            .offset = vk->rebar_pool_size - 4096,
            .size = 4096
        };
        VkDependencyInfo rebarDep = {
            .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
            .bufferMemoryBarrierCount = 1,
            .pBufferMemoryBarriers = &rebarBarrier
        };
        vk->vkCmdPipelineBarrier2(vk->command_buffers[vk->currentFrame], &rebarDep);

        // Push Timing Facts (Offset 0 in Query Anchor)
        vk->vkCmdCopyQueryPoolResults(vk->command_buffers[vk->currentFrame], 
                                      vk->query_pool_perf, 0, 2, 
                                      vk->rebar_buffer, vk->rebar_pool_size - 4096, 
                                      sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }
    if (vk->query_pool_stats) {
        // Push Statistics Facts (Offset 128 in Query Anchor to avoid overlap)
        vk->vkCmdCopyQueryPoolResults(vk->command_buffers[vk->currentFrame], 
                                      vk->query_pool_stats, 0, 1, 
                                      vk->rebar_buffer, vk->rebar_pool_size - 4096 + 128, 
                                      sizeof(uint64_t) * 4, VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    }

    // 3. Transition back to PRESENT_SRC_KHR for Physical Scanout
    barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vk->vkCmdPipelineBarrier2(vk->command_buffers[vk->currentFrame], &depInfo);

    // Upload Staging Buffer to Swapchain Image
    upload_staging_to_image(s, s->paint_buffer, vk->swapchainImages[imageIndex], vk->command_buffers[vk->currentFrame]);

    vk->vkEndCommandBuffer(vk->command_buffers[vk->currentFrame]);

    vk->timelineValue++;
    uint64_t signalValues[] = { 0, vk->timelineValue }; // Binary semaphore value is ignored but must be present
    VkTimelineSemaphoreSubmitInfo timelineInfo = {
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .signalSemaphoreValueCount = 2,
        .pSignalSemaphoreValues = signalValues
    };

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_TRANSFER_BIT };
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = &timelineInfo,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk->imageAvailableSemaphores[vk->currentFrame],
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk->command_buffers[vk->currentFrame],
        .signalSemaphoreCount = 2,
        .pSignalSemaphores = (VkSemaphore[]){ vk->renderFinishedSemaphores[imageIndex % 8], vk->timelineSemaphore }
    };

    vk->vkQueueSubmit(vk->queue, 1, &submitInfo, vk->inFlightFences[vk->currentFrame]);

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &vk->renderFinishedSemaphores[imageIndex % 8],
        .swapchainCount = 1,
        .pSwapchains = &vk->swapchain,
        .pImageIndices = &imageIndex
    };

    if (vk->is_leased) {
        // Mode B: Sovereign Plane 71
        // The display core is directly scanning out from the swapchain image.
        // On some drivers, vkQueuePresentKHR is still used for the display surface, 
        // while on others we would perform a direct atomic commit.
        vk->vkQueuePresentKHR(vk->queue, &presentInfo);
        static uint32_t p71_count = 0;
        if (++p71_count % 100 == 0) printf("\r[HW] Directly drawing to Plane 71 (Epoch: %u)  ", vk->currentFrame);
        fflush(stdout);
    } else {
        // Mode A: Standard Wayland Window
        VkResult res_present = vk->vkQueuePresentKHR(vk->queue, &presentInfo);
        if (res_present != VK_SUCCESS) {
            printf("[VULKAN] vkQueuePresentKHR returned status: %d\n", res_present);
            if (res_present == VK_ERROR_OUT_OF_DATE_KHR || res_present == VK_SUBOPTIMAL_KHR) {
                recreate_swapchain(s);
            }
        }
    }

    vk->currentFrame = (vk->currentFrame + 1) % 3;
}

void set_custom_fragment_shader(const uint32_t *spv, size_t size) {
    custom_frag_spv = spv;
    custom_frag_size = size;
    custom_frag_pipeline = VK_NULL_HANDLE;
    custom_frag_layout = VK_NULL_HANDLE;
}

#include <math.h>
#include <stdlib.h>
#include <strings.h>

#pragma pack(push, 1)
typedef struct {
    char address[64];
    char symbol[64];
    char name[128];
    uint64_t decimals;
    double price_pls;
    uint64_t last_update;
} VkRdbmsTokenRow;

typedef struct {
    uint32_t active;
    uint32_t col_count;
    uint32_t count;
    uint32_t capacity;
    VkRdbmsTokenRow rows[512];
} VkLauRdbmsTable;

typedef struct {
    char token0[64];
    char token1[64];
    double price;
} VkSwapEdge;
#pragma pack(pop)

static void staging_draw_circle(StagingBuffer *sb, int cx, int cy, int r, uint32_t color) {
    for (int y = -r; y <= r; y++) {
        for (int x = -r; x <= r; x++) {
            if (x*x + y*y <= r*r) {
                int px = cx + x;
                int py = cy + y;
                if (px >= 0 && px < (int)sb->width && py >= 0 && py < (int)sb->height) {
                    ((uint32_t*)sb->data)[py * sb->width + px] = color;
                }
            }
        }
    }
}

static void staging_draw_line(StagingBuffer *sb, int x0, int y0, int x1, int y1, uint32_t color) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;
    while (1) {
        if (x0 >= 0 && x0 < (int)sb->width && y0 >= 0 && y0 < (int)sb->height) {
            ((uint32_t*)sb->data)[y0 * sb->width + x0] = color;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

void draw_vulkan_knowledge_graph(VulkanSystem *s) {
    if (!s || !s->paint_buffer) return;
    StagingBuffer *sb = s->paint_buffer;
    
    static bool loaded = false;
    static VkRdbmsTokenRow tokens[512];
    static int token_count = 0;
    static VkSwapEdge edges[1024];
    static int edge_count = 0;
    
    typedef struct {
        char address[64];
        char symbol[16];
        char name[64];
        float x, y;
        float vx, vy;
        uint32_t color;
    } RenderNode;
    
    static RenderNode render_nodes[512];
    static int render_nodes_count = 0;
    static int dragged_node_idx = -1;
    
    if (!loaded) {
        static char sol_addresses[256][64];
        static int sol_addr_count = 0;
        
        FILE *f_sol = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/addresses.sol", "r");
        if (f_sol) {
            char line[512];
            while (fgets(line, sizeof(line), f_sol)) {
                char *addr_start = strstr(line, "address(0x");
                if (addr_start) {
                    addr_start += 8;
                    char *addr_end = strchr(addr_start, ')');
                    if (addr_end && (addr_end - addr_start) == 42) {
                        char clean_addr[64] = "0x";
                        strncat(clean_addr, addr_start + 2, 40);
                        
                        bool dup = false;
                        for (int k = 0; k < sol_addr_count; k++) {
                            if (strcasecmp(sol_addresses[k], clean_addr) == 0) {
                                dup = true;
                                break;
                            }
                        }
                        if (!dup && sol_addr_count < 256) {
                            strcpy(sol_addresses[sol_addr_count++], clean_addr);
                        }
                    }
                }
            }
            fclose(f_sol);
        }

        FILE *f_meta = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/assets/contract_metadata.dat.bin", "rb");
        if (f_meta) {
            VkLauRdbmsTable table;
            if (fread(&table, sizeof(VkLauRdbmsTable), 1, f_meta) == 1) {
                token_count = table.count;
                if (token_count > 512) token_count = 512;
                memcpy(tokens, table.rows, token_count * sizeof(VkRdbmsTokenRow));
            }
            fclose(f_meta);
        }
        
        FILE *f_edges = fopen("/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/assets/swap_edges.dat.bin", "rb");
        if (f_edges) {
            uint32_t count = 0;
            if (fread(&count, sizeof(count), 1, f_edges) == 1) {
                edge_count = count > 1024 ? 1024 : count;
                size_t nr = fread(edges, sizeof(VkSwapEdge), edge_count, f_edges);
                (void)nr;
            }
            fclose(f_edges);
        }
        
        int cx = sb->width / 2;
        int cy = 120 + (sb->height - 120) / 2;
        int r = (sb->height - 120) / 3;
        if (r < 100) r = 100;
        
        for (int i = 0; i < token_count; i++) {
            bool in_sol = false;
            for (int k = 0; k < sol_addr_count; k++) {
                if (strcasecmp(tokens[i].address, sol_addresses[k]) == 0) {
                    in_sol = true;
                    break;
                }
            }
            
            if (in_sol && render_nodes_count < 512) {
                RenderNode *rn = &render_nodes[render_nodes_count++];
                strcpy(rn->address, tokens[i].address);
                strncpy(rn->symbol, tokens[i].symbol, 15);
                rn->symbol[15] = '\0';
                strncpy(rn->name, tokens[i].name, 63);
                rn->name[63] = '\0';
                
                float angle = (float)render_nodes_count * (2.0f * 3.14159f / 40.0f);
                rn->x = cx + r * cosf(angle) + (rand() % 40 - 20);
                rn->y = cy + r * sinf(angle) + (rand() % 40 - 20);
                rn->vx = rn->vy = 0;
                
                if (strcasecmp(rn->address, "0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6") == 0) {
                    rn->color = 0xFFFF0055; // Atropa (Pink/Red)
                } else if (strcasecmp(rn->address, "0x2556f7f8d82ebcdd7b821b0981c38d9da9439cdd") == 0) {
                    rn->color = 0xFF00F2FE; // dOWN (Cyan)
                } else {
                    rn->color = 0xFF4FACFE; // Blue
                }
            }
        }
        loaded = true;
    }
    
    int play_y_min = 120;
    int play_y_max = sb->height - 20;
    int play_x_min = 20;
    int play_x_max = sb->width - 20;
    int cx = sb->width / 2;
    int cy = 120 + (sb->height - 120) / 2;
    
    if (s->mouse_down) {
        if (dragged_node_idx == -1) {
            float best_dist = 30.0f;
            for (int i = 0; i < render_nodes_count; i++) {
                float dx = s->mouse_x - render_nodes[i].x;
                float dy = s->mouse_y - render_nodes[i].y;
                float dist = sqrtf(dx*dx + dy*dy);
                if (dist < best_dist) {
                    best_dist = dist;
                    dragged_node_idx = i;
                }
            }
        }
        if (dragged_node_idx != -1) {
            render_nodes[dragged_node_idx].x = s->mouse_x;
            render_nodes[dragged_node_idx].y = s->mouse_y;
            render_nodes[dragged_node_idx].vx = 0;
            render_nodes[dragged_node_idx].vy = 0;
        }
    } else {
        dragged_node_idx = -1;
    }
    
    // Repulsion
    for (int i = 0; i < render_nodes_count; i++) {
        RenderNode *rn1 = &render_nodes[i];
        for (int j = i + 1; j < render_nodes_count; j++) {
            RenderNode *rn2 = &render_nodes[j];
            float dx = rn2->x - rn1->x;
            float dy = rn2->y - rn1->y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 0.1f) dist = 0.1f;
            if (dist < 120.0f) {
                float force = (120.0f - dist) * 0.015f;
                float fx = (dx / dist) * force;
                float fy = (dy / dist) * force;
                if (i != dragged_node_idx) { rn1->vx -= fx; rn1->vy -= fy; }
                if (j != dragged_node_idx) { rn2->vx += fx; rn2->vy += fy; }
            }
        }
    }
    
    // Attraction
    for (int j = 0; j < edge_count; j++) {
        int src_idx = -1, tgt_idx = -1;
        for (int i = 0; i < render_nodes_count; i++) {
            if (strcasecmp(render_nodes[i].address, edges[j].token0) == 0) src_idx = i;
            if (strcasecmp(render_nodes[i].address, edges[j].token1) == 0) tgt_idx = i;
        }
        if (src_idx != -1 && tgt_idx != -1) {
            RenderNode *rn1 = &render_nodes[src_idx];
            RenderNode *rn2 = &render_nodes[tgt_idx];
            float dx = rn2->x - rn1->x;
            float dy = rn2->y - rn1->y;
            float dist = sqrtf(dx*dx + dy*dy);
            if (dist < 0.1f) dist = 0.1f;
            float desired = 80.0f;
            if (dist > desired) {
                float force = (dist - desired) * 0.004f;
                float fx = (dx / dist) * force;
                float fy = (dy / dist) * force;
                if (src_idx != dragged_node_idx) { rn1->vx += fx; rn1->vy += fy; }
                if (tgt_idx != dragged_node_idx) { rn2->vx -= fx; rn2->vy -= fy; }
            }
        }
    }
    
    // Limits
    for (int i = 0; i < render_nodes_count; i++) {
        RenderNode *rn = &render_nodes[i];
        if (i != dragged_node_idx) {
            rn->vx -= (rn->x - cx) * 0.002f;
            rn->vy -= (rn->y - cy) * 0.002f;
            
            rn->x += rn->vx;
            rn->y += rn->vy;
            rn->vx *= 0.80f;
            rn->vy *= 0.80f;
            
            if (rn->x < play_x_min) rn->x = play_x_min;
            if (rn->x > play_x_max) rn->x = play_x_max;
            if (rn->y < play_y_min) rn->y = play_y_min;
            if (rn->y > play_y_max) rn->y = play_y_max;
        }
    }
    
    // Draw links
    for (int j = 0; j < edge_count; j++) {
        int src_idx = -1, tgt_idx = -1;
        for (int i = 0; i < render_nodes_count; i++) {
            if (strcasecmp(render_nodes[i].address, edges[j].token0) == 0) src_idx = i;
            if (strcasecmp(render_nodes[i].address, edges[j].token1) == 0) tgt_idx = i;
        }
        if (src_idx != -1 && tgt_idx != -1) {
            staging_draw_line(sb, render_nodes[src_idx].x, render_nodes[src_idx].y,
                              render_nodes[tgt_idx].x, render_nodes[tgt_idx].y, 0xFF555555);
        }
    }
    
    // Draw circles
    for (int i = 0; i < render_nodes_count; i++) {
        RenderNode *rn = &render_nodes[i];
        int r = (strcasecmp(rn->address, "0xcc78a0acdf847a2c1714d2a925bb4477df5d48a6") == 0) ? 14 : 9;
        staging_draw_circle(sb, rn->x, rn->y, r, rn->color);
        
        draw_debug_text(sb, rn->x - (strlen(rn->symbol)*4), rn->y - r - 12, rn->symbol, 0xFFFFFFFF, true);
    }
    
    // Selection HUD details
    int active_idx = dragged_node_idx;
    if (active_idx == -1) {
        for (int i = 0; i < render_nodes_count; i++) {
            float dx = s->mouse_x - render_nodes[i].x;
            float dy = s->mouse_y - render_nodes[i].y;
            if (sqrtf(dx*dx + dy*dy) < 15.0f) {
                active_idx = i;
                break;
            }
        }
    }
    
    if (active_idx != -1) {
        char buf[256];
        snprintf(buf, sizeof(buf), "NODE : %s (%s)", render_nodes[active_idx].symbol, render_nodes[active_idx].name);
        draw_debug_text(sb, 20, 105, buf, 0xFF00FF00, true);
        snprintf(buf, sizeof(buf), "ADDR : %s", render_nodes[active_idx].address);
        draw_debug_text(sb, 20, 120, buf, 0xFFCCCCCC, true);
    }
}
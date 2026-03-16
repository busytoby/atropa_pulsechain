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
    printf("[VULKAN] Recreating Swapchain...\n");
    // TODO: cleanup old and re-init
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
}

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

    if (vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &s->staging_vk_buffer) != VK_SUCCESS) return false;

    VkMemoryRequirements memReqs;
    vk->vkGetBufferMemoryRequirements(vk->device, s->staging_vk_buffer, &memReqs);

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

    if (vk->vkAllocateMemory(vk->device, &allocInfo, NULL, &s->staging_vk_memory) != VK_SUCCESS) return false;
    vk->vkBindBufferMemory(vk->device, s->staging_vk_buffer, s->staging_vk_memory, 0);

    if (vk->vkGetMemoryFdKHR) {
        VkMemoryGetFdInfoKHR fdInfo = {
            .sType = VK_STRUCTURE_TYPE_MEMORY_GET_FD_INFO_KHR,
            .pNext = NULL,
            .memory = s->staging_vk_memory,
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

    // 1. Copy CPU data to Staging VkBuffer
    void* mapped;
    vk->vkMapMemory(vk->device, s->staging_vk_memory, 0, sb->size, 0, &mapped);
    memcpy(mapped, sb->data, sb->size);
    vk->vkUnmapMemory(vk->device, s->staging_vk_memory);

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
        .imageExtent = {(uint32_t)sb->width, (uint32_t)sb->height, 1}
    };
    vk->vkCmdCopyBufferToImage(cmd, s->staging_vk_buffer, target, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

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

    vk->vkWaitForFences(vk->device, 1, &vk->inFlightFences[vk->currentFrame], VK_TRUE, UINT64_MAX);
    vk->vkResetFences(vk->device, 1, &vk->inFlightFences[vk->currentFrame]);

    uint32_t imageIndex;
    VkResult res = vk->vkAcquireNextImageKHR(vk->device, vk->swapchain, UINT64_MAX, vk->imageAvailableSemaphores[vk->currentFrame], VK_NULL_HANDLE, &imageIndex);
    
    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain(s);
        return;
    }

    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vk->vkBeginCommandBuffer(vk->command_buffers[vk->currentFrame], &beginInfo);

    // Update UI Elements (CPU Side)
    draw_ui_elements(s);


    // Apply High-Quality Post-Processing
    if (tsfi_bloom_thunk) {
        tsfi_bloom_thunk((uint32_t*)s->paint_buffer->data, s->paint_buffer->width, s->paint_buffer->height, 0.85f, 0.4f);
    }


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

    vk->vkQueuePresentKHR(vk->queue, &presentInfo);

    vk->currentFrame = (vk->currentFrame + 1) % 3;
}
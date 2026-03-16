#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "window_inc/vulkan_struct.h"
#include "tsfi_wave_any.h"
#include "lau_memory.h"

// Function declared in plugins/vulkan_init.c
VulkanContext *init_vulkan(void);
void cleanup_vulkan(VulkanContext *vk);
void tag_vulkan_object(VulkanContext *vk, uint64_t handle, VkObjectType type, const char *name);

int main() {
    printf("=== TSFi WAVE(Any) Sync Validation ===\n");
    
    VulkanContext *vk = init_vulkan();
    if (!vk) return 1;

    // Test a range of wave lengths (1 to 128)
    // We want to verify that synchronization objects are correctly managed 
    // for small, irregular batches that might be merged or split.
    
    int wave_lengths[] = {1, 15, 16, 32, 64, 127, 128};
    int num_tests = sizeof(wave_lengths) / sizeof(int);

    for (int i = 0; i < num_tests; i++) {
        int count = wave_lengths[i];
        printf("[SYNC] Testing WAVE(%d)...\n", count);
        
        // 1. Create Stream
        WaveStream ws = {0};
        ws.count = count;
        ws.atom_size = 15;
        ws.stride = 16;
        
        // 2. Assign Sync Objects
        VkFenceCreateInfo fenceInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        VkSemaphoreTypeCreateInfo timelineInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
            .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
            .initialValue = 0
        };
        VkSemaphoreCreateInfo semInfo = {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = &timelineInfo
        };
        
        if (vk->vkCreateFence(vk->device, &fenceInfo, NULL, &ws.fence) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create fence\n");
            continue;
        }
        if (vk->vkCreateSemaphore(vk->device, &semInfo, NULL, &ws.timeline) != VK_SUCCESS) {
            fprintf(stderr, "Failed to create timeline semaphore\n");
            continue;
        }
        ws.timeline_val = 1;

        // Tag objects for validation
        char fence_name[64], sem_name[64];
        snprintf(fence_name, 64, "Wave_%d_Fence", count);
        snprintf(sem_name, 64, "Wave_%d_Timeline", count);
        tag_vulkan_object(vk, (uint64_t)ws.fence, VK_OBJECT_TYPE_FENCE, fence_name);
        tag_vulkan_object(vk, (uint64_t)ws.timeline, VK_OBJECT_TYPE_SEMAPHORE, sem_name);

        // 3. Simulate GPU Work Submission (Signal Fence)
        // We aren't running a shader here, just verifying the object lifecycle and validation
        // layer tracking of these per-wave objects.
        
        // Submit a dummy signal operation to the timeline
        VkTimelineSemaphoreSubmitInfo timelineSubmit = {
            .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
            .signalSemaphoreValueCount = 1,
            .pSignalSemaphoreValues = &ws.timeline_val
        };
        VkSubmitInfo submitInfo = {
            .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .pNext = &timelineSubmit,
            .signalSemaphoreCount = 1,
            .pSignalSemaphores = &ws.timeline
        };
        
        if (vk->vkQueueSubmit(vk->queue, 1, &submitInfo, ws.fence) != VK_SUCCESS) {
             fprintf(stderr, "Queue Submit failed\n");
        }
        
        // 4. Wait
        vk->vkWaitForFences(vk->device, 1, &ws.fence, VK_TRUE, 1000000000);
        
        // 5. Cleanup (Crucial for Validation Layer check)
        vk->vkDestroyFence(vk->device, ws.fence, NULL);
        vk->vkDestroySemaphore(vk->device, ws.timeline, NULL);
    }

    cleanup_vulkan(vk);
    printf("=== Sync Validation Complete ===\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
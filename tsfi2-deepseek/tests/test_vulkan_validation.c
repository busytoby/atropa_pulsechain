#include "window_inc/vulkan_struct.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void test_first_error_rule(VulkanContext *vk) {
    printf("\n=== TSFi First Error Rule Validation ===\n");
    
    VkBufferCreateInfo badInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 0,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
    };
    VkBuffer badBuffer;

    printf("[TEST] Attempting forbidden operation (Size 0 buffer)...\n");
    printf("[TEST] Expecting immediate SIGABRT via First Error Rule.\n");
    fflush(stdout);

    pid_t pid = fork();
    if (pid == 0) {
        vk->vkCreateBuffer(vk->device, &badInfo, NULL, &badBuffer);
        exit(0);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFSIGNALED(status) && WTERMSIG(status) == SIGABRT) {
            printf("[PASS] First Error Rule cleanly intercepted the fault and aborted the child environment.\n");
        } else {
            printf("\033[31m[FAIL] First Error Rule VIOLATED: Process did not terminate as expected.\033[0m\n");
        }
    } else {
        printf("[FAIL] Fork failed.\n");
    }
}


void test_vulkan_robustness(VulkanContext *vk) {
    printf("\n=== TSFi Vulkan: Robustness 2 Validation ===\n");
    
    VkPhysicalDeviceRobustness2FeaturesEXT robust2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT
    };
    VkPhysicalDeviceFeatures2 features2 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &robust2
    };

    vk->vkGetPhysicalDeviceFeatures2(vk->physical_device, &features2);

    printf("[ROBUSTNESS] robustBufferAccess2: %s\n", robust2.robustBufferAccess2 ? "ACTIVE" : "MISSING");
    printf("[ROBUSTNESS] robustImageAccess2:  %s\n", robust2.robustImageAccess2 ? "ACTIVE" : "MISSING");
    printf("[ROBUSTNESS] nullDescriptor:      %s\n", robust2.nullDescriptor ? "ACTIVE" : "MISSING");

    if (robust2.robustBufferAccess2 && robust2.nullDescriptor) {
        printf("[PASS] Physical Substrate is Robust.\n");
    } else {
        printf("[FAIL] Robustness 2 Requirements Not Met.\n");
    }
}


// Function declared in plugins/vulkan_init.c
VulkanContext *init_vulkan(void);
void cleanup_vulkan(VulkanContext *vk);
void tag_vulkan_object(VulkanContext *vk, uint64_t handle, VkObjectType type, const char *name);

int main() {
    printf("=== TSFi Vulkan Validation Messenger Test ===\n");
    
    VulkanContext *vk = init_vulkan();
    if (!vk) {
        fprintf(stderr, "[FAIL] Failed to initialize Vulkan context.\n");
        return 1;
    }

    if (vk->debugMessenger) {
        printf("[PASS] Vulkan Debug Messenger successfully initialized.\n");
    } else {
        printf("[FAIL] Vulkan Debug Messenger NOT initialized.\n");
    }

    if (vk->vkCreateBuffer) {
        VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .size = 1024,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT
        };
        VkBuffer buffer;
        printf("[VRS] Creating valid test buffer...\n");
        if (vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &buffer) == VK_SUCCESS) {
            tag_vulkan_object(vk, (uint64_t)buffer, VK_OBJECT_TYPE_BUFFER, "Validation_Probe_Buffer");
            printf("[VRS] Buffer tagged as 'Validation_Probe_Buffer'.\n");
            vk->vkDestroyBuffer(vk->device, buffer, NULL);
        }
    }

    test_vulkan_robustness(vk);
    test_first_error_rule(vk);

    cleanup_vulkan(vk);
    printf("\n=== Validation Suite Complete ===\n");
    
    extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
#include "window_inc/vulkan_struct.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lau_memory.h"

// Macros for loading
#define LOAD_PROC(name) \
    vk->name = (PFN_##name)vk->vkGetInstanceProcAddr(vk->instance, #name); \
    if (!vk->name) { printf("[VULKAN] Failed to load %s\n", #name); return NULL; }

// Device Proc Macro
#define LOAD_DEV_PROC(name) \
    vk->name = (PFN_##name)vk->vkGetDeviceProcAddr(vk->device, #name); \
    if (!vk->name) { printf("[VULKAN] Failed to load device proc %s\n", #name); cleanup_vulkan(vk); return NULL; }

#define LOAD_DEV_PROC_OPTIONAL(name) \
    vk->name = (PFN_##name)vk->vkGetDeviceProcAddr(vk->device, #name); \
    if (!vk->name) { printf("[VULKAN] Optional proc %s not loaded.\n", #name); }

#define LOAD_DEV_PROC_FALLBACK(name, fallback_name) \
    vk->name = (PFN_##name)vk->vkGetDeviceProcAddr(vk->device, #name); \
    if (!vk->name) { \
        vk->name = (PFN_##name)vk->vkGetDeviceProcAddr(vk->device, #fallback_name); \
    } \
    if (!vk->name) { printf("[VULKAN] Failed to load %s or %s\n", #name, #fallback_name); cleanup_vulkan(vk); return NULL; }

// --- TSFi Liang-Barsky (Lau) Vulkan Allocation Bridge ---

void tag_vulkan_object(VulkanContext *vk, uint64_t handle, VkObjectType type, const char *name);

static void* VKAPI_CALL tsfi_vulkan_alloc(void* pUserData, size_t size, size_t alignment, VkSystemAllocationScope scope) {
    (void)pUserData; (void)scope;
    // We use lau_memalign to ensure driver memory is optimized for AVX-512 thunks.
    // Default alignment is 64 if not specified higher by the driver.
    return lau_memalign_loc(alignment > 64 ? alignment : 64, size, "VULKAN_DRIVER", 0);
}

static void* VKAPI_CALL tsfi_vulkan_realloc(void* pUserData, void* pOriginal, size_t size, size_t alignment, VkSystemAllocationScope scope) {
    (void)pUserData; (void)scope; (void)alignment;
    return lau_realloc_loc(pOriginal, size, "VULKAN_DRIVER", 0);
}

static void VKAPI_CALL tsfi_vulkan_free(void* pUserData, void* pMemory) {
    (void)pUserData;
    if (pMemory) lau_free(pMemory);
}

static void VKAPI_CALL tsfi_vulkan_internal_alloc_notify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope) {
    (void)pUserData; (void)size; (void)allocationType; (void)scope;
}

static void VKAPI_CALL tsfi_vulkan_internal_free_notify(void* pUserData, size_t size, VkInternalAllocationType allocationType, VkSystemAllocationScope scope) {
    (void)pUserData; (void)size; (void)allocationType; (void)scope;
}

const VkAllocationCallbacks tsfi_alloc_callbacks = {
    .pUserData = NULL,
    .pfnAllocation = tsfi_vulkan_alloc,
    .pfnReallocation = tsfi_vulkan_realloc,
    .pfnFree = tsfi_vulkan_free,
    .pfnInternalAllocation = tsfi_vulkan_internal_alloc_notify,
    .pfnInternalFree = tsfi_vulkan_internal_free_notify
};

void lau_init_sync_objects(VulkanContext *vk);
void lau_init_samplers(VulkanContext *vk);
void lau_init_stuffed_manifold(VulkanContext *vk);
void lau_init_queries(VulkanContext *vk);

void lau_init_samplers(VulkanContext *vk) {
    if (!vk || !vk->device || !vk->vkCreateSampler) return;

    VkSamplerCreateInfo samplerInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
        .anisotropyEnable = VK_FALSE,
        .maxAnisotropy = 1.0f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .mipLodBias = 0.0f,
        .minLod = 0.0f,
        .maxLod = 0.0f
    };

    // 1. Point Sampler
    vk->vkCreateSampler(vk->device, &samplerInfo, &tsfi_alloc_callbacks, &vk->sampler_point);
    tag_vulkan_object(vk, (uint64_t)vk->sampler_point, VK_OBJECT_TYPE_SAMPLER, "TSFi_Sampler_Point");

    // 2. Linear Sampler
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    vk->vkCreateSampler(vk->device, &samplerInfo, &tsfi_alloc_callbacks, &vk->sampler_linear);
    tag_vulkan_object(vk, (uint64_t)vk->sampler_linear, VK_OBJECT_TYPE_SAMPLER, "TSFi_Sampler_Linear");

    // 3. Anisotropic Sampler (Highest Quality)
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    vk->vkCreateSampler(vk->device, &samplerInfo, &tsfi_alloc_callbacks, &vk->sampler_aniso);
    tag_vulkan_object(vk, (uint64_t)vk->sampler_aniso, VK_OBJECT_TYPE_SAMPLER, "TSFi_Sampler_Aniso_16x");

    // --- SAMPLER DESCRIPTOR SET INITIALIZATION ---
    
    // 1. Descriptor Set Layout
    VkDescriptorSetLayoutBinding samplerBindings[3] = {
        { .binding = 0, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL },
        { .binding = 1, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL },
        { .binding = 2, .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1, .stageFlags = VK_SHADER_STAGE_ALL }
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 3,
        .pBindings = samplerBindings
    };

    vk->vkCreateDescriptorSetLayout(vk->device, &layoutInfo, &tsfi_alloc_callbacks, &vk->sampler_descriptor_layout);
    tag_vulkan_object(vk, (uint64_t)vk->sampler_descriptor_layout, VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, "TSFi_Sampler_Layout");

    // 2. Descriptor Pool
    VkDescriptorPoolSize poolSize = { .type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 3 };
    VkDescriptorPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = 1,
        .poolSizeCount = 1,
        .pPoolSizes = &poolSize
    };

    vk->vkCreateDescriptorPool(vk->device, &poolInfo, &tsfi_alloc_callbacks, &vk->sampler_descriptor_pool);
    tag_vulkan_object(vk, (uint64_t)vk->sampler_descriptor_pool, VK_OBJECT_TYPE_DESCRIPTOR_POOL, "TSFi_Sampler_Pool");

    // 3. Allocate Descriptor Set
    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = vk->sampler_descriptor_pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &vk->sampler_descriptor_layout
    };

    vk->vkAllocateDescriptorSets(vk->device, &allocInfo, &vk->sampler_descriptor_set);
    tag_vulkan_object(vk, (uint64_t)vk->sampler_descriptor_set, VK_OBJECT_TYPE_DESCRIPTOR_SET, "TSFi_Sampler_Set");

    // 4. Update Descriptor Set with Samplers
    VkDescriptorImageInfo samplerInfos[3] = {
        { .sampler = vk->sampler_point },
        { .sampler = vk->sampler_linear },
        { .sampler = vk->sampler_aniso }
    };

    VkWriteDescriptorSet samplerWrites[1] = {
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = vk->sampler_descriptor_set,
            .dstBinding = 0,
            .descriptorCount = 3,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .pImageInfo = samplerInfos
        }
    };

    vk->vkUpdateDescriptorSets(vk->device, 1, samplerWrites, 0, NULL);
    printf("[VULKAN] Global Sampler Manifold Formalized (Descriptor Set Active).\n");
}

void lau_init_stuffed_manifold(VulkanContext *vk) {
    if (!vk || !vk->device || !vk->vkCreateBufferView || !vk->rebar_buffer) return;

    // 1. STUFFED FUR VIEW (Isotropic Density)
    // Format: R8_UNORM (8-bit quantized density for 171 FPS efficiency)
    VkBufferViewCreateInfo furViewInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
        .buffer = vk->rebar_buffer,
        .format = VK_FORMAT_R8_UNORM,
        .offset = 0, // Starts at the beginning of ReBAR
        .range = 256 * 1024 * 1024 // 256MB for Fur Density Field
    };
    vk->vkCreateBufferView(vk->device, &furViewInfo, &tsfi_alloc_callbacks, &vk->stuffed_fur_view);
    tag_vulkan_object(vk, (uint64_t)vk->stuffed_fur_view, VK_OBJECT_TYPE_BUFFER_VIEW, "TSFi_Stuffed_Fur_Density_View");

    // 2. STUFFED FEATHER VIEW (Anisotropic Tensors)
    // Format: A2B10G10R10_UNORM_PACK32 (High-precision Tangents for iridescent sheen)
    VkBufferViewCreateInfo featherViewInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO,
        .buffer = vk->rebar_buffer,
        .format = VK_FORMAT_A2B10G10R10_UNORM_PACK32,
        .offset = 256 * 1024 * 1024, // Starts after fur
        .range = 256 * 1024 * 1024 // 256MB for Feather Tensor Field
    };
    vk->vkCreateBufferView(vk->device, &featherViewInfo, &tsfi_alloc_callbacks, &vk->stuffed_feather_view);
    tag_vulkan_object(vk, (uint64_t)vk->stuffed_feather_view, VK_OBJECT_TYPE_BUFFER_VIEW, "TSFi_Stuffed_Feather_Tensor_View");

    printf("[VULKAN] Stuffed Animal Texel Manifold Initialized (Fur + Feathers).\n");
}

void lau_init_queries(VulkanContext *vk) {
    if (!vk || !vk->device || !vk->vkCreateQueryPool) return;

    // 1. Performance & Timestamp Pool (64 slots)
    VkQueryPoolCreateInfo perfInfo = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_TIMESTAMP,
        .queryCount = 64
    };
    vk->vkCreateQueryPool(vk->device, &perfInfo, &tsfi_alloc_callbacks, &vk->query_pool_perf);
    tag_vulkan_object(vk, (uint64_t)vk->query_pool_perf, VK_OBJECT_TYPE_QUERY_POOL, "TSFi_Query_Pool_Perf");

    // 2. Pipeline Statistics Pool
    VkQueryPoolCreateInfo statsInfo = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS,
        .queryCount = 1,
        .pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT | 
                              VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT
    };
    vk->vkCreateQueryPool(vk->device, &statsInfo, &tsfi_alloc_callbacks, &vk->query_pool_stats);
    tag_vulkan_object(vk, (uint64_t)vk->query_pool_stats, VK_OBJECT_TYPE_QUERY_POOL, "TSFi_Query_Pool_Stats");

    // Calculate ReBAR address for Query direct write (last 4KB of 1GB pool)
    vk->query_manifold_address = vk->rebar_device_address + (vk->rebar_pool_size - 4096);
    
    printf("[VULKAN] Query Manifold Formalized. Anchor: 0x%lx\n", (unsigned long)vk->query_manifold_address);
}

void cleanup_vulkan(VulkanContext *vk) {
    if (!vk) return;
    if (vk->device) {
        if (vk->vkDeviceWaitIdle) vk->vkDeviceWaitIdle(vk->device);
        
        // Query Cleanup
        if (vk->query_pool_perf) vk->vkDestroyQueryPool(vk->device, vk->query_pool_perf, &tsfi_alloc_callbacks);
        if (vk->query_pool_stats) vk->vkDestroyQueryPool(vk->device, vk->query_pool_stats, &tsfi_alloc_callbacks);
        if (vk->query_pool_ray) vk->vkDestroyQueryPool(vk->device, vk->query_pool_ray, &tsfi_alloc_callbacks);

        // ReBAR Cleanup
        if (vk->rebar_mapped_ptr && vk->vkUnmapMemory) vk->vkUnmapMemory(vk->device, vk->rebar_memory);
        if (vk->rebar_buffer && vk->vkDestroyBuffer) vk->vkDestroyBuffer(vk->device, vk->rebar_buffer, &tsfi_alloc_callbacks);
        if (vk->rebar_memory && vk->vkFreeMemory) vk->vkFreeMemory(vk->device, vk->rebar_memory, &tsfi_alloc_callbacks);

        // Stuffed Manifold Cleanup
        if (vk->stuffed_fur_view) vk->vkDestroyBufferView(vk->device, vk->stuffed_fur_view, &tsfi_alloc_callbacks);
        if (vk->stuffed_feather_view) vk->vkDestroyBufferView(vk->device, vk->stuffed_feather_view, &tsfi_alloc_callbacks);

        // Sampler Cleanup
        if (vk->sampler_descriptor_layout) vk->vkDestroyDescriptorSetLayout(vk->device, vk->sampler_descriptor_layout, &tsfi_alloc_callbacks);
        if (vk->sampler_descriptor_pool) vk->vkDestroyDescriptorPool(vk->device, vk->sampler_descriptor_pool, &tsfi_alloc_callbacks);
        if (vk->sampler_point) vk->vkDestroySampler(vk->device, vk->sampler_point, &tsfi_alloc_callbacks);
        if (vk->sampler_linear) vk->vkDestroySampler(vk->device, vk->sampler_linear, &tsfi_alloc_callbacks);
        if (vk->sampler_aniso) vk->vkDestroySampler(vk->device, vk->sampler_aniso, &tsfi_alloc_callbacks);

        if (vk->command_pool) vk->vkDestroyCommandPool(vk->device, vk->command_pool, &tsfi_alloc_callbacks);
        if (vk->swapchainImageViews) {
            if (vk->swapchainImageCount > 0 && vk->swapchainImageCount < 16) {
                for (uint32_t i = 0; i < vk->swapchainImageCount; i++) {
                    if (vk->swapchainImageViews[i] && vk->vkDestroyImageView) {
                        vk->vkDestroyImageView(vk->device, vk->swapchainImageViews[i], NULL);
                    }
                }
            }
            vk->swapchainImageViews = NULL;
        }
        if (vk->swapchainImages) {
            vk->swapchainImages = NULL;
        }
        if (vk->swapchain) vk->vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
        for (int i = 0; i < 3; i++) {
            if (vk->imageAvailableSemaphores[i]) vk->vkDestroySemaphore(vk->device, vk->imageAvailableSemaphores[i], NULL);
            if (vk->inFlightFences[i]) vk->vkDestroyFence(vk->device, vk->inFlightFences[i], NULL);
        }
        for (int i = 0; i < 8; i++) {
            if (vk->renderFinishedSemaphores[i]) vk->vkDestroySemaphore(vk->device, vk->renderFinishedSemaphores[i], NULL);
        }
        if (vk->timelineSemaphore) vk->vkDestroySemaphore(vk->device, vk->timelineSemaphore, NULL);
        vk->vkDestroyDevice(vk->device, NULL);
    }
    if (vk->debugMessenger && vk->vkDestroyDebugUtilsMessengerEXT) {
        vk->vkDestroyDebugUtilsMessengerEXT(vk->instance, vk->debugMessenger, NULL);
    }
    if (vk->surface) vk->vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    if (vk->instance) vk->vkDestroyInstance(vk->instance, NULL);
    if (vk->lib_handle) dlclose(vk->lib_handle);
    lau_free(vk);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    (void)messageType; (void)pUserData;
    
    const char* prefix = "[VULKAN VALIDATION]";
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        fprintf(stderr, "\033[31m%s FATAL ERROR: %s\033[0m\n", prefix, pCallbackData->pMessage);
        fprintf(stderr, "[TSFI] First Error Rule Violated. Terminating Simulation.\n");
        fflush(stderr);
        abort(); // Zero-Tolerance Enforcement
    } else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        fprintf(stderr, "\033[33m%s WARNING: %s\033[0m\n", prefix, pCallbackData->pMessage);
    }
    
    return VK_FALSE;
}

VulkanContext *init_vulkan() {
    printf("[VULKAN] Initializing Hybrid Context (API 1.4)...\n"); fflush(stdout);
    VulkanContext *vk = (VulkanContext *)lau_memalign(512, sizeof(VulkanContext));
    if (!vk) return NULL;
    memset(vk, 0, sizeof(VulkanContext));

    vk->lib_handle = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!vk->lib_handle) { printf("[VULKAN] Failed to load libvulkan.so.1\n"); lau_free(vk); return NULL; }

    vk->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vk->lib_handle, "vkGetInstanceProcAddr");
    if (!vk->vkGetInstanceProcAddr) { cleanup_vulkan(vk); return NULL; }
    
    PFN_vkCreateInstance createInstance = (PFN_vkCreateInstance)vk->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    vk->vkCreateInstance = createInstance;

    VkApplicationInfo appInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_4 };
    const char *extensions[] = { 
        "VK_KHR_external_memory_capabilities", 
        "VK_KHR_get_physical_device_properties2", 
        "VK_KHR_surface", 
        "VK_KHR_wayland_surface",
        "VK_EXT_debug_utils" 
    };
    const char *layers[] = { "VK_LAYER_KHRONOS_validation" };
    
    bool use_validation = true; 
    const char* env_val = getenv("TSFI_VALIDATION");
    if (env_val && strcmp(env_val, "0") == 0) use_validation = false;

    bool use_gpu_av = (getenv("TSFI_GPU_AV") != NULL && strcmp(getenv("TSFI_GPU_AV"), "1") == 0);

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback
    };

    VkValidationFeatureEnableEXT enabledFeatures[4];
    uint32_t enabledCount = 0;
    enabledFeatures[enabledCount++] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
    if (use_gpu_av) {
        enabledFeatures[enabledCount++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
        enabledFeatures[enabledCount++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT;
    } else {
        enabledFeatures[enabledCount++] = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;
    }

    VkValidationFeaturesEXT validationFeatures = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .pNext = &debugCreateInfo,
        .enabledValidationFeatureCount = enabledCount,
        .pEnabledValidationFeatures = enabledFeatures
    };

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = use_validation ? &validationFeatures : NULL,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = 5,
        .ppEnabledExtensionNames = extensions,
        .enabledLayerCount = use_validation ? 1 : 0,
        .ppEnabledLayerNames = use_validation ? layers : NULL
    };

    if (vk->vkCreateInstance(&createInfo, &tsfi_alloc_callbacks, &vk->instance) != VK_SUCCESS) { cleanup_vulkan(vk); return NULL; }

    LOAD_PROC(vkDestroyInstance);
    LOAD_PROC(vkEnumeratePhysicalDevices);
    LOAD_PROC(vkEnumerateDeviceExtensionProperties);
    LOAD_PROC(vkGetPhysicalDeviceProperties);
    LOAD_PROC(vkGetPhysicalDeviceFeatures);
    LOAD_PROC(vkGetPhysicalDeviceFeatures2);
    LOAD_PROC(vkGetPhysicalDeviceQueueFamilyProperties);
    LOAD_PROC(vkCreateDevice);
    LOAD_PROC(vkDestroyDevice);
    LOAD_PROC(vkGetDeviceQueue);
    LOAD_PROC(vkGetDeviceProcAddr);
    LOAD_PROC(vkGetPhysicalDeviceMemoryProperties);
    LOAD_PROC(vkCreateWaylandSurfaceKHR);
    LOAD_PROC(vkDestroySurfaceKHR);
    LOAD_PROC(vkGetPhysicalDeviceSurfaceSupportKHR);
    LOAD_PROC(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    LOAD_PROC(vkGetPhysicalDeviceSurfaceFormatsKHR);
    LOAD_PROC(vkGetPhysicalDeviceSurfacePresentModesKHR);
    
    vk->vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vk->vkGetInstanceProcAddr(vk->instance, "vkCreateDebugUtilsMessengerEXT");
    vk->vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vk->vkGetInstanceProcAddr(vk->instance, "vkDestroyDebugUtilsMessengerEXT");

    if (vk->vkCreateDebugUtilsMessengerEXT && use_validation) {
        vk->vkCreateDebugUtilsMessengerEXT(vk->instance, &debugCreateInfo, &tsfi_alloc_callbacks, &vk->debugMessenger);
        printf("[VULKAN] Debug Messenger Initialized.\n");
    }

    uint32_t dev_count = 0;
    vk->vkEnumeratePhysicalDevices(vk->instance, &dev_count, NULL);
    if (dev_count == 0) { cleanup_vulkan(vk); return NULL; }
    VkPhysicalDevice devices[4];
    vk->vkEnumeratePhysicalDevices(vk->instance, &dev_count, devices);
    vk->physical_device = devices[0];
    vk->vkGetPhysicalDeviceMemoryProperties(vk->physical_device, &vk->memory_properties);
    
    uint32_t qCount = 0;
    vk->vkGetPhysicalDeviceQueueFamilyProperties(vk->physical_device, &qCount, NULL);
    VkQueueFamilyProperties *qProps = (VkQueueFamilyProperties*)lau_malloc(sizeof(VkQueueFamilyProperties) * qCount);
    vk->vkGetPhysicalDeviceQueueFamilyProperties(vk->physical_device, &qCount, qProps);
    int qIndex = -1;
    int vqIndex = -1;
    for (uint32_t i = 0; i < qCount; i++) { 
        if (qProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { qIndex = i; }
        if (qProps[i].queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) { vqIndex = i; }
    }
    lau_free(qProps);
    if (qIndex < 0) { cleanup_vulkan(vk); return NULL; }
    vk->queue_family_index = qIndex;
    
    float priority = 1.0f;
    VkDeviceQueueCreateInfo qInfos[2];
    uint32_t qInfoCount = 0;
    
    qInfos[qInfoCount] = (VkDeviceQueueCreateInfo){ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = (uint32_t)qIndex, .queueCount = 1, .pQueuePriorities = &priority };
    qInfoCount++;

    if (vqIndex >= 0 && vqIndex != qIndex) {
        qInfos[qInfoCount] = (VkDeviceQueueCreateInfo){ .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = (uint32_t)vqIndex, .queueCount = 1, .pQueuePriorities = &priority };
        qInfoCount++;
    }
    
    // Feature Chain
    VkPhysicalDeviceRayQueryFeaturesKHR rayQueryFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, .rayQuery = VK_TRUE };
    VkPhysicalDeviceAccelerationStructureFeaturesKHR asFeatures = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, .pNext = &rayQueryFeatures, .accelerationStructure = VK_TRUE };
    VkPhysicalDeviceVulkan12Features features12 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &asFeatures, .timelineSemaphore = VK_TRUE, .bufferDeviceAddress = VK_TRUE, .descriptorIndexing = VK_TRUE };
    VkPhysicalDeviceRobustness2FeaturesEXT robust2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT, .pNext = &features12, .robustBufferAccess2 = VK_TRUE, .nullDescriptor = VK_TRUE };
    VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR localRead = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR, .pNext = &robust2, .dynamicRenderingLocalRead = VK_TRUE };
    VkPhysicalDeviceVulkan13Features features13 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &localRead, .dynamicRendering = VK_TRUE, .synchronization2 = VK_TRUE, .maintenance4 = VK_TRUE };
    VkPhysicalDeviceFeatures2 features2 = { .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features13, .features = { .robustBufferAccess = VK_TRUE } };
    
    VkDeviceCreateInfo devInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .pNext = &features2, .queueCreateInfoCount = qInfoCount, .pQueueCreateInfos = qInfos };

        const char *fullExts[] = { 
            "VK_KHR_external_memory", "VK_KHR_external_memory_fd", "VK_EXT_external_memory_dma_buf", 
            "VK_KHR_swapchain", "VK_EXT_host_image_copy", "VK_KHR_maintenance5", 
            "VK_KHR_dynamic_rendering_local_read", "VK_EXT_global_priority", "VK_EXT_robustness2",
            "VK_KHR_acceleration_structure", "VK_KHR_ray_query", "VK_KHR_deferred_host_operations",
            "VK_EXT_image_drm_format_modifier", "VK_KHR_image_format_list", "VK_KHR_bind_memory2",
            "VK_KHR_video_queue", "VK_KHR_video_encode_queue", "VK_KHR_video_encode_h264"
        };
        
            // We must identify which ones are ACTUALLY supported to avoid vkCreateDevice failure
            uint32_t supportedCount = 0;
            vk->vkEnumerateDeviceExtensionProperties(vk->physical_device, NULL, &supportedCount, NULL);
            VkExtensionProperties *supported = (VkExtensionProperties*)lau_malloc(sizeof(VkExtensionProperties) * supportedCount);
            vk->vkEnumerateDeviceExtensionProperties(vk->physical_device, NULL, &supportedCount, supported);
        
            const char *devEnabledExts[32];
            uint32_t devEnabledCount = 0;
            for (int i = 0; i < 18; i++) {
                for (uint32_t j = 0; j < supportedCount; j++) {
                    if (strcmp(fullExts[i], supported[j].extensionName) == 0) {
                        devEnabledExts[devEnabledCount++] = fullExts[i];
                        break;
                    }
                }
            }
            lau_free(supported);
        
            devInfo.enabledExtensionCount = devEnabledCount;
            devInfo.ppEnabledExtensionNames = devEnabledExts;    
        if (vk->vkCreateDevice(vk->physical_device, &devInfo, &tsfi_alloc_callbacks, &vk->device) != VK_SUCCESS) {
            cleanup_vulkan(vk);
            return NULL;
        }
        
        vk->vkGetDeviceQueue(vk->device, vk->queue_family_index, 0, &vk->queue);
        vk->vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)vk->vkGetDeviceProcAddr(vk->device, "vkSetDebugUtilsObjectNameEXT");
    
        tag_vulkan_object(vk, (uint64_t)vk->device, VK_OBJECT_TYPE_DEVICE, "TSFi_Main_Device");
        tag_vulkan_object(vk, (uint64_t)vk->queue, VK_OBJECT_TYPE_QUEUE, "TSFi_Main_Queue");
    
        // LOAD VIDEO ENCODE PROCS
        LOAD_DEV_PROC_OPTIONAL(vkCreateVideoSessionKHR);
        LOAD_DEV_PROC_OPTIONAL(vkDestroyVideoSessionKHR);
        LOAD_DEV_PROC_OPTIONAL(vkCreateVideoSessionParametersKHR);
        LOAD_DEV_PROC_OPTIONAL(vkDestroyVideoSessionParametersKHR);
        LOAD_DEV_PROC_OPTIONAL(vkGetVideoSessionMemoryRequirementsKHR);
        LOAD_DEV_PROC_OPTIONAL(vkBindVideoSessionMemoryKHR);
        LOAD_DEV_PROC_OPTIONAL(vkCmdBeginVideoCodingKHR);
        LOAD_DEV_PROC_OPTIONAL(vkCmdEndVideoCodingKHR);
        LOAD_DEV_PROC_OPTIONAL(vkCmdEncodeVideoKHR);
        
        LOAD_DEV_PROC(vkCreateCommandPool);
        LOAD_DEV_PROC(vkDestroyCommandPool);
        LOAD_DEV_PROC(vkResetCommandPool);
        LOAD_DEV_PROC(vkAllocateCommandBuffers);
        LOAD_DEV_PROC(vkFreeCommandBuffers);
        LOAD_DEV_PROC(vkBeginCommandBuffer);
        LOAD_DEV_PROC(vkEndCommandBuffer);
        LOAD_DEV_PROC(vkCmdExecuteCommands);
        LOAD_DEV_PROC(vkQueueSubmit);
        LOAD_DEV_PROC(vkQueueWaitIdle);
        LOAD_DEV_PROC(vkDeviceWaitIdle);
        LOAD_DEV_PROC(vkCmdPipelineBarrier);
        LOAD_DEV_PROC(vkCmdClearColorImage);

        LOAD_DEV_PROC_FALLBACK(vkCmdBeginRendering, vkCmdBeginRenderingKHR);
        LOAD_DEV_PROC_FALLBACK(vkCmdEndRendering, vkCmdEndRenderingKHR);
        LOAD_DEV_PROC_FALLBACK(vkCmdPipelineBarrier2, vkCmdPipelineBarrier2KHR);
        LOAD_DEV_PROC(vkCmdDraw);
        LOAD_DEV_PROC(vkCreateImage);
        LOAD_DEV_PROC(vkDestroyImage);
        LOAD_DEV_PROC(vkCreateImageView);
        LOAD_DEV_PROC(vkDestroyImageView);
        LOAD_DEV_PROC(vkCreateSampler);
        LOAD_DEV_PROC(vkDestroySampler);
        LOAD_DEV_PROC(vkAllocateMemory);
        LOAD_DEV_PROC(vkFreeMemory);
        LOAD_DEV_PROC(vkBindImageMemory);
            LOAD_DEV_PROC(vkGetImageMemoryRequirements);
            vk->vkGetImageSubresourceLayout = (PFN_vkGetImageSubresourceLayout)vk->vkGetDeviceProcAddr(vk->device, "vkGetImageSubresourceLayout");
            vk->vkGetMemoryFdKHR = (PFN_vkGetMemoryFdKHR)vk->vkGetDeviceProcAddr(vk->device, "vkGetMemoryFdKHR");
            LOAD_DEV_PROC(vkCreateBuffer);
            LOAD_DEV_PROC(vkDestroyBuffer);
            LOAD_DEV_PROC(vkCreateBufferView);
            LOAD_DEV_PROC(vkDestroyBufferView);
            LOAD_DEV_PROC(vkGetBufferMemoryRequirements);

    LOAD_DEV_PROC(vkBindBufferMemory);
    LOAD_DEV_PROC(vkMapMemory);
    LOAD_DEV_PROC(vkUnmapMemory);
    LOAD_DEV_PROC(vkCmdCopyBufferToImage);

    LOAD_DEV_PROC(vkCreateQueryPool);
    LOAD_DEV_PROC(vkDestroyQueryPool);
    LOAD_DEV_PROC(vkGetQueryPoolResults);
    LOAD_DEV_PROC(vkCmdResetQueryPool);
    LOAD_DEV_PROC(vkCmdWriteTimestamp);
    LOAD_DEV_PROC(vkCmdBeginQuery);
    LOAD_DEV_PROC(vkCmdEndQuery);
    LOAD_DEV_PROC(vkCmdCopyQueryPoolResults);

    LOAD_DEV_PROC(vkCreateSwapchainKHR);
    LOAD_DEV_PROC(vkDestroySwapchainKHR);
    LOAD_DEV_PROC(vkGetSwapchainImagesKHR);
    LOAD_DEV_PROC(vkAcquireNextImageKHR);
    LOAD_DEV_PROC(vkQueuePresentKHR);
    LOAD_DEV_PROC(vkCreateSemaphore);
    LOAD_DEV_PROC(vkDestroySemaphore);
    LOAD_DEV_PROC(vkSignalSemaphore);
    LOAD_DEV_PROC(vkWaitSemaphores);
    LOAD_DEV_PROC(vkGetSemaphoreCounterValue);
    LOAD_DEV_PROC(vkCreateFence);
    LOAD_DEV_PROC(vkDestroyFence);
    LOAD_DEV_PROC(vkWaitForFences);
    LOAD_DEV_PROC(vkResetFences);
    LOAD_DEV_PROC(vkCreateShaderModule);
    LOAD_DEV_PROC(vkDestroyShaderModule);
    LOAD_DEV_PROC(vkCreateDescriptorSetLayout);
    LOAD_DEV_PROC(vkDestroyDescriptorSetLayout);
    LOAD_DEV_PROC(vkCreatePipelineLayout);
    LOAD_DEV_PROC(vkDestroyPipelineLayout);
    LOAD_DEV_PROC(vkCreateComputePipelines);
    LOAD_DEV_PROC(vkDestroyPipeline);
    LOAD_DEV_PROC(vkCreateDescriptorPool);
    LOAD_DEV_PROC(vkDestroyDescriptorPool);
    LOAD_DEV_PROC(vkAllocateDescriptorSets);
    LOAD_DEV_PROC(vkUpdateDescriptorSets);
    LOAD_DEV_PROC(vkCmdBindPipeline);
    LOAD_DEV_PROC(vkCmdBindDescriptorSets);
    LOAD_DEV_PROC(vkCmdDispatch);
    LOAD_DEV_PROC(vkCmdPushConstants);

    LOAD_DEV_PROC_OPTIONAL(vkCreateShadersEXT);
    LOAD_DEV_PROC_OPTIONAL(vkDestroyShaderEXT);
    LOAD_DEV_PROC_OPTIONAL(vkCmdBindShadersEXT);

    LOAD_DEV_PROC(vkCreateAccelerationStructureKHR);
    LOAD_DEV_PROC(vkDestroyAccelerationStructureKHR);
    LOAD_DEV_PROC(vkGetAccelerationStructureBuildSizesKHR);
    LOAD_DEV_PROC(vkGetAccelerationStructureDeviceAddressKHR);
    LOAD_DEV_PROC(vkCmdBuildAccelerationStructuresKHR);
    LOAD_DEV_PROC(vkGetBufferDeviceAddress);

    vk->vkCmdSetRenderingAttachmentLocations = (PFN_vkCmdSetRenderingAttachmentLocationsKHR)vk->vkGetDeviceProcAddr(vk->device, "vkCmdSetRenderingAttachmentLocationsKHR");
    vk->vkCmdSetRenderingInputAttachmentIndices = (PFN_vkCmdSetRenderingInputAttachmentIndicesKHR)vk->vkGetDeviceProcAddr(vk->device, "vkCmdSetRenderingInputAttachmentIndicesKHR");

    VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .queueFamilyIndex = vk->queue_family_index, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
    if (vk->vkCreateCommandPool(vk->device, &poolInfo, &tsfi_alloc_callbacks, &vk->command_pool) == VK_SUCCESS) {
        tag_vulkan_object(vk, (uint64_t)vk->command_pool, VK_OBJECT_TYPE_COMMAND_POOL, "TSFi_Command_Pool");
        
        VkCommandBufferAllocateInfo cmdAlloc = {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = vk->command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 3
        };
        if (vk->vkAllocateCommandBuffers(vk->device, &cmdAlloc, vk->command_buffers) == VK_SUCCESS) {
            tag_vulkan_object(vk, (uint64_t)vk->command_buffers[0], VK_OBJECT_TYPE_COMMAND_BUFFER, "TSFi_Command_Buffer_0");
            tag_vulkan_object(vk, (uint64_t)vk->command_buffers[1], VK_OBJECT_TYPE_COMMAND_BUFFER, "TSFi_Command_Buffer_1");
            tag_vulkan_object(vk, (uint64_t)vk->command_buffers[2], VK_OBJECT_TYPE_COMMAND_BUFFER, "TSFi_Command_Buffer_2");
        }

        // Allocate Secondary Buffers for Parallel Plane 71 Work
        cmdAlloc.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        if (vk->vkAllocateCommandBuffers(vk->device, &cmdAlloc, vk->secondary_command_buffers) == VK_SUCCESS) {
            tag_vulkan_object(vk, (uint64_t)vk->secondary_command_buffers[0], VK_OBJECT_TYPE_COMMAND_BUFFER, "TSFi_Secondary_Buffer_0");
            tag_vulkan_object(vk, (uint64_t)vk->secondary_command_buffers[1], VK_OBJECT_TYPE_COMMAND_BUFFER, "TSFi_Secondary_Buffer_1");
            tag_vulkan_object(vk, (uint64_t)vk->secondary_command_buffers[2], VK_OBJECT_TYPE_COMMAND_BUFFER, "TSFi_Secondary_Buffer_2");
        }
    }

    vk->rebar_pool_size = 1024 * 1024 * 1024;
    VkBufferCreateInfo rebarInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = vk->rebar_pool_size, .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, .sharingMode = VK_SHARING_MODE_EXCLUSIVE };
    if (vk->vkCreateBuffer(vk->device, &rebarInfo, &tsfi_alloc_callbacks, &vk->rebar_buffer) == VK_SUCCESS) {
        tag_vulkan_object(vk, (uint64_t)vk->rebar_buffer, VK_OBJECT_TYPE_BUFFER, "Zhong_ReBAR_Buffer");
        VkMemoryRequirements reqs;
        vk->vkGetBufferMemoryRequirements(vk->device, vk->rebar_buffer, &reqs);
        uint32_t typeIndex = 0; bool found = false;
        VkMemoryPropertyFlags target = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
        for (uint32_t i=0; i<vk->memory_properties.memoryTypeCount; i++) {
            if ((reqs.memoryTypeBits & (1 << i)) && (vk->memory_properties.memoryTypes[i].propertyFlags & target) == target) {
                typeIndex = i; found = true; break;
            }
        }
        if (!found) {
            for (uint32_t i=0; i<vk->memory_properties.memoryTypeCount; i++) {
                if ((reqs.memoryTypeBits & (1 << i)) && (vk->memory_properties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
                    typeIndex = i; found = true; break;
                }
            }
        }
        VkMemoryAllocateFlagsInfo allocFlags = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, .flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT };
        VkMemoryAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .pNext = &allocFlags, .allocationSize = reqs.size, .memoryTypeIndex = typeIndex };
        if (vk->vkAllocateMemory(vk->device, &allocInfo, &tsfi_alloc_callbacks, &vk->rebar_memory) == VK_SUCCESS) {
            tag_vulkan_object(vk, (uint64_t)vk->rebar_memory, VK_OBJECT_TYPE_DEVICE_MEMORY, "Zhong_ReBAR_Memory");
            vk->vkBindBufferMemory(vk->device, vk->rebar_buffer, vk->rebar_memory, 0);
            vk->vkMapMemory(vk->device, vk->rebar_memory, 0, vk->rebar_pool_size, 0, &vk->rebar_mapped_ptr);
            
            if (vk->vkGetBufferDeviceAddress) {
                VkBufferDeviceAddressInfo bdaInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = vk->rebar_buffer };
                vk->rebar_device_address = vk->vkGetBufferDeviceAddress(vk->device, &bdaInfo);
                printf("[VULKAN] 1GB ReBAR Pool Initialized at Host: %p, GPU BDA: 0x%lx\n", vk->rebar_mapped_ptr, (unsigned long)vk->rebar_device_address);

                // --- NEW: Buffer Aliasing Implementation ---
                // We create a second buffer handle that aliases the physical memory but with a "Secret" offset (e.g. 512MB)
                VkBufferCreateInfo secretInfo = rebarInfo;
                secretInfo.size = vk->rebar_pool_size / 2;
                if (vk->vkCreateBuffer(vk->device, &secretInfo, &tsfi_alloc_callbacks, &vk->secret_rebar_buffer) == VK_SUCCESS) {
                    tag_vulkan_object(vk, (uint64_t)vk->secret_rebar_buffer, VK_OBJECT_TYPE_BUFFER, "Zhong_Secret_Aliased_Buffer");
                    // Bind to the SAME physical memory block but at an offset
                    vk->vkBindBufferMemory(vk->device, vk->secret_rebar_buffer, vk->rebar_memory, vk->rebar_pool_size / 2);
                    
                    VkBufferDeviceAddressInfo sBdaInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer = vk->secret_rebar_buffer };
                    VkDeviceAddress secret_bda = vk->vkGetBufferDeviceAddress(vk->device, &sBdaInfo);
                    printf("[VULKAN] Aliased Secret Buffer Ready. GPU BDA: 0x%lx (Offset Switch Path Active)\n", (unsigned long)secret_bda);
                }
            } else {
                printf("[VULKAN] 1GB ReBAR Pool Initialized at %p.\n", vk->rebar_mapped_ptr);
            }
        }
    }

    lau_init_sync_objects(vk);
    lau_init_samplers(vk);
    lau_init_stuffed_manifold(vk);
    lau_init_queries(vk);

    return vk;
}

VulkanContext *init_vulkan_display() {
    printf("[VULKAN] Initializing Direct Display Context (KMS/VTY)...\n"); fflush(stdout);
    VulkanContext *vk = (VulkanContext *)lau_memalign(512, sizeof(VulkanContext));
    if (!vk) return NULL;
    memset(vk, 0, sizeof(VulkanContext));

    vk->lib_handle = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!vk->lib_handle) { printf("[VULKAN] Failed to load libvulkan.so.1\n"); lau_free(vk); return NULL; }

    vk->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(vk->lib_handle, "vkGetInstanceProcAddr");
    if (!vk->vkGetInstanceProcAddr) { cleanup_vulkan(vk); return NULL; }
    
    PFN_vkCreateInstance createInstance = (PFN_vkCreateInstance)vk->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    vk->vkCreateInstance = createInstance;

    VkApplicationInfo appInfo = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_4 };
    const char *extensions[] = { 
        "VK_KHR_surface", 
        "VK_KHR_display",
        "VK_EXT_debug_utils" 
    };
    
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo,
        .enabledExtensionCount = 3,
        .ppEnabledExtensionNames = extensions
    };

    if (vk->vkCreateInstance(&createInfo, &tsfi_alloc_callbacks, &vk->instance) != VK_SUCCESS) { cleanup_vulkan(vk); return NULL; }

    LOAD_PROC(vkDestroyInstance);
    LOAD_PROC(vkEnumeratePhysicalDevices);
    LOAD_PROC(vkGetPhysicalDeviceProperties);
    LOAD_PROC(vkGetPhysicalDeviceMemoryProperties);
    LOAD_PROC(vkGetDeviceProcAddr);
    LOAD_PROC(vkCreateDevice);
    LOAD_PROC(vkDestroyDevice);
    LOAD_PROC(vkGetDeviceQueue);

    // Display Extensions Procs
    LOAD_PROC(vkGetPhysicalDeviceDisplayPropertiesKHR);
    LOAD_PROC(vkGetPhysicalDeviceDisplayPlanePropertiesKHR);
    LOAD_PROC(vkGetDisplayModePropertiesKHR);
    LOAD_PROC(vkCreateDisplayPlaneSurfaceKHR);

    uint32_t dev_count = 0;
    vk->vkEnumeratePhysicalDevices(vk->instance, &dev_count, NULL);
    if (dev_count == 0) { cleanup_vulkan(vk); return NULL; }
    VkPhysicalDevice devices[4];
    vk->vkEnumeratePhysicalDevices(vk->instance, &dev_count, devices);
    vk->physical_device = devices[0];

    // Pick a display and mode
    uint32_t displayCount = 0;
    vk->vkGetPhysicalDeviceDisplayPropertiesKHR(vk->physical_device, &displayCount, NULL);
    if (displayCount == 0) { printf("[VULKAN] No displays found.\n"); cleanup_vulkan(vk); return NULL; }
    
    VkDisplayPropertiesKHR displayProps;
    uint32_t count = 1;
    vk->vkGetPhysicalDeviceDisplayPropertiesKHR(vk->physical_device, &count, &displayProps);
    printf("[VULKAN] Using Display: %s\n", displayProps.displayName);

    uint32_t modeCount = 0;
    vk->vkGetDisplayModePropertiesKHR(vk->physical_device, displayProps.display, &modeCount, NULL);
    VkDisplayModePropertiesKHR modeProps;
    count = 1;
    vk->vkGetDisplayModePropertiesKHR(vk->physical_device, displayProps.display, &count, &modeProps);
    
    VkDisplaySurfaceCreateInfoKHR surfaceInfo = {
        .sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
        .displayMode = modeProps.displayMode,
        .planeIndex = 0, // Assumption: Plane 0 is usable
        .planeStackIndex = 0,
        .transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .globalAlpha = 1.0f,
        .alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
        .imageExtent = modeProps.parameters.visibleRegion
    };
    
    if (vk->vkCreateDisplayPlaneSurfaceKHR(vk->instance, &surfaceInfo, &tsfi_alloc_callbacks, &vk->surface) != VK_SUCCESS) {
        printf("[VULKAN] Failed to create display surface.\n");
        cleanup_vulkan(vk);
        return NULL;
    }

    // Standard Device Init (Reusing logic would be better, but for VTY we keep it simple for now)
    uint32_t qCount = 0;
    vk->vkGetPhysicalDeviceQueueFamilyProperties(vk->physical_device, &qCount, NULL);
    VkQueueFamilyProperties *qProps = (VkQueueFamilyProperties*)lau_malloc(sizeof(VkQueueFamilyProperties) * qCount);
    vk->vkGetPhysicalDeviceQueueFamilyProperties(vk->physical_device, &qCount, qProps);
    int qIndex = -1;
    for (uint32_t i = 0; i < qCount; i++) { if (qProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { qIndex = i; break; } }
    lau_free(qProps);
    
    float priority = 1.0f;
    VkDeviceQueueCreateInfo qInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = qIndex, .queueCount = 1, .pQueuePriorities = &priority };
    
    const char *devExts[] = { "VK_KHR_swapchain" };
    VkDeviceCreateInfo devInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1, .pQueueCreateInfos = &qInfo, .enabledExtensionCount = 1, .ppEnabledExtensionNames = devExts };

    if (vk->vkCreateDevice(vk->physical_device, &devInfo, &tsfi_alloc_callbacks, &vk->device) != VK_SUCCESS) {
        cleanup_vulkan(vk);
        return NULL;
    }

    vk->vkGetDeviceQueue(vk->device, qIndex, 0, &vk->queue);
    
    // Minimal standard proc loading
    LOAD_DEV_PROC(vkCreateSwapchainKHR);
    LOAD_DEV_PROC(vkDestroySwapchainKHR);
    LOAD_DEV_PROC(vkGetSwapchainImagesKHR);
    LOAD_DEV_PROC(vkAcquireNextImageKHR);
    LOAD_DEV_PROC(vkQueuePresentKHR);
    LOAD_DEV_PROC(vkCreateSemaphore);
    LOAD_DEV_PROC(vkDestroySemaphore);
    LOAD_DEV_PROC(vkCreateFence);
    LOAD_DEV_PROC(vkDestroyFence);
    LOAD_DEV_PROC(vkCreateCommandPool);
    LOAD_DEV_PROC(vkAllocateCommandBuffers);
    LOAD_DEV_PROC(vkBeginCommandBuffer);
    LOAD_DEV_PROC(vkEndCommandBuffer);
    LOAD_DEV_PROC(vkQueueSubmit);
    LOAD_DEV_PROC(vkDeviceWaitIdle);
    LOAD_DEV_PROC(vkWaitForFences);
    LOAD_DEV_PROC(vkResetFences);

    lau_init_sync_objects(vk);
    lau_init_samplers(vk);
    lau_init_stuffed_manifold(vk);
    lau_init_queries(vk);

    return vk;
}

void init_vk_swapchain(VulkanContext *vk, int width, int height) {
    if (!vk || !vk->device || !vk->surface) return;

    uint32_t formatCount = 0;
    if (vk->vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physical_device, vk->surface, &formatCount, NULL) != VK_SUCCESS || formatCount == 0) {
        printf("[VULKAN] Error: Failed to get surface formats or surface lost.\n");
        return;
    }
    VkSurfaceFormatKHR *formats = (VkSurfaceFormatKHR*)lau_malloc(sizeof(VkSurfaceFormatKHR) * formatCount);
    vk->vkGetPhysicalDeviceSurfaceFormatsKHR(vk->physical_device, vk->surface, &formatCount, formats);
    lau_free(formats);

    VkSurfaceCapabilitiesKHR caps;
    if (vk->vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->physical_device, vk->surface, &caps) != VK_SUCCESS) {
        printf("[VULKAN] Error: Failed to get surface capabilities.\n");
        return;
    }

    VkSwapchainCreateInfoKHR swapInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, .surface = vk->surface, .minImageCount = caps.minImageCount + 1,
        .imageFormat = VK_FORMAT_B8G8R8A8_SRGB, .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR, .imageExtent = caps.currentExtent,
        .imageArrayLayers = 1, .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE, .preTransform = caps.currentTransform, .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR, .clipped = VK_TRUE, .oldSwapchain = VK_NULL_HANDLE
    };

    // Ensure preTransform is a single valid bit
    if (swapInfo.preTransform == 0 || (swapInfo.preTransform & (swapInfo.preTransform - 1)) != 0) {
        swapInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }

    if (caps.currentExtent.width == 0xFFFFFFFF || (caps.currentExtent.width == 0 && caps.currentExtent.height == 0)) {
            swapInfo.imageExtent.width = (uint32_t)width;
            swapInfo.imageExtent.height = (uint32_t)height;
            
            // Clamp to limits
            if (swapInfo.imageExtent.width < caps.minImageExtent.width) swapInfo.imageExtent.width = caps.minImageExtent.width;
            if (swapInfo.imageExtent.height < caps.minImageExtent.height) swapInfo.imageExtent.height = caps.minImageExtent.height;
            if (swapInfo.imageExtent.width > caps.maxImageExtent.width) swapInfo.imageExtent.width = caps.maxImageExtent.width;
            if (swapInfo.imageExtent.height > caps.maxImageExtent.height) swapInfo.imageExtent.height = caps.maxImageExtent.height;
        }
    
    if (vk->vkCreateSwapchainKHR(vk->device, &swapInfo, &tsfi_alloc_callbacks, &vk->swapchain) != VK_SUCCESS) return;
    vk->vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchainImageCount, NULL);
    vk->swapchainImages = (VkImage*)lau_malloc(sizeof(VkImage) * vk->swapchainImageCount);
    vk->vkGetSwapchainImagesKHR(vk->device, vk->swapchain, &vk->swapchainImageCount, vk->swapchainImages);
    vk->swapchainImageViews = (VkImageView*)lau_malloc(sizeof(VkImageView) * vk->swapchainImageCount);
    memset(vk->swapchainImageViews, 0, sizeof(VkImageView) * vk->swapchainImageCount);
    for (uint32_t i = 0; i < vk->swapchainImageCount; i++) {
        VkImageViewCreateInfo viewInfo = { .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, .image = vk->swapchainImages[i], .viewType = VK_IMAGE_VIEW_TYPE_2D, .format = VK_FORMAT_B8G8R8A8_SRGB, .subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
        vk->vkCreateImageView(vk->device, &viewInfo, &tsfi_alloc_callbacks, &vk->swapchainImageViews[i]);
    }
}

void lau_init_sync_objects(VulkanContext *vk) {

    if (!vk || !vk->device) return;

    VkSemaphoreCreateInfo semInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

    VkFenceCreateInfo fenceInfo = { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = VK_FENCE_CREATE_SIGNALED_BIT };

    

            for (int i = 0; i < 3; i++) {

    

                vk->vkCreateSemaphore(vk->device, &semInfo, &tsfi_alloc_callbacks, &vk->imageAvailableSemaphores[i]);

    

                vk->vkCreateFence(vk->device, &fenceInfo, &tsfi_alloc_callbacks, &vk->inFlightFences[i]);

    

                

    

                char name[64];

    

                snprintf(name, 64, "ImageAvailable_Semaphore_%d", i);

    

                tag_vulkan_object(vk, (uint64_t)vk->imageAvailableSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE, name);

    

                snprintf(name, 64, "InFlight_Fence_%d", i);

    

                tag_vulkan_object(vk, (uint64_t)vk->inFlightFences[i], VK_OBJECT_TYPE_FENCE, name);

    

            }

    

        

    

            for (int i = 0; i < 8; i++) {

    

                vk->vkCreateSemaphore(vk->device, &semInfo, &tsfi_alloc_callbacks, &vk->renderFinishedSemaphores[i]);

    

                char name[64];

    

                snprintf(name, 64, "RenderFinished_Semaphore_%d", i);

    

                tag_vulkan_object(vk, (uint64_t)vk->renderFinishedSemaphores[i], VK_OBJECT_TYPE_SEMAPHORE, name);

    

            }

    

    VkSemaphoreTypeCreateInfo timelineInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO, .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE, .initialValue = 0 };
    VkSemaphoreCreateInfo tSemInfo = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .pNext = &timelineInfo };
    vk->vkCreateSemaphore(vk->device, &tSemInfo, &tsfi_alloc_callbacks, &vk->timelineSemaphore);
    tag_vulkan_object(vk, (uint64_t)vk->timelineSemaphore, VK_OBJECT_TYPE_SEMAPHORE, "Main_Timeline_Semaphore");
}

void tag_vulkan_object(VulkanContext *vk, uint64_t handle, VkObjectType type, const char *name) {
    if (!vk || !vk->vkSetDebugUtilsObjectNameEXT || !handle || !name) return;
    VkDebugUtilsObjectNameInfoEXT nameInfo = { .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT, .objectType = type, .objectHandle = handle, .pObjectName = name };
    vk->vkSetDebugUtilsObjectNameEXT(vk->device, &nameInfo);
}

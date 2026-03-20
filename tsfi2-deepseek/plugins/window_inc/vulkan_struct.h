#ifndef VULKAN_STRUCT_H
#define VULKAN_STRUCT_H

#include <wayland-client.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_wayland.h>
#include <stdbool.h>

// Minimal Vulkan Context for Hybrid Rendering
typedef struct VulkanContext {
    VkInstance instance;
    VkPhysicalDevice physical_device;
    VkDevice device;
    VkQueue queue;
    uint32_t queue_family_index;
    
    // WSI
    VkSurfaceKHR surface;
    VkSwapchainKHR swapchain;
    VkImage *swapchainImages;
    VkImageView *swapchainImageViews;
    uint32_t swapchainImageCount;

    // Debug Utils
    VkDebugUtilsMessengerEXT debugMessenger;
    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT;
    PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT;
    PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectNameEXT;
    
    // Commands
    VkCommandPool command_pool;
    VkCommandBuffer command_buffers[3];
    VkCommandBuffer secondary_command_buffers[3];
    
    // Sync
    VkSemaphore imageAvailableSemaphores[3];
    VkSemaphore renderFinishedSemaphores[8]; // Max Swapchain Images
    VkFence inFlightFences[3];
    uint32_t currentFrame;

    VkSemaphore timelineSemaphore;
    uint64_t timelineValue;
    
    // External Memory Import
    VkImage image;
    VkDeviceMemory memory;
    VkPhysicalDeviceMemoryProperties memory_properties;
    
    // Core Function Pointers (Loaded via dlsym)
    void *lib_handle;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkCreateInstance vkCreateInstance;
    PFN_vkDestroyInstance vkDestroyInstance;
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
    PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties;
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
    PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
    PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
    
    PFN_vkCreateDevice vkCreateDevice;
    PFN_vkDestroyDevice vkDestroyDevice;
    PFN_vkGetDeviceQueue vkGetDeviceQueue;
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr; // CRITICAL
    
    PFN_vkCreateCommandPool vkCreateCommandPool;
    PFN_vkDestroyCommandPool vkDestroyCommandPool;
    PFN_vkResetCommandPool vkResetCommandPool;
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
    PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
    PFN_vkEndCommandBuffer vkEndCommandBuffer;
    PFN_vkCmdExecuteCommands vkCmdExecuteCommands;
    PFN_vkQueueSubmit vkQueueSubmit;
    PFN_vkQueueWaitIdle vkQueueWaitIdle;
    PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
    
    PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
    PFN_vkCmdClearColorImage vkCmdClearColorImage;
    
    // Roadmap 2022/2024 Core
    PFN_vkCmdBeginRendering vkCmdBeginRendering;
    PFN_vkCmdEndRendering vkCmdEndRendering;
    PFN_vkCmdPipelineBarrier2 vkCmdPipelineBarrier2;
    PFN_vkCmdDraw vkCmdDraw;
    
    // Sync Functions
    PFN_vkCreateSemaphore vkCreateSemaphore;
    PFN_vkDestroySemaphore vkDestroySemaphore;
    PFN_vkSignalSemaphore vkSignalSemaphore;
    PFN_vkWaitSemaphores vkWaitSemaphores;
    PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
    PFN_vkCreateFence vkCreateFence;
    PFN_vkDestroyFence vkDestroyFence;
    PFN_vkWaitForFences vkWaitForFences;
    PFN_vkResetFences vkResetFences;
    
    // WSI Function Pointers
    PFN_vkCreateWaylandSurfaceKHR vkCreateWaylandSurfaceKHR;
    PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;
    PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
    PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
    PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
    PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
    PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
    
    // Direct Display (VK_KHR_display)
    PFN_vkGetPhysicalDeviceDisplayPropertiesKHR vkGetPhysicalDeviceDisplayPropertiesKHR;
    PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR vkGetPhysicalDeviceDisplayPlanePropertiesKHR;
    PFN_vkGetDisplayModePropertiesKHR vkGetDisplayModePropertiesKHR;
    PFN_vkCreateDisplayPlaneSurfaceKHR vkCreateDisplayPlaneSurfaceKHR;

    PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
    PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
    PFN_vkQueuePresentKHR vkQueuePresentKHR;
    
    // Memory/Image Pointers
    PFN_vkCreateImage vkCreateImage;
    PFN_vkDestroyImage vkDestroyImage;
    PFN_vkCreateImageView vkCreateImageView;
    PFN_vkDestroyImageView vkDestroyImageView;
    PFN_vkCreateSampler vkCreateSampler;
    PFN_vkDestroySampler vkDestroySampler;
    PFN_vkAllocateMemory vkAllocateMemory;
    PFN_vkFreeMemory vkFreeMemory;
    PFN_vkBindImageMemory vkBindImageMemory;
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
    PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
    PFN_vkGetMemoryFdKHR vkGetMemoryFdKHR;

    // Queries
    PFN_vkCreateQueryPool vkCreateQueryPool;
    PFN_vkDestroyQueryPool vkDestroyQueryPool;
    PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
    PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
    PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
    PFN_vkCmdBeginQuery vkCmdBeginQuery;
    PFN_vkCmdEndQuery vkCmdEndQuery;
    PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;

    // Buffers and Memory Mapping
    PFN_vkCreateBuffer vkCreateBuffer;
    PFN_vkDestroyBuffer vkDestroyBuffer;
    PFN_vkCreateBufferView vkCreateBufferView;
    PFN_vkDestroyBufferView vkDestroyBufferView;
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
    PFN_vkBindBufferMemory vkBindBufferMemory;
    PFN_vkMapMemory vkMapMemory;
    PFN_vkUnmapMemory vkUnmapMemory;
    PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
    
    // Host Image Copy
    PFN_vkGetImageSubresourceLayout2EXT vkGetImageSubresourceLayout2EXT;
    PFN_vkCopyMemoryToImageEXT vkCopyMemoryToImageEXT;
    PFN_vkTransitionImageLayoutEXT vkTransitionImageLayoutEXT;
    
    // --- Compute & PipelineThunk Function Pointers ---
    PFN_vkCreateShaderModule vkCreateShaderModule;
    PFN_vkDestroyShaderModule vkDestroyShaderModule;
    PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
    PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
    PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
    PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
    PFN_vkCreateComputePipelines vkCreateComputePipelines;
    PFN_vkDestroyPipeline vkDestroyPipeline;
    PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
    PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
    PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
    PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
    PFN_vkCmdBindPipeline vkCmdBindPipeline;
    PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
    PFN_vkCmdDispatch vkCmdDispatch;
    PFN_vkCmdPushConstants vkCmdPushConstants;

    // Shader Objects (VK_EXT_shader_object)
    PFN_vkCreateShadersEXT vkCreateShadersEXT;
    PFN_vkDestroyShaderEXT vkDestroyShaderEXT;
    PFN_vkCmdBindShadersEXT vkCmdBindShadersEXT;

    // Dynamic Rendering Local Read
    PFN_vkCmdSetRenderingAttachmentLocationsKHR vkCmdSetRenderingAttachmentLocations;
    PFN_vkCmdSetRenderingInputAttachmentIndicesKHR vkCmdSetRenderingInputAttachmentIndices;

    // --- Ray Query & Acceleration Structure ---
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR;
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR;
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;

    // --- Vulkan Video Encode Function Pointers ---
    PFN_vkCreateVideoSessionKHR vkCreateVideoSessionKHR;
    PFN_vkDestroyVideoSessionKHR vkDestroyVideoSessionKHR;
    PFN_vkCreateVideoSessionParametersKHR vkCreateVideoSessionParametersKHR;
    PFN_vkDestroyVideoSessionParametersKHR vkDestroyVideoSessionParametersKHR;
    PFN_vkGetVideoSessionMemoryRequirementsKHR vkGetVideoSessionMemoryRequirementsKHR;
    PFN_vkBindVideoSessionMemoryKHR vkBindVideoSessionMemoryKHR;
    PFN_vkCmdBeginVideoCodingKHR vkCmdBeginVideoCodingKHR;
    PFN_vkCmdEndVideoCodingKHR vkCmdEndVideoCodingKHR;
    PFN_vkCmdEncodeVideoKHR vkCmdEncodeVideoKHR;

    // --- Logical Pipeline Resources ---
    VkPipelineLayout compute_pipeline_layout;
    VkPipeline compute_pipeline;
    
    // ReBAR Pool (Zhong)
    VkBuffer rebar_buffer;
    VkBuffer secret_rebar_buffer; // Aliased view for secure context switching
    VkDeviceMemory rebar_memory;
    void* rebar_mapped_ptr;
    size_t rebar_pool_size;
    size_t rebar_pool_offset;
    VkDeviceAddress rebar_device_address; // 64-bit raw GPU pointer for BDA massing
    bool is_leased; // If true, rendering directly to Plane 71
    int leased_fd;

    // Sampler Manifold
    VkSampler sampler_point;
    VkSampler sampler_linear;
    VkSampler sampler_aniso;
    VkDescriptorSetLayout sampler_descriptor_layout;
    VkDescriptorPool sampler_descriptor_pool;
    VkDescriptorSet sampler_descriptor_set;

    // Stuffed Animal Texel Manifold (Fur & Feathers)
    VkBufferView stuffed_fur_view;
    VkBufferView stuffed_feather_view;

    // Query Manifold
    VkQueryPool query_pool_perf;
    VkQueryPool query_pool_stats;
    VkQueryPool query_pool_ray;
    VkDeviceAddress query_manifold_address; // ReBAR BDA for query direct write
    
} VulkanContext;

#endif

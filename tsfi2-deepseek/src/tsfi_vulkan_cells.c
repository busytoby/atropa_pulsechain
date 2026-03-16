#include "window_inc/vulkan_struct.h"
#include "tsfi_wire_firmware.h"
#include <string.h>
#include <stdio.h>
#include "lau_memory.h"
#include "lau_registry.h"

// ---------------------------------------------------------
// TSFi VULKAN COMPATIBILITY CELLS (FIRMWARE NATIVE)
// ---------------------------------------------------------

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkInstance* pInstance) {
    (void)pAllocator;
    if (!pCreateInfo || pCreateInfo->sType != VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO || !pInstance) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    *pInstance = (VkInstance)0x1000;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks* pAllocator) {
    (void)instance; (void)pAllocator;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t* pPhysicalDeviceCount, VkPhysicalDevice* pPhysicalDevices) {
    (void)instance;
    if (pPhysicalDevices == NULL) {
        *pPhysicalDeviceCount = 1;
        return VK_SUCCESS;
    }
    if (*pPhysicalDeviceCount < 1) {
        return VK_INCOMPLETE;
    }
    pPhysicalDevices[0] = (VkPhysicalDevice)0x2000;
    *pPhysicalDeviceCount = 1;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures* pFeatures) {
    (void)physicalDevice;
    memset(pFeatures, 0, sizeof(VkPhysicalDeviceFeatures));
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetPhysicalDeviceFeatures2(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures2* pFeatures) {
    (void)physicalDevice;
    memset(pFeatures, 0, sizeof(VkPhysicalDeviceFeatures2));
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties* pProperties) {
    (void)physicalDevice;
    memset(pProperties, 0, sizeof(VkPhysicalDeviceProperties));
    pProperties->limits.maxImageDimension2D = 16384;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties* pMemoryProperties) {
    (void)physicalDevice;
    memset(pMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));
    pMemoryProperties->memoryTypeCount = 1;
    pMemoryProperties->memoryTypes[0].propertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice, uint32_t* pQueueFamilyPropertyCount, VkQueueFamilyProperties* pQueueFamilyProperties) {
    (void)physicalDevice;
    if (pQueueFamilyProperties == NULL) {
        *pQueueFamilyPropertyCount = 1;
    } else {
        pQueueFamilyProperties[0].queueFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
        pQueueFamilyProperties[0].queueCount = 1;
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName, uint32_t* pPropertyCount, VkExtensionProperties* pProperties) {
    (void)physicalDevice; (void)pLayerName;
    if (pProperties == NULL) {
        *pPropertyCount = 2; // Let's mock 2 extensions available
        return VK_SUCCESS;
    }
    if (*pPropertyCount < 2) {
        if (*pPropertyCount > 0) {
            strncpy(pProperties[0].extensionName, "VK_KHR_swapchain", VK_MAX_EXTENSION_NAME_SIZE);
            pProperties[0].specVersion = 1;
        }
        return VK_INCOMPLETE;
    }
    strncpy(pProperties[0].extensionName, "VK_KHR_swapchain", VK_MAX_EXTENSION_NAME_SIZE);
    pProperties[0].specVersion = 1;
    strncpy(pProperties[1].extensionName, "VK_KHR_timeline_semaphore", VK_MAX_EXTENSION_NAME_SIZE);
    pProperties[1].specVersion = 1;
    *pPropertyCount = 2;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    (void)physicalDevice; (void)pCreateInfo; (void)pAllocator;
    *pDevice = (VkDevice)0x3000;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks* pAllocator) {
    (void)device; (void)pAllocator;
}

typedef struct {
    MappedCommon common; // Embed Thunk Topography
    uint32_t queue_family_index;
    uint32_t queue_index;
    VkResult (*submit)(void* ctx, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
    VkResult (*wait_idle)(void* ctx);
    VkResult (*present)(void* ctx, const VkPresentInfoKHR* pPresentInfo);
} TSFiQueue;

static void tsfi_queue_teardown(void *ptr) {
    (void)ptr;
}

static VkResult tsfi_queue_submit_impl(void* ctx, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence);
static VkResult tsfi_queue_wait_idle_impl(void* ctx);
static VkResult tsfi_queue_present_impl(void* ctx, const VkPresentInfoKHR* pPresentInfo);

const ThunkSignature TSFiQueue_Schema[] = {
    { offsetof(TSFiQueue, submit), THUNK_BAKED, 1, (void*)tsfi_queue_submit_impl },
    { offsetof(TSFiQueue, wait_idle), THUNK_BAKED, 1, (void*)tsfi_queue_wait_idle_impl },
    { offsetof(TSFiQueue, present), THUNK_BAKED, 1, (void*)tsfi_queue_present_impl }
};
const int TSFiQueue_Schema_Count = sizeof(TSFiQueue_Schema) / sizeof(ThunkSignature);

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue* pQueue) {
    (void)device;
    TSFiQueue* q = (TSFiQueue*)lau_malloc_wired(sizeof(TSFiQueue));
    if (!q) {
        *pQueue = VK_NULL_HANDLE;
        return;
    }
    q->queue_family_index = queueFamilyIndex;
    q->queue_index = queueIndex;
    
    TSFI_BIND_SCHEMA(q, TSFiQueue_Schema, TSFiQueue_Schema_Count);
    LauWiredHeader *h = (LauWiredHeader *)((char *)q - 8192);
    h->layer0_teardown = tsfi_queue_teardown;

    h->proxy = (void*)ThunkProxy_create();
    ThunkProxy_emit_mapped((ThunkProxy*)h->proxy, (MappedCommon*)q, h);
    
    // Attempt Layer-1 Sealing by default
    extern void* ThunkProxy_emit_layer1_seal(ThunkProxy *p, void *target_fn);
    if (h->proxy && q->submit) q->submit = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)q->submit);
    if (h->proxy && q->wait_idle) q->wait_idle = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)q->wait_idle);
    if (h->proxy && q->present) q->present = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)q->present);
    
    lau_seal_object(q);
    
    *pQueue = (VkQueue)q;
    // printf("[FIRMWARE] vkGetDeviceQueue allocated hardware TSFiQueue at %p and SEALED it.\\n", q);
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkCommandPool* pCommandPool) {
    (void)device; (void)pCreateInfo; (void)pAllocator;
    *pCommandPool = (VkCommandPool)0x5000;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks* pAllocator) {
    (void)device; (void)commandPool; (void)pAllocator;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo* pAllocateInfo, VkCommandBuffer* pCommandBuffers) {
    (void)device;
    for (uint32_t i = 0; i < pAllocateInfo->commandBufferCount; i++) {
        pCommandBuffers[i] = (VkCommandBuffer)(uintptr_t)(0x6000 + i);
    }
    return VK_SUCCESS;
}












static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateFence(VkDevice device, const VkFenceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkFence* pFence) {
    (void)device; (void)pCreateInfo; (void)pAllocator;
    *pFence = (VkFence)0xA000;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks* pAllocator) {
    (void)device; (void)fence; (void)pAllocator;
}

static VkResult tsfi_queue_submit_impl(void* ctx, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) {
    (void)fence; 
    lau_unseal_object(ctx);
    
    for (uint32_t s = 0; s < submitCount; s++) {
        const VkSubmitInfo* info = &pSubmits[s];
        
        for (uint32_t c = 0; c < info->commandBufferCount; c++) {
            // We use void* to represent TSFiCommandBuffer as it might not be fully defined here
            void* cb_ptr = (void*)info->pCommandBuffers[c];
            if (cb_ptr) {
                // Here we simulate hardware submission
                // printf("[FIRMWARE] Queue Submitted PM4 Command Buffer\n");
            }
        }
    }
    lau_seal_object(ctx);
    return VK_SUCCESS;
}

static VkResult tsfi_queue_wait_idle_impl(void* ctx) { 
    (void)ctx; 
    // printf("[FIRMWARE] Queue Wait Idle\n");
    return VK_SUCCESS; 
}

static VkResult tsfi_queue_present_impl(void* ctx, const VkPresentInfoKHR* pPresentInfo) { 
    (void)ctx;
    if (pPresentInfo && pPresentInfo->swapchainCount > 0) {
        // printf("[FIRMWARE] Queue Present\n");
    }
    return VK_SUCCESS; 
}


// --- Reflective TSFiBuffer ---
typedef struct TSFiBuffer_s {
    MappedCommon common;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkDeviceMemory bound_memory;
    VkDeviceSize bound_offset;
    void (*get_memory_requirements)(void* ctx, VkMemoryRequirements* pMemoryRequirements);
    VkResult (*bind_memory)(void* ctx, VkDeviceMemory memory, VkDeviceSize memoryOffset);
} TSFiBuffer;

static void tsfi_buffer_teardown(void *ptr) { (void)ptr; }

static void tsfi_buffer_get_memory_requirements_impl(void* ctx, VkMemoryRequirements* pMemoryRequirements) {
    TSFiBuffer* b = (TSFiBuffer*)ctx;
    lau_unseal_object(ctx);
    pMemoryRequirements->size = b->size;
    pMemoryRequirements->alignment = 4096;
    pMemoryRequirements->memoryTypeBits = 0x7; // Bits 0, 1, and 2 are all valid (VRAM, ReBAR, SysRAM)
    lau_seal_object(ctx);
}

static VkResult tsfi_buffer_bind_memory_impl(void* ctx, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    TSFiBuffer* b = (TSFiBuffer*)ctx;
    lau_unseal_object(ctx);
    b->bound_memory = memory;
    b->bound_offset = memoryOffset;
    lau_seal_object(ctx);
    return VK_SUCCESS;
}

__attribute__((used)) static void tsfi_buffer_get_memory_requirements_impl(void* ctx, VkMemoryRequirements* pMemoryRequirements);
__attribute__((used)) static VkResult tsfi_buffer_bind_memory_impl(void* ctx, VkDeviceMemory memory, VkDeviceSize memoryOffset);

const ThunkSignature TSFiBuffer_Schema[] = {
    { offsetof(TSFiBuffer, get_memory_requirements), THUNK_BAKED, 1, (void*)tsfi_buffer_get_memory_requirements_impl },
    { offsetof(TSFiBuffer, bind_memory), THUNK_BAKED, 1, (void*)tsfi_buffer_bind_memory_impl }
};
const int TSFiBuffer_Schema_Count = sizeof(TSFiBuffer_Schema) / sizeof(ThunkSignature);


// --- Reflective TSFiDeviceMemory ---
typedef struct TSFiDeviceMemory_s {
    MappedCommon common;
    VkDeviceSize size;
    void* host_ptr; // The actual LAU_GPU allocation
    VkResult (*map_memory)(void* ctx, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
    void (*unmap_memory)(void* ctx);
} TSFiDeviceMemory;

static void tsfi_device_memory_teardown(void *ptr) { 
    TSFiDeviceMemory* m = (TSFiDeviceMemory*)ptr;
    if (m->host_ptr) {
        lau_free(m->host_ptr);
        m->host_ptr = NULL;
    }
}

static VkResult tsfi_device_memory_map_memory_impl(void* ctx, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    (void)size; (void)flags;
    TSFiDeviceMemory* m = (TSFiDeviceMemory*)ctx;
    lau_unseal_object(ctx);
    *ppData = (void*)((uint8_t*)m->host_ptr + offset);
    lau_seal_object(ctx);
    return VK_SUCCESS;
}

static void tsfi_device_memory_unmap_memory_impl(void* ctx) {
    (void)ctx;
    // Zero copy, nothing to unmap physically
}

__attribute__((used)) static VkResult tsfi_device_memory_map_memory_impl(void* ctx, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
__attribute__((used)) static void tsfi_device_memory_unmap_memory_impl(void* ctx);

const ThunkSignature TSFiDeviceMemory_Schema[] = {
    { offsetof(TSFiDeviceMemory, map_memory), THUNK_BAKED, 1, (void*)tsfi_device_memory_map_memory_impl },
    { offsetof(TSFiDeviceMemory, unmap_memory), THUNK_BAKED, 1, (void*)tsfi_device_memory_unmap_memory_impl }
};
const int TSFiDeviceMemory_Schema_Count = sizeof(TSFiDeviceMemory_Schema) / sizeof(ThunkSignature);

// --- Reflective TSFiSemaphore ---
typedef struct TSFiSemaphore_s {
    MappedCommon common;
    uint64_t value;
} TSFiSemaphore;

static void tsfi_semaphore_teardown(void *ptr) { (void)ptr; }



static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateBuffer(VkDevice device, const VkBufferCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkBuffer* pBuffer) {
    (void)device; (void)pAllocator;
    TSFiBuffer* b = (TSFiBuffer*)lau_malloc_wired(sizeof(TSFiBuffer));
    if (!b) return VK_ERROR_OUT_OF_HOST_MEMORY;
    
    b->size = pCreateInfo->size;
    b->usage = pCreateInfo->usage;
    
    TSFI_BIND_SCHEMA(b, TSFiBuffer_Schema, TSFiBuffer_Schema_Count);
    LauWiredHeader *h = (LauWiredHeader *)((char *)b - 8192);
    h->layer0_teardown = tsfi_buffer_teardown;

    h->proxy = (void*)ThunkProxy_create();
    ThunkProxy_emit_mapped((ThunkProxy*)h->proxy, (MappedCommon*)b, h);
    
    extern void* ThunkProxy_emit_layer1_seal(ThunkProxy *p, void *target_fn);
    if (h->proxy && b->get_memory_requirements) b->get_memory_requirements = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)b->get_memory_requirements);
    if (h->proxy && b->bind_memory) b->bind_memory = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)b->bind_memory);
    
    lau_seal_object(b);
    *pBuffer = (VkBuffer)b;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks* pAllocator) {
    (void)device; (void)pAllocator;
    if (buffer == (VkBuffer)0x7000) return; // Legacy mock fallback
    TSFiBuffer* b = (TSFiBuffer*)buffer;
    if (b) {
        lau_unseal_object(b);
        lau_free(b);
    }
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements* pMemoryRequirements) {
    (void)device;
    if (buffer == (VkBuffer)0x7000) {
        // Legacy mock
        pMemoryRequirements->size = 1024 * 1024 * 1024;
        pMemoryRequirements->alignment = 4096;
        pMemoryRequirements->memoryTypeBits = 0x7;
        return;
    }
    TSFiBuffer* b = (TSFiBuffer*)buffer;
    if (b && b->get_memory_requirements) {
        b->get_memory_requirements(b, pMemoryRequirements);
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo* pAllocateInfo, const VkAllocationCallbacks* pAllocator, VkDeviceMemory* pMemory) {
    (void)device; (void)pAllocator;
    TSFiDeviceMemory* m = (TSFiDeviceMemory*)lau_malloc_wired(sizeof(TSFiDeviceMemory));
    if (!m) return VK_ERROR_OUT_OF_HOST_MEMORY;
    
    m->size = pAllocateInfo->allocationSize;
    m->host_ptr = lau_malloc_gpu_loc(m->size, LAU_GPU_REBAR, __FILE__, __LINE__);
    
    TSFI_BIND_SCHEMA(m, TSFiDeviceMemory_Schema, TSFiDeviceMemory_Schema_Count);
    LauWiredHeader *h = (LauWiredHeader *)((char *)m - 8192);
    h->layer0_teardown = tsfi_device_memory_teardown;

    h->proxy = (void*)ThunkProxy_create();
    ThunkProxy_emit_mapped((ThunkProxy*)h->proxy, (MappedCommon*)m, h);
    
    extern void* ThunkProxy_emit_layer1_seal(ThunkProxy *p, void *target_fn);
    if (h->proxy && m->map_memory) m->map_memory = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)m->map_memory);
    if (h->proxy && m->unmap_memory) m->unmap_memory = (void*)ThunkProxy_emit_layer1_seal((ThunkProxy*)h->proxy, (void*)m->unmap_memory);
    
    lau_seal_object(m);
    *pMemory = (VkDeviceMemory)m;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks* pAllocator) {
    (void)device; (void)pAllocator;
    TSFiDeviceMemory* m = (TSFiDeviceMemory*)memory;
    // Some legacy tests might pass raw memory addresses directly
    LauMetadata* meta = lau_registry_find((void*)m);
    if (meta && (meta->alloc_size >> 56) == LAU_TYPE_GPU) {
        lau_free((void*)m);
        return;
    }
    if (m) {
        lau_unseal_object(m);
        lau_free(m);
    }
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset) {
    (void)device;
    if (buffer == (VkBuffer)0x7000) return VK_SUCCESS;
    TSFiBuffer* b = (TSFiBuffer*)buffer;
    if (b && b->bind_memory) return b->bind_memory(b, memory, memoryOffset);
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData) {
    (void)device;
    TSFiDeviceMemory* m = (TSFiDeviceMemory*)memory;
    
    // Legacy support
    LauMetadata* meta = lau_registry_find((void*)m);
    if (meta && (meta->alloc_size >> 56) == LAU_TYPE_GPU) {
        *ppData = (void*)((uint8_t*)m + offset);
        return VK_SUCCESS;
    }
    
    if (m && m->map_memory) return m->map_memory(m, offset, size, flags, ppData);
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkUnmapMemory(VkDevice device, VkDeviceMemory memory) {
    (void)device;
    TSFiDeviceMemory* m = (TSFiDeviceMemory*)memory;
    
    // Legacy support
    LauMetadata* meta = lau_registry_find((void*)m);
    if (meta && (meta->alloc_size >> 56) == LAU_TYPE_GPU) {
        return;
    }
    
    if (m && m->unmap_memory) m->unmap_memory(m);
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSemaphore* pSemaphore) {
    (void)device; (void)pCreateInfo; (void)pAllocator;
    TSFiSemaphore* sem = (TSFiSemaphore*)lau_malloc_wired(sizeof(TSFiSemaphore));
    if (!sem) return VK_ERROR_OUT_OF_HOST_MEMORY;
    
    sem->value = 0;
    
    // No schema needed right now unless we wire wait/signal ops, but we make it a full wired object
    LauWiredHeader *h = (LauWiredHeader *)((char *)sem - 8192);
    h->layer0_teardown = tsfi_semaphore_teardown;
    h->proxy = (void*)ThunkProxy_create();
    // No methods to emit yet, but the thunk pool exists to satisfy the biological structure constraint
    
    lau_seal_object(sem);
    *pSemaphore = (VkSemaphore)sem;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks* pAllocator) {
    (void)device; (void)pAllocator;
    if (semaphore == (VkSemaphore)0x9000) return; // Legacy mock
    TSFiSemaphore* sem = (TSFiSemaphore*)semaphore;
    if (sem) {
        lau_unseal_object(sem);
        lau_free(sem);
    }
}

// STUBS for Render
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo* pBeginInfo) { (void)commandBuffer; (void)pBeginInfo; return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkEndCommandBuffer(VkCommandBuffer commandBuffer) { (void)commandBuffer; return VK_SUCCESS; }

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo* pSubmits, VkFence fence) { 
    if (queue == (VkQueue)0x4000) return VK_SUCCESS; // Legacy mock
    TSFiQueue* q = (TSFiQueue*)queue;
    if (q && q->submit) return q->submit(q, submitCount, pSubmits, fence);
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkQueueWaitIdle(VkQueue queue) { 
    if (queue == (VkQueue)0x4000) return VK_SUCCESS;
    TSFiQueue* q = (TSFiQueue*)queue;
    if (q && q->wait_idle) return q->wait_idle(q);
    return VK_SUCCESS;
}
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkDeviceWaitIdle(VkDevice device) { (void)device; return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences, VkBool32 waitAll, uint64_t timeout) { (void)device; (void)fenceCount; (void)pFences; (void)waitAll; (void)timeout; return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence* pFences) { (void)device; (void)fenceCount; (void)pFences; return VK_SUCCESS; }

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateSwapchainKHR(VkDevice device, const VkSwapchainCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSwapchainKHR* pSwapchain) { (void)device; (void)pCreateInfo; (void)pAllocator; *pSwapchain = (VkSwapchainKHR)0xB000; return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroySwapchainKHR(VkDevice device, VkSwapchainKHR swapchain, const VkAllocationCallbacks* pAllocator) { (void)device; (void)swapchain; (void)pAllocator; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkGetSwapchainImagesKHR(VkDevice device, VkSwapchainKHR swapchain, uint32_t* pSwapchainImageCount, VkImage* pSwapchainImages) { (void)device; (void)swapchain; if (pSwapchainImages == NULL) *pSwapchainImageCount = 3; return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateImageView(VkDevice device, const VkImageViewCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkImageView* pView) { (void)device; (void)pCreateInfo; (void)pAllocator; *pView = (VkImageView)0xC000; return VK_SUCCESS; }
static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks* pAllocator) { (void)device; (void)imageView; (void)pAllocator; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkAcquireNextImageKHR(VkDevice device, VkSwapchainKHR swapchain, uint64_t timeout, VkSemaphore semaphore, VkFence fence, uint32_t* pImageIndex) { (void)device; (void)swapchain; (void)timeout; (void)semaphore; (void)fence; *pImageIndex = 0; return VK_SUCCESS; }
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo) { 
    if (queue == (VkQueue)0x4000) return VK_SUCCESS;
    TSFiQueue* q = (TSFiQueue*)queue;
    if (q && q->present) return q->present(q, pPresentInfo);
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateWaylandSurfaceKHR(VkInstance instance, const void* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface) {
    (void)instance; (void)pCreateInfo; (void)pAllocator;
    if (pSurface) *pSurface = (VkSurfaceKHR)0xD000;
    return VK_SUCCESS;
}

// Dummy memory FD
static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkGetMemoryFdKHR(VkDevice device, const void* pGetFdInfo, int* pFd) { (void)device; (void)pGetFdInfo; *pFd = 1; return VK_SUCCESS; }

static VKAPI_ATTR VkResult VKAPI_CALL dummy_vulkan_success() { 
    static _Atomic int call_count = 0;
    if (atomic_fetch_add(&call_count, 1) < 5) {
        fprintf(stderr, "[DEBUG VK] dummy_vulkan_success called\n"); fflush(stderr);
    }
    return VK_SUCCESS; 
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkWaitSemaphores(VkDevice device, const VkSemaphoreWaitInfo* pWaitInfo, uint64_t timeout) {
    (void)device; (void)pWaitInfo; (void)timeout;
    // Real synchronization simulation: immediately return success as we mock the timeline
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkSignalSemaphore(VkDevice device, const VkSemaphoreSignalInfo* pSignalInfo) {
    (void)device; (void)pSignalInfo;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    (void)device; (void)pNameInfo;
    return VK_SUCCESS;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkCreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger) {
    (void)instance; (void)pCreateInfo; (void)pAllocator;
    if (pMessenger) *pMessenger = (VkDebugUtilsMessengerEXT)0xE000;
    return VK_SUCCESS;
}

static VKAPI_ATTR void VKAPI_CALL tsfi_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator) {
    (void)instance; (void)messenger; (void)pAllocator;
}

static VKAPI_ATTR VkResult VKAPI_CALL tsfi_vkGetSemaphoreCounterValue(VkDevice device, VkSemaphore semaphore, uint64_t* pValue) {
    (void)device; (void)semaphore;
    if (pValue) {
        LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
        if (fw) *pValue = fw->rtl.zhong_timeline_sig_val;
        else *pValue = 0;
    }
    return VK_SUCCESS;
}

// Export proc address to act as our "Loader"
VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL tsfi_vkGetInstanceProcAddr(VkInstance instance, const char* pName) {
    fprintf(stderr, "[DEBUG VK] Requesting proc: %s\n", pName); fflush(stderr);
    (void)instance;
    if (strcmp(pName, "vkSetDebugUtilsObjectNameEXT") == 0) { return (PFN_vkVoidFunction)tsfi_vkSetDebugUtilsObjectNameEXT; }
    if (strcmp(pName, "vkCreateDebugUtilsMessengerEXT") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateDebugUtilsMessengerEXT; }
    if (strcmp(pName, "vkDestroyDebugUtilsMessengerEXT") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyDebugUtilsMessengerEXT; }
    if (strcmp(pName, "vkGetSemaphoreCounterValue") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetSemaphoreCounterValue; }
    if (strcmp(pName, "vkGetSemaphoreCounterValueKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetSemaphoreCounterValue; }
    if (strcmp(pName, "vkWaitSemaphores") == 0) { return (PFN_vkVoidFunction)tsfi_vkWaitSemaphores; }
    if (strcmp(pName, "vkWaitSemaphoresKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkWaitSemaphores; }
    if (strcmp(pName, "vkSignalSemaphore") == 0) { return (PFN_vkVoidFunction)tsfi_vkSignalSemaphore; }
    if (strcmp(pName, "vkSignalSemaphoreKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkSignalSemaphore; }
    if (strcmp(pName, "vkCreateInstance") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateInstance; }
    if (strcmp(pName, "vkDestroyInstance") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyInstance; }
    if (strcmp(pName, "vkEnumeratePhysicalDevices") == 0) { return (PFN_vkVoidFunction)tsfi_vkEnumeratePhysicalDevices; }
    if (strcmp(pName, "vkGetPhysicalDeviceFeatures") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetPhysicalDeviceFeatures; }
    if (strcmp(pName, "vkGetPhysicalDeviceFeatures2") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetPhysicalDeviceFeatures2; }
    if (strcmp(pName, "vkGetPhysicalDeviceProperties") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetPhysicalDeviceProperties; }
    if (strcmp(pName, "vkGetPhysicalDeviceMemoryProperties") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetPhysicalDeviceMemoryProperties; }
    if (strcmp(pName, "vkGetPhysicalDeviceQueueFamilyProperties") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetPhysicalDeviceQueueFamilyProperties; }
    if (strcmp(pName, "vkEnumerateDeviceExtensionProperties") == 0) { return (PFN_vkVoidFunction)tsfi_vkEnumerateDeviceExtensionProperties; }
    if (strcmp(pName, "vkCreateDevice") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateDevice; }
    if (strcmp(pName, "vkGetDeviceProcAddr") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetInstanceProcAddr; }
    
    // Device procs
    if (strcmp(pName, "vkDestroyDevice") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyDevice; }
    if (strcmp(pName, "vkGetDeviceQueue") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetDeviceQueue; }
    if (strcmp(pName, "vkCreateCommandPool") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateCommandPool; }
    if (strcmp(pName, "vkDestroyCommandPool") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyCommandPool; }
    if (strcmp(pName, "vkAllocateCommandBuffers") == 0) { return (PFN_vkVoidFunction)tsfi_vkAllocateCommandBuffers; }
    if (strcmp(pName, "vkCreateBuffer") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateBuffer; }
    if (strcmp(pName, "vkGetBufferMemoryRequirements") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetBufferMemoryRequirements; }
    if (strcmp(pName, "vkAllocateMemory") == 0) { return (PFN_vkVoidFunction)tsfi_vkAllocateMemory; }
    if (strcmp(pName, "vkBindBufferMemory") == 0) { return (PFN_vkVoidFunction)tsfi_vkBindBufferMemory; }
    if (strcmp(pName, "vkMapMemory") == 0) { return (PFN_vkVoidFunction)tsfi_vkMapMemory; }
    if (strcmp(pName, "vkUnmapMemory") == 0) { return (PFN_vkVoidFunction)tsfi_vkUnmapMemory; }
    if (strcmp(pName, "vkDestroyBuffer") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyBuffer; }
    if (strcmp(pName, "vkFreeMemory") == 0) { return (PFN_vkVoidFunction)tsfi_vkFreeMemory; }
    
    if (strcmp(pName, "vkCreateSemaphore") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateSemaphore; }
    if (strcmp(pName, "vkDestroySemaphore") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroySemaphore; }
    if (strcmp(pName, "vkCreateFence") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateFence; }
    if (strcmp(pName, "vkDestroyFence") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyFence; }
    
    if (strcmp(pName, "vkBeginCommandBuffer") == 0) { return (PFN_vkVoidFunction)tsfi_vkBeginCommandBuffer; }
    if (strcmp(pName, "vkEndCommandBuffer") == 0) { return (PFN_vkVoidFunction)tsfi_vkEndCommandBuffer; }
    if (strcmp(pName, "vkQueueSubmit") == 0) { return (PFN_vkVoidFunction)tsfi_vkQueueSubmit; }
    if (strcmp(pName, "vkQueueWaitIdle") == 0) { return (PFN_vkVoidFunction)tsfi_vkQueueWaitIdle; }
    if (strcmp(pName, "vkDeviceWaitIdle") == 0) { return (PFN_vkVoidFunction)tsfi_vkDeviceWaitIdle; }
    if (strcmp(pName, "vkWaitForFences") == 0) { return (PFN_vkVoidFunction)tsfi_vkWaitForFences; }
    if (strcmp(pName, "vkResetFences") == 0) { return (PFN_vkVoidFunction)tsfi_vkResetFences; }
    
    if (strcmp(pName, "vkCreateSwapchainKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateSwapchainKHR; }
    if (strcmp(pName, "vkDestroySwapchainKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroySwapchainKHR; }
    if (strcmp(pName, "vkGetSwapchainImagesKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetSwapchainImagesKHR; }
    if (strcmp(pName, "vkCreateImageView") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateImageView; }
    if (strcmp(pName, "vkDestroyImageView") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyImageView; }
    if (strcmp(pName, "vkAcquireNextImageKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkAcquireNextImageKHR; }
    if (strcmp(pName, "vkQueuePresentKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkQueuePresentKHR; }
    
    if (strcmp(pName, "vkGetMemoryFdKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkGetMemoryFdKHR; }

    // Optional WSI stuff, we mock returning a success so it doesn't crash
    if (strcmp(pName, "vkDestroySurfaceKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkDestroyInstance; }
    if (strcmp(pName, "vkCreateWaylandSurfaceKHR") == 0) { return (PFN_vkVoidFunction)tsfi_vkCreateWaylandSurfaceKHR; } 
    if (strcmp(pName, "vkGetPhysicalDeviceSurfaceSupportKHR") == 0) { return (PFN_vkVoidFunction)dummy_vulkan_success; }
    if (strcmp(pName, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR") == 0) { return (PFN_vkVoidFunction)dummy_vulkan_success; }
    if (strcmp(pName, "vkGetPhysicalDeviceSurfaceFormatsKHR") == 0) { return (PFN_vkVoidFunction)dummy_vulkan_success; }
    if (strcmp(pName, "vkGetPhysicalDeviceSurfacePresentModesKHR") == 0) { return (PFN_vkVoidFunction)dummy_vulkan_success; }    
    return (PFN_vkVoidFunction)dummy_vulkan_success;
}

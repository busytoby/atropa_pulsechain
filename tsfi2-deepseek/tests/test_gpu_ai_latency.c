#define TSFI_TIME_LIB
#include "tsfi_time.h"
#include "lau_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vulkan/vulkan.h>
#include <dlfcn.h>

// Reuse MiniVk logic
typedef struct {
    VkInstance instance;
    VkPhysicalDevice phys_dev;
    VkDevice device;
    VkQueue queue;
    uint32_t q_family;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
} MiniVk;

MiniVk* init_vulkan_compute() {
    MiniVk *vk = (MiniVk*)lau_malloc(sizeof(MiniVk));
    if (vk) memset(vk, 0, sizeof(MiniVk));
    else return NULL;

    void *lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!lib) return NULL;
    
    vk->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr");
    if (!vk->vkGetInstanceProcAddr) return NULL;

    PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)vk->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    VkApplicationInfo app = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3, .pApplicationName = "AI_LATENCY" };
    VkInstanceCreateInfo info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app };
    vkCreateInstance(&info, NULL, &vk->instance);
    
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vk->vkGetInstanceProcAddr(vk->instance, "vkEnumeratePhysicalDevices");
    PFN_vkCreateDevice vkCreateDevice = (PFN_vkCreateDevice)vk->vkGetInstanceProcAddr(vk->instance, "vkCreateDevice");
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vk->vkGetInstanceProcAddr(vk->instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    
    uint32_t count = 1;
    vkEnumeratePhysicalDevices(vk->instance, &count, &vk->phys_dev);
    
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk->phys_dev, &qCount, NULL);
    VkQueueFamilyProperties *qProps = (VkQueueFamilyProperties*)lau_malloc(sizeof(VkQueueFamilyProperties) * qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vk->phys_dev, &qCount, qProps);
    for (uint32_t i=0; i<qCount; i++) {
        if (qProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) { vk->q_family = i; break; }
    }
    lau_free(qProps);

    float prio = 1.0f;
    VkDeviceQueueCreateInfo qInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = vk->q_family, .queueCount = 1, .pQueuePriorities = &prio };
    VkDeviceCreateInfo devInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1, .pQueueCreateInfos = &qInfo };
    vkCreateDevice(vk->phys_dev, &devInfo, NULL, &vk->device);
    
    PFN_vkGetDeviceQueue vkGetDeviceQueue = (PFN_vkGetDeviceQueue)vk->vkGetInstanceProcAddr(vk->instance, "vkGetDeviceQueue");
    vkGetDeviceQueue(vk->device, vk->q_family, 0, &vk->queue);
    
    return vk;
}

int main() {
    printf("[AI BENCH] GPU Latency & Timing Correlation\n");
    MiniVk *vk = init_vulkan_compute();
    if (!vk) { printf("Vulkan Init Failed\n"); return 1; }

    // Load Procs
    PFN_vkCreateCommandPool vkCreateCommandPool = (PFN_vkCreateCommandPool)vk->vkGetInstanceProcAddr(vk->instance, "vkCreateCommandPool");
    PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = (PFN_vkAllocateCommandBuffers)vk->vkGetInstanceProcAddr(vk->instance, "vkAllocateCommandBuffers");
    PFN_vkBeginCommandBuffer vkBeginCommandBuffer = (PFN_vkBeginCommandBuffer)vk->vkGetInstanceProcAddr(vk->instance, "vkBeginCommandBuffer");
    PFN_vkEndCommandBuffer vkEndCommandBuffer = (PFN_vkEndCommandBuffer)vk->vkGetInstanceProcAddr(vk->instance, "vkEndCommandBuffer");
    PFN_vkQueueSubmit vkQueueSubmit = (PFN_vkQueueSubmit)vk->vkGetInstanceProcAddr(vk->instance, "vkQueueSubmit");
    PFN_vkQueueWaitIdle vkQueueWaitIdle = (PFN_vkQueueWaitIdle)vk->vkGetInstanceProcAddr(vk->instance, "vkQueueWaitIdle");
    PFN_vkCreateBuffer vkCreateBuffer = (PFN_vkCreateBuffer)vk->vkGetInstanceProcAddr(vk->instance, "vkCreateBuffer");
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = (PFN_vkGetBufferMemoryRequirements)vk->vkGetInstanceProcAddr(vk->instance, "vkGetBufferMemoryRequirements");
    PFN_vkAllocateMemory vkAllocateMemory = (PFN_vkAllocateMemory)vk->vkGetInstanceProcAddr(vk->instance, "vkAllocateMemory");
    PFN_vkBindBufferMemory vkBindBufferMemory = (PFN_vkBindBufferMemory)vk->vkGetInstanceProcAddr(vk->instance, "vkBindBufferMemory");
    PFN_vkCmdFillBuffer vkCmdFillBuffer = (PFN_vkCmdFillBuffer)vk->vkGetInstanceProcAddr(vk->instance, "vkCmdFillBuffer");
    PFN_vkCreateQueryPool vkCreateQueryPool = (PFN_vkCreateQueryPool)vk->vkGetInstanceProcAddr(vk->instance, "vkCreateQueryPool");
    PFN_vkCmdResetQueryPool vkCmdResetQueryPool = (PFN_vkCmdResetQueryPool)vk->vkGetInstanceProcAddr(vk->instance, "vkCmdResetQueryPool");
    PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp = (PFN_vkCmdWriteTimestamp)vk->vkGetInstanceProcAddr(vk->instance, "vkCmdWriteTimestamp");
    PFN_vkGetQueryPoolResults vkGetQueryPoolResults = (PFN_vkGetQueryPoolResults)vk->vkGetInstanceProcAddr(vk->instance, "vkGetQueryPoolResults");
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = (PFN_vkGetPhysicalDeviceProperties)vk->vkGetInstanceProcAddr(vk->instance, "vkGetPhysicalDeviceProperties");

    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(vk->phys_dev, &props);
    float timestamp_period = props.limits.timestampPeriod;
    printf("[AI BENCH] GPU Timestamp Period: %.4f ns\n", timestamp_period);

    // Resources
    VkCommandPool pool;
    VkCommandPoolCreateInfo poolInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, .queueFamilyIndex = vk->q_family, .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT };
    vkCreateCommandPool(vk->device, &poolInfo, NULL, &pool);

    VkCommandBuffer cmd;
    VkCommandBufferAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, .commandPool = pool, .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY, .commandBufferCount = 1 };
    vkAllocateCommandBuffers(vk->device, &allocInfo, &cmd);

    VkQueryPool queryPool;
    VkQueryPoolCreateInfo queryInfo = { .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO, .queryType = VK_QUERY_TYPE_TIMESTAMP, .queryCount = 2 };
    vkCreateQueryPool(vk->device, &queryInfo, NULL, &queryPool);

    // Buffer (128MB)
    size_t size = 128 * 1024 * 1024;
    VkBuffer buffer;
    VkBufferCreateInfo bufInfo = { .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT };
    vkCreateBuffer(vk->device, &bufInfo, NULL, &buffer);
    
    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(vk->device, buffer, &memReqs);
    VkMemoryAllocateInfo memInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = memReqs.size, .memoryTypeIndex = 0 }; // Using Heap 0 (System) or whatever is available, simplistic
    VkDeviceMemory mem;
    vkAllocateMemory(vk->device, &memInfo, NULL, &mem);
    vkBindBufferMemory(vk->device, buffer, mem, 0);

    // --- Benchmark ---
    
    // Warmup
    VkCommandBufferBeginInfo beginInfo = { .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(cmd, &beginInfo);
    vkCmdFillBuffer(cmd, buffer, 0, size, 0x12345678);
    vkEndCommandBuffer(cmd);
    VkSubmitInfo submitInfo = { .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &cmd };
    vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk->queue);

    printf("Warmup Complete. Measuring...\n");

    // Timed Run
    vkBeginCommandBuffer(cmd, &beginInfo);
    vkCmdResetQueryPool(cmd, queryPool, 0, 2);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, queryPool, 0);
    vkCmdFillBuffer(cmd, buffer, 0, size, 0xABCDEF00); // Simulate "Kernel Execution"
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, queryPool, 1);
    vkEndCommandBuffer(cmd);

    unsigned long long cpu_start = get_time_ns();
    vkQueueSubmit(vk->queue, 1, &submitInfo, VK_NULL_HANDLE);
    
    // Wait Loop (CPU side latency)
    // We poll wait idle to catch the exact moment it's done? No, WaitIdle blocks.
    vkQueueWaitIdle(vk->queue);
    unsigned long long cpu_end = get_time_ns();

    uint64_t queries[2];
    vkGetQueryPoolResults(vk->device, queryPool, 0, 2, sizeof(queries), queries, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT);

    double gpu_ns = (queries[1] - queries[0]) * timestamp_period;
    double cpu_ns = (double)(cpu_end - cpu_start);
    double overhead_ns = cpu_ns - gpu_ns;

    printf("[RESULT] Tensor Op Size: %.2f MB\n", (double)size/1024/1024);
    printf("[RESULT] GPU Execution Time: %.3f ms (Throughput: %.2f GB/s)\n", gpu_ns/1e6, ((double)size/1e9)/(gpu_ns/1e9));
    printf("[RESULT] CPU Wall Time:      %.3f ms\n", cpu_ns/1e6);
    printf("[RESULT] Driver Overhead:    %.3f ms\n", overhead_ns/1e6);
    
    if (overhead_ns > 1000000) { // > 1ms overhead is bad
        printf("[WARN] High Driver Overhead detected! Optimize submission batching.\n");
    } else {
        printf("[PASS] Low Driver Overhead.\n");
    }

        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

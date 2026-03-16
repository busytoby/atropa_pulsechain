#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <vulkan/vulkan.h>
#include <dlfcn.h>
#include <immintrin.h>
#include "tsfi_resonance.h"
#include "tsfi_zhao.h"
#include "lau_memory.h"

// Dispatcher task for Zhao scheduler
static void dispatcher_task(void *arg) {
    LauWireThread *wt = (LauWireThread *)arg;
    while (!wt->finished) {
        tsfi_wire_thread_dispatch(wt);
    }
}

// Minimal Vulkan Context (Self-Contained for "Native" Proof)
typedef struct {
    VkInstance instance;
    VkPhysicalDevice phys_dev;
    VkDevice device;
    VkQueue queue;
    uint32_t q_family;
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
} MiniVk;

// --- Load Vulkan ---
MiniVk* init_mini_vulkan() {
    MiniVk *vk = (MiniVk*)lau_malloc(sizeof(MiniVk));
    if (vk) memset(vk, 0, sizeof(MiniVk));
    else return NULL;

    void *lib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
    if (!lib) return NULL;
    
    vk->vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(lib, "vkGetInstanceProcAddr");
    if (!vk->vkGetInstanceProcAddr) return NULL;

    PFN_vkCreateInstance vkCreateInstance = (PFN_vkCreateInstance)vk->vkGetInstanceProcAddr(NULL, "vkCreateInstance");
    
    VkApplicationInfo app = { .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO, .apiVersion = VK_API_VERSION_1_3, .pApplicationName = "RDNA4_NATIVE" };
    VkInstanceCreateInfo info = { .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, .pApplicationInfo = &app };
    
    if (vkCreateInstance(&info, NULL, &vk->instance) != VK_SUCCESS) return NULL;
    
    // Load Instance Procs
    PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = (PFN_vkEnumeratePhysicalDevices)vk->vkGetInstanceProcAddr(vk->instance, "vkEnumeratePhysicalDevices");
    PFN_vkCreateDevice vkCreateDevice = (PFN_vkCreateDevice)vk->vkGetInstanceProcAddr(vk->instance, "vkCreateDevice");
    PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = (PFN_vkGetPhysicalDeviceQueueFamilyProperties)vk->vkGetInstanceProcAddr(vk->instance, "vkGetPhysicalDeviceQueueFamilyProperties");
    
    uint32_t count = 1;
    vkEnumeratePhysicalDevices(vk->instance, &count, &vk->phys_dev);
    
    // Find Queue
    uint32_t qCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vk->phys_dev, &qCount, NULL);
    VkQueueFamilyProperties *qProps = (VkQueueFamilyProperties*)lau_malloc(sizeof(VkQueueFamilyProperties) * qCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vk->phys_dev, &qCount, qProps);
    for (uint32_t i=0; i<qCount; i++) {
        if (qProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT) { vk->q_family = i; break; }
    }
    lau_free(qProps);

    // Create Device
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, .queueFamilyIndex = vk->q_family, .queueCount = 1, .pQueuePriorities = &prio };
    VkDeviceCreateInfo devInfo = { .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, .queueCreateInfoCount = 1, .pQueueCreateInfos = &qInfo };
    
    if (vkCreateDevice(vk->phys_dev, &devInfo, NULL, &vk->device) != VK_SUCCESS) return NULL;
    
    return vk;
}

// --- Benchmark Worker ---
#define TEST_SIZE (256 * 1024 * 1024) // 256 MB
#define THREAD_COUNT 16

typedef struct {
    uint8_t *start;
    size_t size;
} ThreadJob;

void *worker_avx512_stream(void *arg) {
    ThreadJob *job = (ThreadJob*)arg;
    __m512i pattern = _mm512_set1_epi32(0xAAAAAAAA);
    
    // Unroll 8x (512 bytes)
    size_t chunks = job->size / 512;
    uint8_t *d = job->start;

    for (size_t i = 0; i < chunks; i++) {
        uint8_t *base = d + (i * 512);
        _mm512_storeu_si512((void*)(base + 0),   pattern);
        _mm512_storeu_si512((void*)(base + 64),  pattern);
        _mm512_storeu_si512((void*)(base + 128), pattern);
        _mm512_storeu_si512((void*)(base + 192), pattern);
        _mm512_storeu_si512((void*)(base + 256), pattern);
        _mm512_storeu_si512((void*)(base + 320), pattern);
        _mm512_storeu_si512((void*)(base + 384), pattern);
        _mm512_storeu_si512((void*)(base + 448), pattern);
    }
    return NULL;
}

int main() {
    printf("[RDNA4] Native ReBAR (L2/SYS) Validator\n");
    
    MiniVk *vk = init_mini_vulkan();
    if (!vk) { printf("Vulkan Init Failed\n"); return 1; }
    
    // Get Procs
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = (PFN_vkGetPhysicalDeviceMemoryProperties)vk->vkGetInstanceProcAddr(vk->instance, "vkGetPhysicalDeviceMemoryProperties");
    PFN_vkAllocateMemory vkAllocateMemory = (PFN_vkAllocateMemory)vk->vkGetInstanceProcAddr(vk->instance, "vkAllocateMemory");
    PFN_vkMapMemory vkMapMemory = (PFN_vkMapMemory)vk->vkGetInstanceProcAddr(vk->instance, "vkMapMemory");
    PFN_vkUnmapMemory vkUnmapMemory = (PFN_vkUnmapMemory)vk->vkGetInstanceProcAddr(vk->instance, "vkUnmapMemory");
    PFN_vkFreeMemory vkFreeMemory = (PFN_vkFreeMemory)vk->vkGetInstanceProcAddr(vk->instance, "vkFreeMemory");

    VkPhysicalDeviceMemoryProperties memProps;
    vkGetPhysicalDeviceMemoryProperties(vk->phys_dev, &memProps);
    
    // Find ReBAR Type
    int typeIndex = -1;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++) {
        VkMemoryPropertyFlags f = memProps.memoryTypes[i].propertyFlags;
        // MUST be DEVICE_LOCAL and HOST_VISIBLE
        if ((f & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) && (f & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)) {
             // Heuristic: Must be the large heap (VRAM), not the small GART heap
             if (memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size > 1024ull * 1024 * 1024) {
                 typeIndex = i;
                 printf("Found ReBAR Memory Type: %d (Heap Size: %.2f GB)\n", i, 
                     (double)memProps.memoryHeaps[memProps.memoryTypes[i].heapIndex].size / (1024*1024*1024));
                 break;
             }
        }
    }
    
    if (typeIndex == -1) {
        printf("[FAIL] No ReBAR (Device Local + Host Visible) memory type found.\n");
        return 1;
    }

    // Allocate
    VkMemoryAllocateInfo allocInfo = { .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, .allocationSize = TEST_SIZE, .memoryTypeIndex = typeIndex };
    VkDeviceMemory mem;
    if (vkAllocateMemory(vk->device, &allocInfo, NULL, &mem) != VK_SUCCESS) {
        printf("[FAIL] Allocation failed.\n");
        return 1;
    }
    
    // Map
    void *ptr;
    if (vkMapMemory(vk->device, mem, 0, TEST_SIZE, 0, &ptr) != VK_SUCCESS) {
        printf("[FAIL] Map failed.\n");
        return 1;
    }
    
    printf("[SUCCESS] Allocated %d MB in VRAM via ReBAR.\n", TEST_SIZE/1024/1024);
    
    // Check Alignment for WMMA
    if (((uintptr_t)ptr % 128) == 0) {
        printf("[INFO] Pointer Alignment: 128+ bytes (Compatible with RDNA4 WMMA/Block Loads)\n");
    } else {
        printf("[WARN] Pointer Alignment < 128 bytes. WMMA performance may degrade.\n");
    }
    
    // Write Bandwidth Test (Multi-threaded)
    printf("Benchmarking Write Speed (CPU -> VRAM) with %d Threads...\n", THREAD_COUNT);
    
    struct timespec ts_start, ts_end;
    
    LauWireThread *threads[THREAD_COUNT];
    ThreadJob jobs[THREAD_COUNT];
    size_t chunk_size = TEST_SIZE / THREAD_COUNT;
    
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    
    tsfi_zhao_init(THREAD_COUNT);
    
    for (int i = 0; i < THREAD_COUNT; i++) {
        jobs[i].start = (uint8_t*)ptr + (i * chunk_size);
        jobs[i].size = chunk_size;
        threads[i] = tsfi_wire_thread_create(i, 1.0);
        threads[i]->start_routine = (void*(*)(void*))worker_avx512_stream;
        threads[i]->arg = &jobs[i];
        tsfi_zhao_submit(dispatcher_task, threads[i]);
    }

    tsfi_zhao_sync();
    
    _mm_sfence(); // Ensure all WC buffers are flushed
    
    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    
    double seconds = (ts_end.tv_sec - ts_start.tv_sec) + 
                     (ts_end.tv_nsec - ts_start.tv_nsec) / 1000000000.0;
    
    double gb = (double)TEST_SIZE / (1024.0 * 1024.0 * 1024.0);
    double bw = gb / seconds;
    
    printf("Write Bandwidth: ~%.2f GB/s (Wall Clock)\n", bw);
    
    tsfi_zhao_shutdown();
    for (int i = 0; i < THREAD_COUNT; i++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)threads[i] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL;
        lau_free(threads[i]);
    }
    
    // Verification
    volatile uint8_t *vptr = (uint8_t*)ptr;
    if (vptr[0] != 0xAA || vptr[TEST_SIZE-1] != 0xAA) {
        printf("[FAIL] Data verification failed!\n");
    } else {
        printf("[PASS] Data verification success.\n");
    }

    vkUnmapMemory(vk->device, mem);
    vkFreeMemory(vk->device, mem, NULL);
    
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}
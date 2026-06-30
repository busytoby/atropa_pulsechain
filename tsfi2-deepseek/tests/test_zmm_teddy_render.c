#include "tsfi_spirv_kernels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vulkan/vulkan.h>

// Struct matching shader ZMMRegisters block
typedef struct {
    float basePhase;
    float channelFreq;
    float signalAmp;
    float poleSteer;
    float foundationRail;
    float chinRail;
    float rBeta;
    float rCutoff;
    float accNPN;
    float accPNP;
} ZMMRegs;

// Struct matching shader OutputBuffer elements
typedef struct {
    float x, y, z, w;
    float r, g, b, a;
} ShaderVertex;

// Declarations from our Vulkan compute driver
extern void tsfi_zmm_dispatch_traverse(
    VkDevice device,
    VkQueue queue,
    VkCommandPool cmdPool,
    VkBuffer regBuffer,
    VkBuffer weightBuffer,
    VkBuffer outputBuffer,
    uint32_t workgroupCount
);

// Helper to create and allocate Vulkan Buffer
static void createBuffer(
    VkDevice device,
    VkPhysicalDevice physDev,
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties,
    VkBuffer* buffer,
    VkDeviceMemory* bufferMemory
) {
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCreateBuffer(device, &bufferInfo, NULL, buffer);

    VkMemoryRequirements memReqs;
    vkGetBufferMemoryRequirements(device, *buffer, &memReqs);

    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDev, &memProperties);

    uint32_t memTypeIndex = (uint32_t)(-1);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) && 
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            memTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size,
        .memoryTypeIndex = memTypeIndex
    };
    vkAllocateMemory(device, &allocInfo, NULL, bufferMemory);
    vkBindBufferMemory(device, *buffer, *bufferMemory, 0);
}

int main() {
    printf("[REAL ZMM TEST] Initializing local Vulkan device driver...\n");

    // 1. Initialize Vulkan Instance
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "ZMM Teddy Render",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "ZMM Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0
    };
    VkInstanceCreateInfo instInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo
    };
    VkInstance instance;
    if (vkCreateInstance(&instInfo, NULL, &instance) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create Vulkan instance\n");
        return -1;
    }

    // 2. Select Physical Device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);
    if (deviceCount == 0) {
        fprintf(stderr, "No Vulkan compatible GPUs found\n");
        return -1;
    }
    VkPhysicalDevice* physDevs = malloc(sizeof(VkPhysicalDevice) * deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, physDevs);
    VkPhysicalDevice physDev = physDevs[0];
    free(physDevs);

    // 3. Create Logical Device and Queue
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = 0,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };
    VkDeviceCreateInfo devInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo
    };
    VkDevice device;
    if (vkCreateDevice(physDev, &devInfo, NULL, &device) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create logical device\n");
        return -1;
    }

    VkQueue queue;
    vkGetDeviceQueue(device, 0, 0, &queue);

    // 4. Create Command Pool
    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT
    };
    VkCommandPool cmdPool;
    vkCreateCommandPool(device, &poolInfo, NULL, &cmdPool);

    // 5. Setup Input Data Buffers
    // Register Buffer (Input Registers)
    VkBuffer regBuffer;
    VkDeviceMemory regMemory;
    createBuffer(device, physDev, sizeof(ZMMRegs), 
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &regBuffer, &regMemory);

    ZMMRegs regs = {
        .basePhase = 0.5f,
        .channelFreq = 300.0f,
        .signalAmp = 1.0f,
        .poleSteer = 50.0f,
        .foundationRail = 1200.0f,
        .chinRail = 600.0f,
        .rBeta = 20.0f,     // High Gain beta bias point
        .rCutoff = 30.0f,   // Low cutoff threshold
        .accNPN = 45.0f,
        .accPNP = 15.0f
    };
    void* dataPtr;
    vkMapMemory(device, regMemory, 0, sizeof(ZMMRegs), 0, &dataPtr);
    memcpy(dataPtr, &regs, sizeof(ZMMRegs));
    vkUnmapMemory(device, regMemory);

    // Simulated local GGUF Weight Manifold Buffer (256KB of float weights)
    uint32_t weightCount = 65536;
    VkBuffer weightBuffer;
    VkDeviceMemory weightMemory;
    createBuffer(device, physDev, sizeof(float) * weightCount, 
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &weightBuffer, &weightMemory);

    float* simulatedWeights = malloc(sizeof(float) * weightCount);
    for (uint32_t i = 0; i < weightCount; i++) {
        // Generate continuous high-frequency weight fluctuations mimicking a model's parameters
        simulatedWeights[i] = sinf((float)i * 0.05f) * cosf((float)i * 0.01f);
    }
    vkMapMemory(device, weightMemory, 0, sizeof(float) * weightCount, 0, &dataPtr);
    memcpy(dataPtr, simulatedWeights, sizeof(float) * weightCount);
    vkUnmapMemory(device, weightMemory);
    free(simulatedWeights);

    // Output Vertex Buffer (For storing projected coordinates)
    uint32_t vertexCount = 1024;
    VkBuffer outputBuffer;
    VkDeviceMemory outputMemory;
    createBuffer(device, physDev, sizeof(ShaderVertex) * vertexCount, 
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &outputBuffer, &outputMemory);

    // 6. Dispatch GPU Compute Shader
    printf("[REAL ZMM TEST] Submitting zmm_traverse compute pipeline to GPU queue...\n");
    tsfi_zmm_dispatch_traverse(device, queue, cmdPool, regBuffer, weightBuffer, outputBuffer, vertexCount / 256);
    printf("[REAL ZMM TEST] Compute pass completed.\n");

    // 7. Retrieve coordinates and render local PPM Image of the Teddy Bear
    vkMapMemory(device, outputMemory, 0, sizeof(ShaderVertex) * vertexCount, 0, &dataPtr);
    ShaderVertex* vertices = (ShaderVertex*)dataPtr;

    // Rasterize points to a 2D grid image (256x256 pixels)
    int imgDim = 256;
    uint8_t* ppmPixels = calloc(imgDim * imgDim * 3, 1);

    for (uint32_t i = 0; i < vertexCount; i++) {
        // Project 3D coordinate space onto 2D image plane
        int px = (int)((vertices[i].x / 10.0f + 0.5f) * (float)imgDim);
        int py = (int)((vertices[i].y / 10.0f + 0.5f) * (float)imgDim);

        if (px >= 0 && px < imgDim && py >= 0 && py < imgDim) {
            int idx = (px + py * imgDim) * 3;
            ppmPixels[idx] = (uint8_t)(vertices[i].r * 255.0f);     // Red channel
            ppmPixels[idx + 1] = (uint8_t)(vertices[i].g * 255.0f); // Green channel
            ppmPixels[idx + 2] = (uint8_t)(vertices[i].b * 255.0f); // Blue channel
        }
    }
    vkUnmapMemory(device, outputMemory);

    // Write to PPM file
    const char* filename = "teddy_bear_output.ppm";
    FILE* f = fopen(filename, "wb");
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", imgDim, imgDim);
        fwrite(ppmPixels, 1, imgDim * imgDim * 3, f);
        fclose(f);
        printf("[SUCCESS] Rendered local image: %s\n", filename);
    } else {
        fprintf(stderr, "Failed to write image output file\n");
    }

    free(ppmPixels);

    // Cleanup Vulkan Resources
    vkDestroyBuffer(device, regBuffer, NULL);
    vkFreeMemory(device, regMemory, NULL);
    vkDestroyBuffer(device, weightBuffer, NULL);
    vkFreeMemory(device, weightMemory, NULL);
    vkDestroyBuffer(device, outputBuffer, NULL);
    vkFreeMemory(device, outputMemory, NULL);
    vkDestroyCommandPool(device, cmdPool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}

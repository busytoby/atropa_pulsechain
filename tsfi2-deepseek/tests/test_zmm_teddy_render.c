#define _GNU_SOURCE
#include "tsfi_spirv_kernels.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <vulkan/vulkan.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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

// Bresenham's Line Drawing Algorithm to connect wireframe vertices
static void drawLine(uint8_t* pixels, int w, int h, int x0, int y0, int x1, int y1, uint8_t r, uint8_t g, uint8_t b) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        if (x0 >= 0 && x0 < w && y0 >= 0 && y0 < h) {
            int idx = (x0 + y0 * w) * 3;
            pixels[idx] = r;
            pixels[idx + 1] = g;
            pixels[idx + 2] = b;
        }
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

int main() {
    printf("[REAL ZMM TEST] Initializing Vulkan device driver...\n");

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "ZMM Teddy Coherent Render",
        .apiVersion = VK_API_VERSION_1_0
    };
    VkInstanceCreateInfo instInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &appInfo
    };
    VkInstance instance;
    vkCreateInstance(&instInfo, NULL, &instance);

    uint32_t devCount = 1;
    VkPhysicalDevice physDev;
    vkEnumeratePhysicalDevices(instance, &devCount, &physDev);

    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueCount = 1,
        .pQueuePriorities = &queuePriority
    };
    VkDeviceCreateInfo devInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo
    };
    VkDevice device;
    vkCreateDevice(physDev, &devInfo, NULL, &device);

    VkQueue queue;
    vkGetDeviceQueue(device, 0, 0, &queue);

    VkCommandPoolCreateInfo poolInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .queueFamilyIndex = 0
    };
    VkCommandPool cmdPool;
    vkCreateCommandPool(device, &poolInfo, NULL, &cmdPool);

    // Buffers allocation
    VkBuffer regBuffer;
    VkDeviceMemory regMemory;
    createBuffer(device, physDev, sizeof(ZMMRegs), 
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &regBuffer, &regMemory);

    uint32_t weightCount = 65536;
    VkBuffer weightBuffer;
    VkDeviceMemory weightMemory;
    createBuffer(device, physDev, sizeof(float) * weightCount, 
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &weightBuffer, &weightMemory);

    float* simulatedWeights = malloc(sizeof(float) * weightCount);
    for (uint32_t i = 0; i < weightCount; i++) {
        // Continuous coherent manifold structure (no random noise)
        simulatedWeights[i] = sinf((float)i * 0.01f) * cosf((float)i * 0.003f) * 0.5f;
    }
    void* dataPtr;
    vkMapMemory(device, weightMemory, 0, sizeof(float) * weightCount, 0, &dataPtr);
    memcpy(dataPtr, simulatedWeights, sizeof(float) * weightCount);
    vkUnmapMemory(device, weightMemory);
    free(simulatedWeights);

    uint32_t vertexCount = 1024;
    VkBuffer outputBuffer;
    VkDeviceMemory outputMemory;
    createBuffer(device, physDev, sizeof(ShaderVertex) * vertexCount, 
                 VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 
                 &outputBuffer, &outputMemory);

    // PPM output image allocation
    int imgDim = 512;
    uint8_t* ppmPixels = calloc(imgDim * imgDim * 3, 1);

    // Initial register states
    ZMMRegs regs = {
        .basePhase = 0.0f,
        .channelFreq = 300.0f,
        .signalAmp = 1.0f,
        .poleSteer = 0.0f,
        .foundationRail = 1200.0f,
        .chinRail = 600.0f,
        .rBeta = 10.0f,     // High initial gain
        .rCutoff = 20.0f    // Low initial cutoff
    };

    printf("[REAL ZMM TRAVERSAL] Running multi-frame loop modulating resistors and impedance...\n");

    // We run 32 sweeps of phase accumulation to stitch the dots into continuous wireframe lines
    int numSweeps = 32;
    for (int frame = 0; frame < numSweeps; frame++) {
        // Dynamic impedance modulation: change phase and frequency based on frame index
        regs.basePhase = (float)frame * (2.0f * M_PI / (float)numSweeps);
        
        // Modulate resistors dynamically across the sweeps to sweep the threshold
        regs.rBeta = 10.0f + sinf((float)frame * 0.5f) * 5.0f;
        regs.rCutoff = 20.0f + cosf((float)frame * 0.5f) * 10.0f;

        // Modulate the dual accumulators representing charge transport
        regs.accNPN = 50.0f + 25.0f * sinf((float)frame * 0.2f);
        regs.accPNP = 50.0f + 25.0f * cosf((float)frame * 0.2f);

        // Map registers to Vulkan buffer
        vkMapMemory(device, regMemory, 0, sizeof(ZMMRegs), 0, &dataPtr);
        memcpy(dataPtr, &regs, sizeof(ZMMRegs));
        vkUnmapMemory(device, regMemory);

        // Dispatch compute shader
        tsfi_zmm_dispatch_traverse(device, queue, cmdPool, regBuffer, weightBuffer, outputBuffer, vertexCount / 256);

        // Map output vertices to draw
        vkMapMemory(device, outputMemory, 0, sizeof(ShaderVertex) * vertexCount, 0, &dataPtr);
        ShaderVertex* vertices = (ShaderVertex*)dataPtr;

        // Connect the vertices with lines to form the coherent wireframe geometry
        for (uint32_t i = 0; i < vertexCount - 1; i++) {
            // Screen projection scaling
            int x0 = (int)((vertices[i].x / 14.0f + 0.5f) * (float)imgDim);
            int y0 = (int)((vertices[i].y / 14.0f + 0.5f) * (float)imgDim);
            int x1 = (int)((vertices[i + 1].x / 14.0f + 0.5f) * (float)imgDim);
            int y1 = (int)((vertices[i + 1].y / 14.0f + 0.5f) * (float)imgDim);

            // Connect consecutive vertices sharing same group
            if (abs(x0 - x1) < 100 && abs(y0 - y1) < 100) {
                uint8_t r = (uint8_t)(vertices[i].r * 255.0f);
                uint8_t g = (uint8_t)(vertices[i].g * 255.0f);
                uint8_t b = (uint8_t)(vertices[i].b * 255.0f);
                drawLine(ppmPixels, imgDim, imgDim, x0, y0, x1, y1, r, g, b);
            }
        }
        vkUnmapMemory(device, outputMemory);
    }

    // Write final accumulated wireframe mesh to file
    const char* filename = "teddy_bear_output.ppm";
    FILE* f = fopen(filename, "wb");
    if (f) {
        fprintf(f, "P6\n%d %d\n255\n", imgDim, imgDim);
        fwrite(ppmPixels, 1, imgDim * imgDim * 3, f);
        fclose(f);
        printf("[SUCCESS] Coherent wireframe bear rendered: %s\n", filename);
    }

    free(ppmPixels);

    // Cleanup resources
    vkDestroyBuffer(device, regBuffer, NULL);
    vkFreeMemory(device, regMemory, NULL);
    vkDestroyBuffer(device, weightBuffer, NULL);
    vkFreeMemory(device, weightMemory, NULL);
    vkDestroyBuffer(device, outputBuffer, NULL);
    vkFreeMemory(device, outputMemory, NULL);
    vkDestroyCommandPool(device, cmdPool, NULL);
    vkDeviceWaitIdle(device);
    vkDestroyDevice(device, NULL);
    vkDestroyInstance(instance, NULL);

    return 0;
}

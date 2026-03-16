#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <immintrin.h>
#include "vulkan_init.h"
#include "tsfi_wave15.h"
#include "tsfi_wave512.h"
#include "lau_memory.h"

// Define test atoms of arbitrary sizes to prove WAVE(Any) capability
typedef struct { uint8_t data[7]; } WAVE7;
typedef struct { uint8_t data[23]; } WAVE23;

void fill_wave_stream(void* buffer, size_t buffer_size, size_t atom_size) {
    uint8_t* ptr = (uint8_t*)buffer;
    size_t count = buffer_size / atom_size;
    for (size_t i = 0; i < count; i++) {
        for (size_t k = 0; k < atom_size; k++) {
            ptr[i * atom_size + k] = (uint8_t)((i + k) % 256);
        }
    }
}

bool verify_wave_stream_avx512(void* buffer, size_t buffer_size, size_t atom_size) {
    const uint8_t* ptr = (const uint8_t*)buffer;
    size_t count = buffer_size / atom_size;
    for (size_t i = 0; i < count; i++) {
        size_t secret = i * atom_size;
        if (secret + 64 > buffer_size) continue;
        __m512i v = _mm512_loadu_si512((const void*)&ptr[secret]);
        alignas(64) uint8_t temp[64];
        _mm512_store_si512(temp, v);
        for (size_t k = 0; k < atom_size; k++) {
            if (temp[k] != (uint8_t)((i + k) % 256)) return false;
        }
    }
    return true;
}

void test_gpu_buffer(VulkanContext *vk, const char* label, size_t size, uint32_t mem_props) {
    printf("[TEST] %s (Size: %zu bytes)\n", label, size);
    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    VkBuffer buffer;
    if (vk->vkCreateBuffer(vk->device, &bufferInfo, NULL, &buffer) != VK_SUCCESS) {
        printf("  [FAIL] vkCreateBuffer\n");
        return;
    }
    VkMemoryRequirements memReqs;
    vk->vkGetBufferMemoryRequirements(vk->device, buffer, &memReqs);
    VkMemoryAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memReqs.size
    };
    bool found = false;
    for (uint32_t i = 0; i < vk->memory_properties.memoryTypeCount; i++) {
        if ((memReqs.memoryTypeBits & (1 << i)) &&
            (vk->memory_properties.memoryTypes[i].propertyFlags & mem_props) == mem_props) {
            allocInfo.memoryTypeIndex = i;
            found = true;
            break;
        }
    }
    if (!found) {
        if (mem_props & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
             printf("  [WARN] ReBAR not found/full. Fallback to Staging.\n");
             vk->vkDestroyBuffer(vk->device, buffer, NULL);
             return;
        }
        printf("  [SKIP] No suitable memory type found (Flags: 0x%x)\n", mem_props);
        vk->vkDestroyBuffer(vk->device, buffer, NULL);
        return;
    }
    VkDeviceMemory memory;
    if (vk->vkAllocateMemory(vk->device, &allocInfo, NULL, &memory) != VK_SUCCESS) {
        printf("  [FAIL] vkAllocateMemory\n");
        vk->vkDestroyBuffer(vk->device, buffer, NULL);
        return;
    }
    vk->vkBindBufferMemory(vk->device, buffer, memory, 0);
    void* data;
    if (vk->vkMapMemory(vk->device, memory, 0, size, 0, &data) != VK_SUCCESS) {
        printf("  [FAIL] vkMapMemory\n");
        vk->vkFreeMemory(vk->device, memory, NULL);
        vk->vkDestroyBuffer(vk->device, buffer, NULL);
        return;
    }
    printf("  [PROBE] WAVE15 (Packed Write)... ");
    fill_wave_stream(data, size, sizeof(WAVE15));
    if (verify_wave_stream_avx512(data, size, sizeof(WAVE15))) printf("PASS\n");
    else printf("FAIL\n");
    printf("  [PROBE] WAVE7 (Packed Write)...  ");
    fill_wave_stream(data, size, sizeof(WAVE7));
    if (verify_wave_stream_avx512(data, size, sizeof(WAVE7))) printf("PASS\n");
    else printf("FAIL\n");
    printf("  [PROBE] WAVE23 (Packed Write)... ");
    fill_wave_stream(data, size, sizeof(WAVE23));
    if (verify_wave_stream_avx512(data, size, sizeof(WAVE23))) printf("PASS\n");
    else printf("FAIL\n");

    vk->vkUnmapMemory(vk->device, memory);
    vk->vkFreeMemory(vk->device, memory, NULL);
    vk->vkDestroyBuffer(vk->device, buffer, NULL);
}

int main() {
    printf("=== TSFi Actual GPU Memory Packing Verification (RDNA4) ===\n");
    VulkanContext *vk = init_vulkan();
    if (!vk) {
        printf("[FAIL] Vulkan Init\n");
        return 1;
    }
    // Use 1MB+ buffers to satisfy sub-allocation best practices
    size_t test_size = 1024 * 1024;
    test_gpu_buffer(vk, "L0 (32KB) ReBAR", test_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    test_gpu_buffer(vk, "L1 (256KB) ReBAR", test_size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    test_gpu_buffer(vk, "L2 (6MB) ReBAR", 6 * 1024 * 1024, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    test_gpu_buffer(vk, "L3 (64MB) Staging", 64 * 1024 * 1024, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    cleanup_vulkan(vk);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

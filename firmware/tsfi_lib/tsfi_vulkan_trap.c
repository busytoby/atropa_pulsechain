#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>

#define FRAME_WIDTH 800
#define FRAME_HEIGHT 600
#define PIXEL_SIZE 4 // RGBA is 4 bytes
#define FRAME_BUFFER_SIZE (FRAME_WIDTH * FRAME_HEIGHT * PIXEL_SIZE)

typedef struct {
    uint8_t guest_vram[FRAME_BUFFER_SIZE] __attribute__((aligned(64)));
    uint32_t frame_index;
    bool is_locked;
    pthread_mutex_t mutex;
} VulkanFrameTrap;

void vulkan_trap_init(VulkanFrameTrap *trap) {
    memset(trap->guest_vram, 0, FRAME_BUFFER_SIZE);
    trap->frame_index = 0;
    trap->is_locked = false;
    pthread_mutex_init(&trap->mutex, NULL);
}

// Trap a Vulkan frame write, copying raw RGBA pixels directly to guest memory space
bool vulkan_trap_frame(VulkanFrameTrap *trap, const void *pixels, size_t size) {
    pthread_mutex_lock(&trap->mutex);
    if (trap->is_locked) {
        pthread_mutex_unlock(&trap->mutex);
        return false; // Dropped frame (guest is currently reading)
    }
    
    size_t copy_size = size < FRAME_BUFFER_SIZE ? size : FRAME_BUFFER_SIZE;
    memcpy(trap->guest_vram, pixels, copy_size);
    trap->frame_index++;
    
    pthread_mutex_unlock(&trap->mutex);
    return true;
}

// Guest locks the frame buffer to perform image transform/analysis without shearing
const uint8_t* vulkan_trap_lock_guest_frame(VulkanFrameTrap *trap) {
    pthread_mutex_lock(&trap->mutex);
    trap->is_locked = true;
    pthread_mutex_unlock(&trap->mutex);
    return trap->guest_vram;
}

void vulkan_trap_unlock_guest_frame(VulkanFrameTrap *trap) {
    pthread_mutex_lock(&trap->mutex);
    trap->is_locked = false;
    pthread_mutex_unlock(&trap->mutex);
}

void vulkan_trap_destroy(VulkanFrameTrap *trap) {
    pthread_mutex_destroy(&trap->mutex);
}

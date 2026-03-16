#ifndef TSFI_SD_THUNK_H
#define TSFI_SD_THUNK_H

#include <stdint.h>
#include <stdbool.h>
#include <vulkan/vulkan.h>

// Forward declarations for proprietary TSFi Memory limits
#include "lau_memory.h"
#include "tsfi_zhao.h"

#include "tsfi_safetensors_cache.h"

// The Absolute ReBAR Tensor Matrix
typedef struct {
    TsfiSafetensorsAsset *asset;   // High-Speed SHM Cache
    uint8_t* raw_safetensors;      // Mapped via lau_malloc_wired (Memory Type 3 - Host Visible)
    size_t total_mass_bytes;
    
    // Vulkan Structural Equivalents
    VkBuffer unet_buffer_vk;       // The ReBAR Zero-Copy UNet Tensor buffer
    VkDeviceMemory unet_memory_vk; // Bound to tsfi_zhao timeline semaphores
    
    // Textural Shaders (VAE Decode)
    VkPipeline vae_compute_pipeline;
    VkPipelineLayout vae_pipeline_layout;
    
    // Asynchronous Handshakes
    VkSemaphore tsfi_zhong_timeline_sem;
    uint64_t current_inference_tick;
} TsfiSdContext;

// The Primary Interface
// 1. Instantiates the Stable Diffusion Matrix natively inside the TSFi Wavelet Arena
bool tsfi_sd_thunk_init(TsfiSdContext* ctx, const char* safetensors_path);

// 2. The Zero-Copy Execution Thunk
// Takes the DNA Geometric mask, runs the UNet Graph natively, and dumps RGB pixels via Vulkan Textural Shaders
void tsfi_sd_thunk_paint_frame(TsfiSdContext* ctx, const uint8_t* in_dna_mask, uint8_t* out_pixels, int w, int h);

// 3. Graceful Memory Annihilation
void tsfi_sd_thunk_teardown(TsfiSdContext* ctx);

#endif // TSFI_SD_THUNK_H

#ifndef TSFI_SAFETENSORS_CACHE_H
#define TSFI_SAFETENSORS_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct {
    char path[512];
    void *data;
    size_t size;
    int shm_fd;
    uint32_t ref_count;
} TsfiSafetensorsAsset;

// High-Speed Attachment: Returns a pointer to the model data.
// If the model is already in SHM/ReBAR, it attaches instantly (Zero-Copy).
// Otherwise, it performs the initial load and caches it.
TsfiSafetensorsAsset* tsfi_safetensors_cache_attach(const char *path);

// Decrements ref count and detaches if zero.
void tsfi_safetensors_cache_detach(TsfiSafetensorsAsset *asset);

// Zero-Copy Injection: Allows a pre-loaded memory block to be registered as a path.
void tsfi_safetensors_cache_inject(const char *path, void *data, size_t size);

// Global Stats
size_t tsfi_safetensors_cache_total_bytes(void);

// Sovereign Secret Extraction
void tsfi_sovereign_hydrate_from_taesd(TsfiSafetensorsAsset *asset, float *melanin, float *roughness, float *iridescence);

#endif // TSFI_SAFETENSORS_CACHE_H

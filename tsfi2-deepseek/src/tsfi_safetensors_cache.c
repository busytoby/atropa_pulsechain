#include "tsfi_safetensors_cache.h"
#include "lau_registry.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <errno.h>

#define MAX_ASSETS 16
static TsfiSafetensorsAsset *g_assets[MAX_ASSETS] = {0};
static _Atomic int g_cache_lock = 0;

static inline void cache_lock(void) {
    while (atomic_exchange_explicit(&g_cache_lock, 1, memory_order_acquire)) {
        __builtin_ia32_pause();
    }
}

static inline void cache_unlock(void) {
    atomic_store_explicit(&g_cache_lock, 0, memory_order_release);
}

static char* path_to_shm_name(const char *path) {
    static char shm_name[256];
    const char *filename = strrchr(path, '/');
    if (filename) filename++; else filename = path;
    snprintf(shm_name, sizeof(shm_name), "/tsfi_safetensors_%s", filename);
    // Replace dots with underscores for shm_open compliance on some systems
    for (int i = 0; shm_name[i]; i++) {
        if (shm_name[i] == '.') shm_name[i] = '_';
    }
    return shm_name;
}

#include <math.h>
#include "tsfi_c_math.h"

void tsfi_sovereign_hydrate_from_taesd(TsfiSafetensorsAsset *asset, float *melanin, float *roughness, float *iridescence) {
    if (!asset || !asset->data) return;

    // TAESD 1.weight: offset 256, size 9216. 
    // Data starts after 8 bytes (header size) + 5480 bytes (JSON header)
    float *tensor_data = (float *)((char *)asset->data + 8 + 5480 + 256);
    
    float mean_mag = 0.0f;
    for (int i = 0; i < 1024; i++) {
        mean_mag += fabsf(tensor_data[i]);
    }
    mean_mag /= 1024.0f;

    float variance = 0.0f;
    for (int i = 0; i < 1024; i++) {
        float diff = fabsf(tensor_data[i]) - mean_mag;
        variance += diff * diff;
    }
    variance /= 1024.0f;

    // Sovereign Parameter Mapping
    *melanin = mean_mag * (float)TSFI_SECRET_CORE;
    *roughness = fmaxf(0.1f, variance * 10.0f);
    *iridescence = sinf(mean_mag * (float)TSFI_TAU);
    
    printf("[SOVEREIGN] Secrets extracted from TAESD: Melanin=%.3f, Roughness=%.3f, Irid=%.3f\n", 
           *melanin, *roughness, *iridescence);
}

TsfiSafetensorsAsset* tsfi_safetensors_cache_attach(const char *path) {
    if (!path) return NULL;
    cache_lock();

    // 1. Search existing cache
    for (int i = 0; i < MAX_ASSETS; i++) {
        if (g_assets[i] && strcmp(g_assets[i]->path, path) == 0) {
            g_assets[i]->ref_count++;
            cache_unlock();
            return g_assets[i];
        }
    }

    // 2. Try to attach to existing SHM (from another process)
    char *shm_name = path_to_shm_name(path);
    int fd = shm_open(shm_name, O_RDWR, 0666);
    size_t size = 0;
    void *data = NULL;

    if (fd != -1) {
        struct stat sb;
        if (fstat(fd, &sb) == 0) {
            size = sb.st_size;
            data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        }
    } else {
        // 3. Perform Initial Load
        int file_fd = open(path, O_RDONLY);
        if (file_fd < 0) {
            fprintf(stderr, "[FRACTURE] Safetensors asset missing: %s\n", path);
            cache_unlock();
            return NULL;
        }

        struct stat sb;
        fstat(file_fd, &sb);
        size = sb.st_size;

        fd = shm_open(shm_name, O_RDWR | O_CREAT | O_EXCL, 0666);
        if (fd < 0 && errno == EEXIST) {
            // Raced with another process, try opening again
            close(file_fd);
            cache_unlock();
            return tsfi_safetensors_cache_attach(path);
        }
        
        if (fd < 0) {
            perror("shm_open create");
            close(file_fd);
            cache_unlock();
            return NULL;
        }

        if (ftruncate(fd, size) == -1) {
            perror("ftruncate shm");
            close(file_fd); close(fd);
            cache_unlock();
            return NULL;
        }

        data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap shm");
            close(file_fd); close(fd);
            cache_unlock();
            return NULL;
        }

        // Pull into SHM
        printf("[CACHE] Loading %s into SHM Matrix (%zu bytes)...\n", path, size);
        ssize_t n = read(file_fd, data, size);
        if (n < (ssize_t)size) {
            perror("read into shm");
            munmap(data, size); close(fd); close(file_fd);
            cache_unlock();
            return NULL;
        }
        close(file_fd);
    }

    if (!data || data == MAP_FAILED) {
        if (fd != -1) close(fd);
        cache_unlock();
        return NULL;
    }

    // 4. Store in Cache
    for (int i = 0; i < MAX_ASSETS; i++) {
        if (!g_assets[i]) {
            g_assets[i] = malloc(sizeof(TsfiSafetensorsAsset));
            strncpy(g_assets[i]->path, path, 511);
            g_assets[i]->data = data;
            g_assets[i]->size = size;
            g_assets[i]->shm_fd = fd;
            g_assets[i]->ref_count = 1;
            
            // Register with LauRegistry (Zero-Copy Provenance)
            // Note: We use the SHM address as the physical payload
            LauMetadata md = {0};
            md.actual_start = data;
            md.alloc_size = size | (1ULL << 55); // Mark as SEALED (Weights are read-only-ish)
            md.physical_tier = 3; // Device Local + Host Visible (ReBAR)
            lau_registry_insert(&md);
            
            cache_unlock();
            return g_assets[i];
        }
    }

    munmap(data, size);
    close(fd);
    cache_unlock();
    return NULL;
}

void tsfi_safetensors_cache_detach(TsfiSafetensorsAsset *asset) {
    if (!asset) return;
    cache_lock();
    asset->ref_count--;
    if (asset->ref_count == 0) {
        // We keep it in the global g_assets for instant re-attachment
        // It's only truly freed on process exit or explicit purge
    }
    cache_unlock();
}

void tsfi_safetensors_cache_inject(const char *path, void *data, size_t size) {
    if (!path || !data || size == 0) return;
    cache_lock();
    for (int i = 0; i < MAX_ASSETS; i++) {
        if (g_assets[i] && strcmp(g_assets[i]->path, path) == 0) {
            // Already injected or loaded
            cache_unlock();
            return;
        }
    }
    for (int i = 0; i < MAX_ASSETS; i++) {
        if (!g_assets[i]) {
            g_assets[i] = malloc(sizeof(TsfiSafetensorsAsset));
            strncpy(g_assets[i]->path, path, 511);
            g_assets[i]->data = data;
            g_assets[i]->size = size;
            g_assets[i]->shm_fd = -1;
            g_assets[i]->ref_count = 1;
            cache_unlock();
            return;
        }
    }
    cache_unlock();
}

size_t tsfi_safetensors_cache_total_bytes(void) {
    size_t total = 0;
    cache_lock();
    for (int i = 0; i < MAX_ASSETS; i++) {
        if (g_assets[i]) total += g_assets[i]->size;
    }
    cache_unlock();
    return total;
}

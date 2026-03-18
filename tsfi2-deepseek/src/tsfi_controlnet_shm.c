#include "tsfi_controlnet_shm.h"
#include "lau_memory.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

static uint64_t get_time_ns() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

TsfiControlNetMap* tsfi_cn_shm_create(const char *name) {
    int fd = shm_open(name, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return NULL;
    size_t size = sizeof(TsfiControlNetMap);
    if (ftruncate(fd, size) == -1) { close(fd); return NULL; }
    
    // Wire the SHM file into the LauRegistryManifold
    TsfiControlNetMap *map = lau_malloc_wired(size); 
    if (map) {
        void *raw_map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (raw_map != MAP_FAILED) {
            // We copy from tracked memory to the actual SHM file when needed,
            // or we could map directly over the payload. For simplicity and tracking,
            // we let lau_malloc track the pointer, and we will sync it on close.
            // Ideally, lau_malloc_wired would accept an fd, but we use it here to ensure
            // the pointer is registered in the Manifold.
            memcpy(raw_map, map, sizeof(TsfiControlNetMap));
            munmap(raw_map, size);
        }
    }
    close(fd);
    
    // We map it the old way but track it via lau_malloc for auditing if needed,
    // actually, let's strictly replace the mapping with a registered pointer.
    
    // The correct TSFi architecture way is to let the kernel map it, but register
    // the pointer manually, OR use lau_malloc_wired and let the user process the memory.
    // For now, to prevent orphaned memory, we will just use the standard mmap but 
    // we MUST ensure we track it.
    
    TsfiControlNetMap *real_map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (real_map == MAP_FAILED) return NULL;
    
    real_map->magic = 0x54434E4D;
    real_map->version = 1;
    real_map->width = 256;
    real_map->height = 256;
    real_map->channels = 3;
    real_map->timestamp_ns = get_time_ns();
    return real_map;
}

void tsfi_cn_shm_close(TsfiControlNetMap *map) {
    if (map) munmap(map, sizeof(TsfiControlNetMap));
}

const TsfiControlNetMap* tsfi_cn_shm_attach(const char *name) {
    int fd = shm_open(name, O_RDONLY, 0666);
    if (fd < 0) return NULL;
    TsfiControlNetMap *map = mmap(NULL, sizeof(TsfiControlNetMap), PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return NULL;
    if (map->magic != 0x54434E4D) { munmap(map, sizeof(TsfiControlNetMap)); return NULL; }
    return map;
}

void tsfi_cn_shm_detach(const TsfiControlNetMap *map) {
    if (map) munmap((void*)map, sizeof(TsfiControlNetMap));
}

TsfiDynamicGuidance* tsfi_dgui_shm_create() {
    int fd = shm_open(TSFI_CN_SHM_DGUI, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return NULL;
    size_t size = sizeof(TsfiDynamicGuidance);
    if (ftruncate(fd, size) == -1) { close(fd); return NULL; }
    TsfiDynamicGuidance *dgui = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (dgui == MAP_FAILED) return NULL;
    dgui->magic = 0x44475549;
    dgui->depth_strength = 0.8f;
    dgui->pose_strength = 0.6f;
    dgui->cfg_scale = 1.0f;
    dgui->steps = 4;
    dgui->terminate = false;
    return dgui;
}

const TsfiDynamicGuidance* tsfi_dgui_shm_attach() {
    int fd = shm_open(TSFI_CN_SHM_DGUI, O_RDONLY, 0666);
    if (fd < 0) return NULL;
    TsfiDynamicGuidance *dgui = mmap(NULL, sizeof(TsfiDynamicGuidance), PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (dgui == MAP_FAILED) return NULL;
    if (dgui->magic != 0x44475549) { munmap(dgui, sizeof(TsfiDynamicGuidance)); return NULL; }
    return dgui;
}

void tsfi_dgui_shm_detach(const TsfiDynamicGuidance *dgui) {
    if (dgui) munmap((void*)dgui, sizeof(TsfiDynamicGuidance));
}

// --- Zero-Copy Latent Bridge Implementation ---

TsfiLatentMap* tsfi_latent_shm_create() {
    int fd = shm_open(TSFI_LATENT_SHM, O_RDWR | O_CREAT, 0666);
    if (fd < 0) return NULL;
    size_t size = sizeof(TsfiLatentMap);
    if (ftruncate(fd, size) == -1) { close(fd); return NULL; }
    TsfiLatentMap *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return NULL;
    map->magic = 0x4C41544E;
    map->version = 1;
    map->width = 32;
    map->height = 32;
    map->channels = 4;
    map->timestamp_ns = get_time_ns();
    return map;
}

void tsfi_latent_shm_close(TsfiLatentMap *map) {
    if (map) munmap(map, sizeof(TsfiLatentMap));
}

const TsfiLatentMap* tsfi_latent_shm_attach() {
    int fd = shm_open(TSFI_LATENT_SHM, O_RDONLY, 0666);
    if (fd < 0) return NULL;
    TsfiLatentMap *map = mmap(NULL, sizeof(TsfiLatentMap), PROT_READ, MAP_SHARED, fd, 0);
    close(fd);
    if (map == MAP_FAILED) return NULL;
    if (map->magic != 0x4C41544E) { munmap(map, sizeof(TsfiLatentMap)); return NULL; }
    return map;
}

void tsfi_latent_shm_detach(const TsfiLatentMap *map) {
    if (map) munmap((void*)map, sizeof(TsfiLatentMap));
}

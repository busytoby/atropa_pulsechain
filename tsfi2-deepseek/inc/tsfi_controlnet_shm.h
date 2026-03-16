#ifndef TSFI_CONTROLNET_SHM_H
#define TSFI_CONTROLNET_SHM_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define TSFI_CN_MAP_SIZE (512 * 512 * 3) // 256x256 * RGB
#define TSFI_CN_SHM_DEPTH "/tsfi_cn_depth"
#define TSFI_CN_SHM_POSE  "/tsfi_cn_pose"

typedef struct {
    uint32_t magic;         // 0x54434E4D 'TCNM'
    uint32_t version;
    uint32_t width;
    uint32_t height;
    uint32_t channels;      // Usually 3 (RGB)
    uint64_t timestamp_ns;
    uint8_t  data[TSFI_CN_MAP_SIZE];
} TsfiControlNetMap;

typedef struct {
    uint32_t magic;         // 0x44475549 'DGUI'
    float depth_strength;   // Default 0.8
    float pose_strength;    // Default 0.6
    float cfg_scale;        // Default 1.0 (for LCM)
    uint32_t steps;         // Default 4
    bool  terminate;        // Trigger for worker
    uint8_t _pad[11];       // Correct padding
} TsfiDynamicGuidance;

#define TSFI_CN_SHM_DGUI "/tsfi_cn_dgui"

// --- Zero-Copy Latent Bridge ---
#define TSFI_LATENT_SHM "/tsfi_cn_latent"
#define TSFI_LATENT_MAP_SIZE (32 * 32 * 4)

typedef struct {
    uint32_t magic;         // 0x4C41544E 'LATN'
    uint32_t version;
    uint32_t width;         // 32
    uint32_t height;        // 32
    uint32_t channels;      // 4
    uint64_t timestamp_ns;
    float    data[TSFI_LATENT_MAP_SIZE];
} TsfiLatentMap;

// API for C (Skeleton Generator / Tuner / Bridge)
TsfiControlNetMap* tsfi_cn_shm_create(const char *name);
void tsfi_cn_shm_close(TsfiControlNetMap *map);
TsfiDynamicGuidance* tsfi_dgui_shm_create();
const TsfiLatentMap* tsfi_latent_shm_attach();
void tsfi_latent_shm_detach(const TsfiLatentMap *map);

// API for C++ (SD Worker)
const TsfiControlNetMap* tsfi_cn_shm_attach(const char *name);
void tsfi_cn_shm_detach(const TsfiControlNetMap *map);
const TsfiDynamicGuidance* tsfi_dgui_shm_attach();
void tsfi_dgui_shm_detach(const TsfiDynamicGuidance *dgui);
TsfiLatentMap* tsfi_latent_shm_create();
void tsfi_latent_shm_close(TsfiLatentMap *map);

#endif // TSFI_CONTROLNET_SHM_H

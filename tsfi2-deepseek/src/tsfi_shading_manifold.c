#include "tsfi_shading_manifold.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static TsfiShadingManifold g_manifold = {0};
static int g_manifold_fd = -1;

void tsfi_shading_manifold_init(void) {
    g_manifold_fd = open("assets/shading_manifold.bin", O_RDONLY);
    if (g_manifold_fd < 0) {
        // Safe Fallback: Neutral grey manifold
        g_manifold.size = SHADING_MAP_SIZE;
        g_manifold.data = (uint8_t*)malloc(SHADING_MAP_SIZE);
        memset(g_manifold.data, 128, SHADING_MAP_SIZE);
        return;
    }

    struct stat st;
    fstat(g_manifold_fd, &st);
    g_manifold.size = st.st_size;
    g_manifold.data = (uint8_t*)mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, g_manifold_fd, 0);
}

void tsfi_shading_manifold_teardown(void) {
    if (g_manifold_fd >= 0) {
        munmap(g_manifold.data, g_manifold.size);
        close(g_manifold_fd);
    } else {
        free(g_manifold.data);
    }
    g_manifold.data = NULL;
}

static inline uint8_t* sample_ptr(float u, float v) {
    if (!g_manifold.data) return NULL;
    u = fmodf(fabsf(u), 1.0f);
    v = fmodf(fabsf(v), 1.0f);
    int px = (int)(u * (SHADING_MAP_RES - 1));
    int py = (int)(v * (SHADING_MAP_RES - 1));
    return &g_manifold.data[(py * SHADING_MAP_RES + px) * 4];
}

Color3 tsfi_sample_manifold_rgb(float u, float v) {
    uint8_t *p = sample_ptr(u, v);
    if (!p) return (Color3){0.5f, 0.5f, 1.0f};
    return (Color3){(float)p[0]/255.0f, (float)p[1]/255.0f, (float)p[2]/255.0f};
}

float tsfi_sample_manifold_rim(float u, float v) {
    uint8_t *p = sample_ptr(u, v);
    if (!p) return 0.0f;
    return (float)p[3]/255.0f;
}

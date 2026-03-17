#include "tsfi_taste_cache.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static int g_taste_fd = -1;
static TsfiTasteRegistry *g_registry = NULL;

void tsfi_taste_cache_init(const char *path) {
    g_taste_fd = open(path, O_RDWR | O_CREAT, 0644);
    if (g_taste_fd < 0) return;

    struct stat st;
    fstat(g_taste_fd, &st);

    if (st.st_size < (off_t)TASTE_CACHE_SIZE) {
        if (ftruncate(g_taste_fd, TASTE_CACHE_SIZE) != 0) return;
    }

    g_registry = mmap(NULL, TASTE_CACHE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_taste_fd, 0);
    if (g_registry == MAP_FAILED) {
        close(g_taste_fd); g_taste_fd = -1;
        return;
    }

    if (g_registry->magic != TASTE_CACHE_MAGIC) {
        memset(g_registry, 0, TASTE_CACHE_SIZE);
        g_registry->magic = TASTE_CACHE_MAGIC;
    }
}

void tsfi_taste_cache_teardown(void) {
    if (g_registry) {
        msync(g_registry, TASTE_CACHE_SIZE, MS_SYNC);
        munmap(g_registry, TASTE_CACHE_SIZE);
    }
    if (g_taste_fd >= 0) close(g_taste_fd);
    g_registry = NULL; g_taste_fd = -1;
}

bool tsfi_taste_cache_persist(uint64_t hash, const TsfiSubjectiveAwareness *unit) {
    if (!g_registry || !unit) return false;

    for (uint32_t i = 0; i < g_registry->entry_count; i++) {
        if (g_registry->metas[i].subjective_hash == hash) {
            memcpy(&g_registry->units[i], unit, sizeof(TsfiSubjectiveAwareness));
            g_registry->metas[i].sequence++;
            g_registry->metas[i].timestamp = (uint32_t)time(NULL);
            msync(&g_registry->units[i], sizeof(TsfiSubjectiveAwareness), MS_ASYNC);
            return true;
        }
    }

    if (g_registry->entry_count < TASTE_REGISTRY_ENTRIES) {
        uint32_t idx = g_registry->entry_count++;
        g_registry->metas[idx].subjective_hash = hash;
        g_registry->metas[idx].sequence = 1;
        g_registry->metas[idx].timestamp = (uint32_t)time(NULL);
        memcpy(&g_registry->units[idx], unit, sizeof(TsfiSubjectiveAwareness));
        msync(&g_registry->metas[idx], sizeof(TsfiTasteMeta), MS_ASYNC);
        msync(&g_registry->units[idx], sizeof(TsfiSubjectiveAwareness), MS_ASYNC);
        return true;
    }
    return false;
}

const TsfiSubjectiveAwareness* tsfi_taste_cache_query(uint64_t hash) {
    if (!g_registry) return NULL;
    for (uint32_t i = 0; i < g_registry->entry_count; i++) {
        if (g_registry->metas[i].subjective_hash == hash) return &g_registry->units[i];
    }
    return NULL;
}

#include "tsfi_wavelet_arena.h"
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <nmmintrin.h>

static int g_journal_fd = -1;
static uint8_t *g_journal_map = NULL;
static size_t g_journal_size = 0;
static uint64_t g_journal_sequence = 0;

#define MAX_JOURNAL_SIZE (1024 * 1024 * 1024) // 1 GiB for infinite persistence

typedef struct {
    uint64_t Xi;
    uint64_t Alpha;
    uint64_t Beta;
    size_t   offset;
} TsfiIndexEntry;

#define MAX_INDEX_ENTRIES 65536
static TsfiIndexEntry g_index[MAX_INDEX_ENTRIES];

static uint64_t tsfi_index_hash(uint64_t Xi, uint64_t Alpha, uint64_t Beta) {
    return (Xi ^ Alpha ^ Beta) % MAX_INDEX_ENTRIES;
}

void tsfi_journal_init(const char *path) {
    g_journal_fd = open(path, O_RDWR | O_CREAT, 0644);
    if (g_journal_fd == -1) {
        perror("[JOURNAL] Failed to open lore_journal.pos");
        return;
    }

    struct stat st;
    fstat(g_journal_fd, &st);
    g_journal_size = st.st_size;

    if (g_journal_size == 0) {
        if (ftruncate(g_journal_fd, 4096) != 0) { perror("[JOURNAL] ftruncate failed"); }
        g_journal_size = 4096;
    }

    g_journal_map = mmap(NULL, MAX_JOURNAL_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, g_journal_fd, 0);
    if (g_journal_map == MAP_FAILED) {
        perror("[JOURNAL] mmap failed");
        close(g_journal_fd);
        g_journal_fd = -1;
        return;
    }

    printf("[JOURNAL] Infinite Persistence substrate initialized at %s (%zu bytes).\n", path, st.st_size);
}

void tsfi_journal_append(TsfiWavelet *W) {
    if (g_journal_fd == -1 || !g_journal_map) return;

    uint32_t crc = 0;
    uint64_t *ptr = (uint64_t*)W;
    for (int i = 0; i < 64; i++) {
        crc = _mm_crc32_u64(crc, ptr[i]);
    }

    TsfiRigidityFooter footer;
    footer.sentinel = TSFI_RIGIDITY_SENTINEL;
    footer.sequence = g_journal_sequence++;
    footer.timestamp = (uint64_t)time(NULL);
    footer.checksum = crc;
    memset(footer.helmholtz_root, 0, 32);

    size_t offset = (g_journal_sequence - 1) * TSFI_JOURNAL_ENTRY_SIZE;
    if (offset + TSFI_JOURNAL_ENTRY_SIZE > g_journal_size) {
        size_t new_size = g_journal_size + (1024 * 1024);
        if (ftruncate(g_journal_fd, new_size) == 0) {
            g_journal_size = new_size;
        } else {
            perror("[JOURNAL] Expand ftruncate failed");
        }
    }

    memcpy(g_journal_map + offset, W, 512);
    memcpy(g_journal_map + offset + 512, &footer, 64);

    uint64_t h = tsfi_index_hash(W->telemetry.Xi, W->telemetry.Fa.identity, W->quota);
    while (g_index[h].offset != 0) {
        h = (h + 1) % MAX_INDEX_ENTRIES;
    }
    g_index[h].Xi = W->telemetry.Xi;
    g_index[h].Alpha = W->telemetry.Fa.identity;
    g_index[h].Beta = W->quota;
    g_index[h].offset = offset + 1;

    msync(g_journal_map + offset, TSFI_JOURNAL_ENTRY_SIZE, MS_SYNC);

    W->state = WAVELET_STATE_ARCHIVED;
    printf("  -> [JOURNAL] UID %llu anchored to disk at sequence %llu.\n", 
           (unsigned long long)W->telemetry.unique_id, (unsigned long long)footer.sequence);
}

void tsfi_journal_recover(TsfiWaveletArena *arena) {
    if (g_journal_fd == -1 || !g_journal_map) return;
    (void)arena;

    printf("[JOURNAL] Starting Boot Recovery Audit...\n");
    size_t valid_entries = 0;
    memset(g_index, 0, sizeof(g_index));

    for (size_t offset = 0; offset + TSFI_JOURNAL_ENTRY_SIZE <= g_journal_size; offset += TSFI_JOURNAL_ENTRY_SIZE) {
        TsfiRigidityFooter *f = (TsfiRigidityFooter *)(g_journal_map + offset + 512);
        if (f->sentinel != TSFI_RIGIDITY_SENTINEL) {
            if (offset > 0) {
                printf("  -> [RECOVERY] Fracture detected at offset %zu. Truncating...\n", offset);
                if (ftruncate(g_journal_fd, offset) != 0) { perror("[RECOVERY] Truncate failed"); }
                g_journal_size = offset;
            }
            break;
        }

        uint32_t crc = 0;
        uint64_t *ptr = (uint64_t*)(g_journal_map + offset);
        for (int i = 0; i < 64; i++) {
            crc = _mm_crc32_u64(crc, ptr[i]);
        }

        if (crc != (uint32_t)f->checksum) {
            printf("  -> [RECOVERY] Checksum mismatch at sequence %llu. State compromised.\n", (unsigned long long)f->sequence);
            break;
        }

        TsfiWavelet *W = (TsfiWavelet *)(g_journal_map + offset);
        if (W->telemetry.unique_id != 0) {
            uint64_t h = tsfi_index_hash(W->telemetry.Xi, W->telemetry.Fa.identity, W->quota);
            while (g_index[h].offset != 0) {
                h = (h + 1) % MAX_INDEX_ENTRIES;
            }
            g_index[h].Xi = W->telemetry.Xi;
            g_index[h].Alpha = W->telemetry.Fa.identity;
            g_index[h].Beta = W->quota;
            g_index[h].offset = offset + 1;
            valid_entries++;
        }

        g_journal_sequence = f->sequence + 1;
    }

    printf("[JOURNAL] Recovery Audit complete. %zu valid materials restored and indexed.\n", valid_entries);
}

TsfiWavelet* tsfi_FetchMaterial(uint64_t Xi, uint64_t Alpha, uint64_t Beta) {
    if (g_journal_fd == -1 || !g_journal_map) return NULL;

    uint64_t h = tsfi_index_hash(Xi, Alpha % MOTZKIN_PRIME, Beta);
    size_t start_h = h;

    do {
        if (g_index[h].offset == 0) break;
        if (g_index[h].Xi == Xi && g_index[h].Alpha == (Alpha % MOTZKIN_PRIME) && g_index[h].Beta == Beta) {
            return (TsfiWavelet *)(g_journal_map + (g_index[h].offset - 1));
        }
        h = (h + 1) % MAX_INDEX_ENTRIES;
    } while (h != start_h);

    return NULL;
}

void tsfi_journal_teardown() {
    if (g_journal_map) munmap(g_journal_map, MAX_JOURNAL_SIZE);
    if (g_journal_fd != -1) close(g_journal_fd);
    g_journal_map = NULL;
    g_journal_fd = -1;
}

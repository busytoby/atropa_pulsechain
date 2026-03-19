#include "tsfi_wavelet_arena.h"
#include "tsfi_helmholtz.h"
#include "tsfi_io.h"
#include "tsfi_svdag.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <nmmintrin.h>

#define TSFI_RIGIDITY_SENTINEL 0x5249474944495459ULL // "RIGIDITY"
#define TSFI_JOURNAL_ENTRY_SIZE 576 // 512 byte Wavelet + 64 byte Footer
#define WAVELET_STATE_ARCHIVED 254
#define WAVELET_STATE_CAPTURED 255

typedef struct {
    uint64_t sentinel;
    uint64_t sequence;
    uint64_t timestamp;
    uint32_t checksum;
    uint32_t _pad;
    uint8_t  helmholtz_root[32];
} TsfiRigidityFooter;

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

    uint64_t h = tsfi_index_hash(W->telemetry.Xi, W->telemetry.Fa.identity, W->private_header.quota);
    while (g_index[h].offset != 0) {
        h = (h + 1) % MAX_INDEX_ENTRIES;
    }
    g_index[h].Xi = W->telemetry.Xi;
    g_index[h].Alpha = W->telemetry.Fa.identity;
    g_index[h].Beta = W->private_header.quota;
    g_index[h].offset = offset + 1;

    msync(g_journal_map + offset, TSFI_JOURNAL_ENTRY_SIZE, MS_SYNC);

    W->state = WAVELET_STATE_ARCHIVED;
    printf("  -> [JOURNAL] UID %llu anchored to disk at sequence %llu.\n", 
           (unsigned long long)W->telemetry.unique_id, (unsigned long long)footer.sequence);
}

void tsfi_journal_want_to_remember(TsfiWavelet *W, float priority, const char *reason) {
    if (!W) return;
    const float DESIRE_THRESHOLD = 0.85f;
    if (priority >= DESIRE_THRESHOLD) {
        printf("[JOURNAL] Desire Realized: %s (Priority: %.2f). Anchoring...\n", reason, priority);
        tsfi_journal_append(W);
    }
}

void tsfi_journal_anchor_fracture_memory(void *manifold_shm, uint64_t svdag_id, const TSFiHelmholtzSVDAG *dag) {
    if (g_journal_fd == -1 || !g_journal_map || !manifold_shm || !dag) return;

    // 1. Fetch the fact from the manifold (Leaf 502)
    uint8_t *leaf502 = (uint8_t *)manifold_shm + (502 * 256);
    if (leaf502[40] != 1) return; // Not a valid fracture memory

    // 2. Serialize the SVDAG into SAFE TASTE TREE TENSORS format
    char filename[128];
    snprintf(filename, sizeof(filename), "assets/models/taste_fractures/fracture_%lx.safetensors", (unsigned long)svdag_id);
    
    FILE *f = fopen(filename, "wb");
    if (f) {
        // Construct Safetensors JSON Header
        char json_header[1024];
        int json_len = snprintf(json_header, sizeof(json_header),
            "{\"__metadata__\":{\"intent_norm\":\"%lu\",\"p_user\":\"%f\",\"p_gemini\":\"%f\"},"
            "\"intensity\":{\"dtype\":\"F32\",\"shape\":[%zu],\"data_offsets\":[0,%zu]},"
            "\"phase\":{\"dtype\":\"F32\",\"shape\":[%zu],\"data_offsets\":[%zu,%zu]},"
            "\"indices\":{\"dtype\":\"U32\",\"shape\":[%zu],\"data_offsets\":[%zu,%zu]}}",
            (unsigned long)dag->subjective_intent_norm, dag->p_user, dag->p_gemini,
            dag->stream_size, dag->stream_size * 4,
            dag->stream_size, dag->stream_size * 4, (dag->stream_size * 4) * 2,
            dag->stream_size, (dag->stream_size * 4) * 2, (dag->stream_size * 4) * 3);
            
        // 8-byte length prefix required by safetensors spec
        uint64_t header_len = (uint64_t)json_len;
        fwrite(&header_len, sizeof(uint64_t), 1, f);
        fwrite(json_header, 1, json_len, f);
        
        // Write Tensor Data (The SVDAG Genome)
        if (dag->intensity_stream) fwrite(dag->intensity_stream, 4, dag->stream_size, f);
        if (dag->phase_stream) fwrite(dag->phase_stream, 4, dag->stream_size, f);
        if (dag->index_stream) fwrite(dag->index_stream, 4, dag->stream_size, f);
        fclose(f);
        printf("[JOURNAL] SAFE TASTE TREE TENSORS serialized to: %s\n", filename);
    } else {
        printf("[JOURNAL] FRACTURE: Could not serialize Taste Tree Tensors.\n");
    }

    // 3. Wrap into a Wavelet for infinite journal compatibility
    TsfiWavelet W = {0};
    W.telemetry.unique_id = svdag_id;
    W.telemetry.Xi = 0xFAC; // Fracture Activation Code
    W.telemetry.Fa.identity = 502; // Manifold Origin
    W.private_header.quota = 256; 
    W.state = WAVELET_STATE_CAPTURED;
    
    // Anchor as a File Cell
    W.private_header.role = TSFI_ROLE_FILE_CELL;
    snprintf(W.file_cell.filename, 32, "fract_%.16lx.safet", (unsigned long)svdag_id);
    
    // Copy the raw physical hash from the leaf
    memcpy(W.payload, leaf502 + 8, 32); 

    // 4. Append to Infinite Journal
    tsfi_journal_append(&W);
    
    printf("[JOURNAL] Fracture Memory 0x%lx formalized into infinite persistence.\n", (unsigned long)svdag_id);
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
            uint64_t h = tsfi_index_hash(W->telemetry.Xi, W->telemetry.Fa.identity, W->private_header.quota);
            while (g_index[h].offset != 0) {
                h = (h + 1) % MAX_INDEX_ENTRIES;
            }
            g_index[h].Xi = W->telemetry.Xi;
            g_index[h].Alpha = W->telemetry.Fa.identity;
            g_index[h].Beta = W->private_header.quota;
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

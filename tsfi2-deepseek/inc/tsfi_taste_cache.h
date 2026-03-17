#ifndef TSFI_TASTE_CACHE_H
#define TSFI_TASTE_CACHE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// --- TSFi Subjective Awareness Unit ---
// A hardware-transcendent unit for Subjective Realism.
// Integrates 64-byte Taste Atoms with RDNA4 control metadata.

typedef struct __attribute__((aligned(64))) {
    float secrets[16]; // 16 Sovereign Secrets (ZMM Ingestion)
} TsfiTasteAtom;

typedef struct {
    TsfiTasteAtom atom;       // 64 bytes: Source of the Waveform
    uint32_t      htile_mask; // 4 bytes: Alligator's culling mask
    float         guardband;  // 4 bytes: Fringe stability coefficient
    uint32_t      vop_seeds;  // 4 bytes: Instruction hints (VOPD/VOP3P)
    uint32_t      msaa_sample_count; // 4 bytes: MSAA samples (1, 4, 8)
    uint32_t      sdma_jump;  // 4 bytes: Nested awareness pointer
} TsfiSubjectiveAwareness;

typedef struct {
    uint64_t subjective_hash; // Xi ^ Alpha ^ Beta
    uint32_t sequence;        // Improvement iteration
    uint32_t timestamp;
} TsfiTasteMeta;

#define TASTE_REGISTRY_ENTRIES 1024
#define TASTE_CACHE_MAGIC 0x5441535445343134ULL

typedef struct __attribute__((aligned(512))) {
    uint64_t magic;
    uint32_t entry_count;
    uint32_t _pad;
    TsfiTasteMeta metas[TASTE_REGISTRY_ENTRIES];
    TsfiSubjectiveAwareness units[TASTE_REGISTRY_ENTRIES];
} TsfiTasteRegistry;

// Surgical substrate for 1024 Awareness Units
#define TASTE_CACHE_SIZE (sizeof(TsfiTasteRegistry))

void tsfi_taste_cache_init(const char *path);
void tsfi_taste_cache_teardown(void);

// Persistence: Stores a complete Subjective Awareness
bool tsfi_taste_cache_persist(uint64_t hash, const TsfiSubjectiveAwareness *unit);

// Retrieval: Returns a direct pointer to the 80-byte hardware-aligned Unit
const TsfiSubjectiveAwareness* tsfi_taste_cache_query(uint64_t hash);

static inline uint64_t tsfi_taste_hash(uint64_t Xi, uint64_t Alpha, uint64_t Beta) {
    return (Xi ^ Alpha ^ Beta);
}

#endif

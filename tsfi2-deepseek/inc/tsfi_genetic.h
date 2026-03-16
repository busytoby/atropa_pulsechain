#ifndef TSFI_GENETIC_H
#define TSFI_GENETIC_H

#include "tsfi_dysnomia.h"

// --- Function Pointer Types for Thunks ---
typedef struct Dai* (*ReactSHIO_Fn)(struct SHIO* Mu, TSFiBigInt* Pi);
typedef struct Dai* (*ReactSHA_Fn)(struct SHA* Mu, TSFiBigInt* Pi, TSFiBigInt* Theta);

// --- Genetic Wrapper (The "Fourier-G" Context) ---
#define GENETIC_TYPE_YI   0
#define GENETIC_TYPE_YANG 1

typedef struct {
    int type; // 0 = YI, 1 = YANG
    void* dys_ptr; 
    
    // Genetic Behavior (Thunks)
    ReactSHIO_Fn react_shio;
    ReactSHA_Fn  react_sha;
    
    // Provenance
    char generation_id[64];
} GeneticNode;

// --- Core API ---

// Universal Crossover: Creates 'Child' from 'A' and 'B'.
// 'Child' must be pre-allocated (e.g. via lau_malloc_wired).
void Fourier_UniversalCrossover(GeneticNode* A, GeneticNode* B, GeneticNode* Child);

// High-performance in-place crossover.
// Internal dys_ptr structures in Child MUST already be allocated and sized appropriately.
void Fourier_UniversalCrossover_InPlace(GeneticNode* A, GeneticNode* B, GeneticNode* Child);

// --- K0Rn Stream Evolution ---
#include "tsfi_k0rn_ops.h" // Includes DEFINED K0RnStream
#include "tsfi_vision.h"

// Factory
K0RnStream* tsfi_k0rn_compile_teddy(void);
K0RnStream* tsfi_k0rn_compile_char(uint32_t c);

// Mutates a single K0Rn stream based on a mutation rate (0.0 - 1.0)
void tsfi_k0rn_mutate(K0RnStream *stream, float rate);

// Performs crossover between two K0Rn streams. Result in 'child'.
void tsfi_k0rn_crossover(const K0RnStream *parent_a, const K0RnStream *parent_b, K0RnStream *child);

// Guided Mutation: Influences mutation parameters based on vision feedback.
// resonance: Current analysis from tsfi_vision
void tsfi_k0rn_evolve_guided(K0RnStream *stream, TSFiResonanceAnalysis resonance, float intensity);
void tsfi_k0rn_evolve_vision(K0RnStream *s, const TSFiResonanceAnalysis *vis, float aggressiveness);
void tsfi_k0rn_symmetrize(K0RnStream *s);

// Benchmarking
// Writes report to 'buffer' (up to 'max_len' bytes).
void tsfi_genetic_benchmark_run(int iterations, char* buffer, size_t max_len);

#endif // TSFI_GENETIC_H

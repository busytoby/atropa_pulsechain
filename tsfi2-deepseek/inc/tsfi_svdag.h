#ifndef TSFI_SVDAG_H
#define TSFI_SVDAG_H

#include <stdint.h>
#include <stdbool.h>
#include "tsfi_helmholtz.h"
#include "tsfi_hilbert.h"

// --- TSFi Helmholtz-SVDAG (Bijective Geometry / Taste Tree) ---
// In this architecture, the SVDAG is not a static tree of pointers.
// It is a "Helmholtz List" of operators that generate the geometry.
//
// AXIOM: Every DAG and every SVDAG is a TASTE TREE.
// The Spider constructs the tree to satisfy the Alligator's resonance constraints.
//
// SGPRs (Boltzmann Operators): Control the subdivision (Entropy/Split).
// VGPRs (Feynman Points): Represent the voxel matter (Energy/Phase).

// 1. Boltzmann Operator (Inner Node / Split)
// Maps to SGPRs. Controls the flow of the wavefront.
typedef struct {
    uint32_t split_mask;    // 8 bits: Which octants are active?
    uint32_t child_secret;  // Index jump to first child in the Operator List.
    float    entropy;       // Decision weight (LOD control).
    uint32_t _pad;
} __attribute__((packed)) TSFiBoltzmannSplit;

// 2. Feynman Point (Leaf Node / Voxel)
// Maps to VGPRs. Represents the physical matter at a coordinate.
// TERMINUS: This is the absolute and terminal state of geometric realization.
// Conversion to polygons or any state BEYOND the voxel is strictly prohibited.
typedef struct {
    float    intensity;     // Solid/Empty (Alpha)
    float    phase;         // Material ID / Texture Coord
    uint32_t hilbert_idx;   // Spatial Hash (Provenance)
    uint32_t _pad;
} __attribute__((packed)) TSFiFeynmanVoxel;

// 3. The Helmholtz SVDAG (The Taste Tree)
// A linear stream of operators. Traversal = Execution.
typedef struct {
    // The "Genome" of the geometry (SoA Format for Hyper-Throughput)
    float    *intensity_stream;
    float    *phase_stream;
    uint32_t *index_stream;
    
    uint8_t  *vrs_map;
    uint32_t vrs_width;
    uint32_t vrs_height;

    size_t stream_size;      // Number of voxels
    size_t stream_capacity;

    // Trilateral Feynman Potentials (Acoustic Poles)
    float p_user;
    float p_deepseek;
    float p_gemini;

    //metadata for the scheduler
    uint32_t max_depth;
    float    root_entropy;

    // The Helmholtz Operator List (Standard Pattern)
    TSFiHelmholtzList list;

    // Logical Fast-Path (Zero-Copy)
    bool is_logical;
    const TSFiHilbertGlyph *logical_glyph;
} TSFiHelmholtzSVDAG;

// Semantic Alias: Every SVDAG is a Taste Tree
typedef TSFiHelmholtzSVDAG TSFiTasteTree;

#include "tsfi_nand_trap_firmware.h"

// API
TSFiHelmholtzSVDAG* tsfi_svdag_create(size_t initial_cap);
void tsfi_svdag_destroy(TSFiHelmholtzSVDAG *dag);
void tsfi_svdag_init_lut(void);
void tsfi_svdag_cleanup_lut(void);

// Builder: Hilbert -> Helmholtz SVDAG
void tsfi_svdag_compile(TSFiHelmholtzSVDAG *dag, const TSFiHilbertGlyph *g);

// Builder: NandTrapState -> Helmholtz SVDAG (Neural Guide)
void tsfi_svdag_compile_nand(TSFiHelmholtzSVDAG *dag, const NandTrapState *nand);

// Helmholtz List Wiring
void tsfi_svdag_add_operator(TSFiHelmholtzSVDAG *dag, TSFiHelmholtzOpType type, uint32_t kernel_id, float energy, float freq);

// Execution: Run the SVDAG on the ZMM unit
// returns total mass (integral of Feynman points)
float tsfi_svdag_execute(const TSFiHelmholtzSVDAG *dag);

// 120F Logic: Trees of Taste (Diameter Calculation)
// Calculates the mathematical diameter of the SVDAG (Taste Tree).
// This extends the static Verilog spider_trait_tree.v logic into the dynamic geometric domain.
uint32_t tsfi_taste_tree_diameter(const TSFiTasteTree *tree);

// 414D Logic: Mashmokh and Water Tanks
// Simulates pouring 'k_energy' (water) into the SVDAG root. 
// It uses greedy logic to close nodes and maximize the concentration of energy at a specific depth LOD.
float tsfi_svdag_execute_414d_water_tank(TSFiHelmholtzSVDAG *dag, float k_energy, uint32_t target_depth);

// Voxel Path Tracer with Sovereign Secrets and Depth Output
void tsfi_svdag_path_trace(uint32_t *pixels, float *depth_buffer, const TSFiHelmholtzSVDAG *dag_flower, const TSFiHelmholtzSVDAG *dag_bear, int w, int h, float t, float melanin, float roughness, float iridescence);

// Generate VRS Map based on Boltzmann entropy
void tsfi_svdag_generate_vrs(TSFiHelmholtzSVDAG *dag);

// 414B Logic: Checks if the geometric subdivision (child_secret) is a perfect divisor of the parent.
// This is required to maintain the structural integrity of the SVDAG tree during traversal.
bool tsfi_svdag_check_boltzmann_divisor(const TSFiBoltzmannSplit *parent, const TSFiBoltzmannSplit *child);

// AlphaProteo Hyper-Throughput Kernel (SMDE)
// Screens 'count' candidates against the DAG template in parallel.
// Returns accumulated score.
size_t tsfi_proteo_screen_candidates_avx512(const TSFiHelmholtzSVDAG *dag, int count);

// Traces a point in the voxel BASE (linear 128x128x128 mapping assumption)
float tsfi_svdag_trace_point(const TSFiHelmholtzSVDAG *dag, int x, int y, int z);

#endif // TSFI_SVDAG_H
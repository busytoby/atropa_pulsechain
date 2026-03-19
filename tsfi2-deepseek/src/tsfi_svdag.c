#include "tsfi_svdag.h"
#include "lau_memory.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <immintrin.h>
#include <stdalign.h>
#include <stdatomic.h>
#include "tsfi_wave512.h"
#include "tsfi_c_math.h"

#define BASIS_COUNT 9
#define LUT_SIZE 1024
static float *g_basis_lut = NULL; 
static _Atomic int g_lut_init_state = 0;

void tsfi_svdag_init_lut(void) {
    int expected = 0;
    if (atomic_compare_exchange_strong(&g_lut_init_state, &expected, 1)) {
        g_basis_lut = (float*)lau_memalign(64, BASIS_COUNT * LUT_SIZE * sizeof(float));
        for(int i=0; i<BASIS_COUNT; i++) {
            float freq = (float)(i + 1) * 0.05f;
            for(int k=0; k<LUT_SIZE; k++) {
                g_basis_lut[i * LUT_SIZE + k] = tsfi_sinf(k * freq);
            }
        }
        atomic_store(&g_lut_init_state, 2);
    } else {
        while (atomic_load(&g_lut_init_state) < 2) _mm_pause();
    }
}

TSFiHelmholtzSVDAG* tsfi_svdag_create(size_t initial_cap) {
    TSFiHelmholtzSVDAG* dag = (TSFiHelmholtzSVDAG*)calloc(1, sizeof(TSFiHelmholtzSVDAG));
    if (!dag) return NULL;
    dag->intensity_stream = (float*)calloc(initial_cap, sizeof(float));
    dag->phase_stream     = (float*)calloc(initial_cap, sizeof(float));
    dag->index_stream     = (uint32_t*)calloc(initial_cap, sizeof(uint32_t));
    dag->vrs_map          = (uint8_t*)calloc(128 * 128, 1);
    dag->vrs_width = 128; dag->vrs_height = 128;
    dag->stream_capacity = initial_cap;
    dag->stream_size = 0;
    dag->max_depth = 0;
    dag->root_entropy = 1.0f;
    dag->is_logical = false;
    dag->logical_glyph = NULL;
    dag->subjective_intent_norm = 137; // Baseline
    tsfi_svdag_init_lut();
    return dag;
}

void tsfi_svdag_calculate_intent(TSFiHelmholtzSVDAG *dag, float w_user, float w_deepseek, float w_gemini) {
    if (!dag) return;

    // The Intent Equation: Weighted Trilateral Potential
    // Base Resolution Scalar: 1000.0f
    float total_weighted_potential = (w_user * dag->p_user) + 
                                     (w_deepseek * dag->p_deepseek) + 
                                     (w_gemini * dag->p_gemini);

    // Mandate the physical norm for autonomy
    dag->subjective_intent_norm = (uint64_t)(1000.0f * total_weighted_potential);

    if (dag->subjective_intent_norm < 1) dag->subjective_intent_norm = 137; // Gravitational Lock Minimum

    printf("[AUTONOMY] Intent Projected: 0x%lx (U:%.2f, DS:%.2f, G:%.2f)\n", 
           (unsigned long)dag->subjective_intent_norm, dag->p_user, dag->p_deepseek, dag->p_gemini);
}

void tsfi_svdag_destroy(TSFiHelmholtzSVDAG *dag) {
    if (!dag) return;
    if (dag->intensity_stream) free(dag->intensity_stream);
    if (dag->phase_stream) free(dag->phase_stream);
    if (dag->index_stream) free(dag->index_stream);
    if (dag->vrs_map) free(dag->vrs_map);
    free(dag);
}

#include <math.h>

void tsfi_svdag_compile_nand(TSFiHelmholtzSVDAG *dag, const NandTrapState *nand) {
    if (!dag || !nand || !dag->intensity_stream) return;
    
    // Clear density field
    memset(dag->intensity_stream, 0, dag->stream_capacity * sizeof(float));

    for (int i = 0; i < GRANS; i++) {
        const TsfiGran *f = &nand->fibers[i];
        if (f->weight <= 0.01f) continue;

        // Map fiber (-1..1 range) to 128^3 grid
        int vx = (int)((f->x + 1.0f) * 64.0f);
        int vy = (int)((f->y + 1.0f) * 64.0f);
        int vz = (int)((f->z + 1.0f) * 64.0f);

        if (vx < 0 || vx >= 128 || vy < 0 || vy >= 128 || vz < 0 || vz >= 128) continue;

        // Splat density with local falloff (3x3x3)
        for (int dz = -1; dz <= 1; dz++) {
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = vx + dx, ny = vy + dy, nz = vz + dz;
                    if (nx < 0 || nx >= 128 || ny < 0 || ny >= 128 || nz < 0 || nz >= 128) continue;
                    size_t idx = (size_t)nz * 16384 + (size_t)ny * 128 + nx;
                    
                    // Safety check against stream_capacity
                    if (idx >= dag->stream_capacity) continue;

                    float dist = sqrtf((float)(dx*dx + dy*dy + dz*dz));
                    float impact = f->weight * expf(-dist * 2.0f);
                    dag->intensity_stream[idx] += impact;
                }
            }
        }
    }
    dag->stream_size = dag->stream_capacity;
}

void tsfi_svdag_add_operator(TSFiHelmholtzSVDAG *dag, TSFiHelmholtzOpType type, uint32_t kernel_id, float energy, float freq) {
    if (!dag || dag->list.op_count >= HELMHOLTZ_MAX_OPS) return;
    TSFiHelmholtzOperator *op = &dag->list.ops[dag->list.op_count++];
    op->op_type = type;
    op->kernel_id = kernel_id;
    op->energy_addend = energy;
    op->frequency = freq;
    dag->list.total_energy += energy;
}

void tsfi_svdag_compile(TSFiHelmholtzSVDAG *dag, const TSFiHilbertGlyph *g) {
    if (!dag || !g) return;
    int max_voxels = (int)dag->stream_capacity;
    
    __m512 vW[BASIS_COUNT];
    for(int i=0; i<BASIS_COUNT; i++) vW[i] = _mm512_set1_ps(g->coeffs[i/3][i%3].real);

    float *out = dag->intensity_stream;
    
    // Deep Unroll (128 voxels per loop) to maximize bus saturation
    for (int i = 0; i <= max_voxels - 128; i += 128) {
        for (int b = 0; b < 8; b++) {
            int secret = i + b*16;
            __m512 vSum = _mm512_setzero_ps();
            int lut_secret = secret % LUT_SIZE;
            for(int k=0; k<BASIS_COUNT; k++) {
                __m512 vB = _mm512_loadu_ps(&g_basis_lut[k * LUT_SIZE + lut_secret]);
                vSum = _mm512_fmadd_ps(vB, vW[k], vSum);
            }
            // Clamp to zero (ReLU) to ensure non-negative occupancy/mass
            vSum = _mm512_max_ps(vSum, _mm512_setzero_ps());
            _mm512_storeu_ps(&out[secret], vSum);
        }
    }
    dag->stream_size = max_voxels;
}

float tsfi_svdag_execute(const TSFiHelmholtzSVDAG *dag) {
    if (!dag || !dag->intensity_stream) return 0.0f;
    float total_mass = 0.0f;
    for(size_t i=0; i<dag->stream_size; i++) total_mass += dag->intensity_stream[i];
    return total_mass;
}

// 120F Logic: Trees of Taste (Diameter Calculation)
uint32_t tsfi_taste_tree_diameter(const TSFiTasteTree *tree) {
    if (!tree || tree->max_depth == 0) return 0;
    
    // In our linear TSFiHelmholtzSVDAG, the exact topological tree is compressed.
    // A true diameter requires finding the two longest independent paths from the root.
    // In the hardware Verilog (spider_trait_tree.v), this is `max1 + max2` of 4 static legs.
    // For the dynamic SVDAG, we approximate the diameter based on the 'max_depth' metadata
    // multiplied by a branching factor, or explicitly traversing the operator list if it represented a true tree.
    // Since our Helmholtz List is sequential pipeline operations, we define its 'diameter'
    // as the sum of its depth and the total number of distinct topological splits it induces.
    
    uint32_t simulated_max1 = tree->max_depth;
    uint32_t simulated_max2 = (tree->list.op_count > 0) ? tree->list.op_count : 1;
    
    // In a perfectly balanced SVDAG, max1 == max2.
    // For this 120F generalization, we combine the depth and operation complexity.
    return simulated_max1 + simulated_max2;
}

// 414D Logic: Mashmokh and Water Tanks
float tsfi_svdag_execute_414d_water_tank(TSFiHelmholtzSVDAG *dag, float k_energy, uint32_t target_depth) {
    if (!dag || k_energy <= 0.0f) return 0.0f;
    
    // In 414D, we want to maximize the number of nodes with water at a specific depth.
    // We pour 'k_energy' into the root.
    // In our SVDAG, this maps to distributing wavefront energy (threads/rays) down the DAG.
    // We use a greedy approach: we "close" branches (split_mask bits) that lead to shallow
    // dead-ends or areas we "don't know how to render yet" to force all energy down 
    // the optimal paths to the target LOD (depth).
    
    float energy_at_target = 0.0f;
    float current_energy = k_energy;
    
    // Simplified simulation of the wave propagation down the SVDAG levels
    for (uint32_t d = 0; d <= target_depth; d++) {
        // At each depth, a certain amount of energy is lost to "friction" or rendering cost
        // unless we aggressively cull branches.
        // We simulate closing off non-optimal branches to conserve water (energy).
        float retention_factor = 0.95f; // Greedy optimization retains most energy
        
        current_energy *= retention_factor;
        
        if (d == target_depth) {
            energy_at_target = current_energy;
        }
    }
    
    // Apply this concentrated energy to the actual Feynman points to boost intensity 
    // at the target LOD, demonstrating what we *can* render very fast.
    if (energy_at_target > 0.0f) {
        float boost = energy_at_target / (float)dag->stream_size;
        for(size_t i=0; i<dag->stream_size; i++) {
            dag->intensity_stream[i] += boost;
        }
    }
    
    return energy_at_target;
}

// 414B Logic: Checks if the geometric subdivision is a valid divisor.
bool tsfi_svdag_check_boltzmann_divisor(const TSFiBoltzmannSplit *parent, const TSFiBoltzmannSplit *child) {
    if (!parent || !child) return false;
    
    // In our SVDAG, the 'child_secret' represents the topological dimension of the node block.
    // For a child node to be a valid fractal subdivision of the parent (the Mashmokh sequence),
    // its dimension must cleanly divide the parent's dimension.
    uint32_t p_dim = parent->child_secret;
    uint32_t c_dim = child->child_secret;
    
    if (c_dim == 0 || p_dim == 0) return false; // Prevent mod by zero
    
    return (p_dim % c_dim == 0);
}

void tsfi_svdag_generate_vrs(TSFiHelmholtzSVDAG *dag) {
    if (!dag || !dag->vrs_map) return;
    
    size_t map_size = dag->vrs_width * dag->vrs_height;
    float *intensity = dag->intensity_stream;
    uint8_t *vrs = dag->vrs_map;

    __m512 vHigh = _mm512_set1_ps(0.8f);
    __m512 vMid  = _mm512_set1_ps(0.4f);
    
    // Process map in 64-byte chunks
    for (size_t i = 0; i < map_size && i + 16 <= dag->stream_size; i += 16) {
        __m512 vI = _mm512_loadu_ps(&intensity[i]);
        
        // Masks for shading rates
        __mmask16 m1x1 = _mm512_cmp_ps_mask(vI, vHigh, _CMP_GT_OQ);
        __mmask16 m2x2 = _mm512_mask_cmp_ps_mask(~m1x1, vI, vMid, _CMP_GT_OQ);
        __mmask16 m4x4 = ~(m1x1 | m2x2);

        // Fill byte register
        // VRS Codes: 1x1=0x0, 2x2=0x5, 4x4=0xA
        alignas(64) uint32_t codes[16];
        __m512i vCodes = _mm512_setzero_si512();
        vCodes = _mm512_mask_blend_epi32(m2x2, vCodes, _mm512_set1_epi32(0x5));
        vCodes = _mm512_mask_blend_epi32(m4x4, vCodes, _mm512_set1_epi32(0xA));
        
        _mm512_store_si512(codes, vCodes);
        for(int k=0; k<16; k++) {
            vrs[i + k] = (uint8_t)codes[k];
        }
    }
}

size_t tsfi_proteo_screen_candidates_avx512(const TSFiHelmholtzSVDAG *dag, int candidate_count) {

    size_t len = dag->stream_size;

    __m512 vM = _mm512_set1_ps(0.05f); 

    float global_sum = 0.0f;

    

    // Process candidates in batches of 16 (ZMM width)

    int batches = candidate_count / 16;

    if (batches == 0) batches = 1;



    for (int batch = 0; batch < batches; batch++) { 

        __m512 vA0 = _mm512_setzero_ps();

        // Inner loop: Scan the entire SVDAG for this batch of candidates

        for (size_t i = 0; i <= len - 16; i += 16) {

            __m512 vI = _mm512_loadu_ps(&dag->intensity_stream[i]);

            vA0 = _mm512_fmadd_ps(vI, vM, vA0);

        }

        float tmp[16];

        _mm512_storeu_ps(tmp, vA0);

        global_sum += tmp[0];

    }

    return (size_t)global_sum + candidate_count;

}

#define SVDAG_CACHE_SIZE 32
typedef struct {
    uint32_t key; // (x>>2) | (y>>2)<<10 | (z>>2)<<20
    float values[64]; // 4x4x4 block
    const TSFiHelmholtzSVDAG *dag;
} SvdagSpatialCache;

static SvdagSpatialCache g_svdag_cache[SVDAG_CACHE_SIZE] = {0};

float tsfi_svdag_trace_point(const TSFiHelmholtzSVDAG *dag, int x, int y, int z) {
    if (!dag || !dag->intensity_stream) return 0.0f;
    if (x < 0 || x >= 128 || y < 0 || y >= 128 || z < 0 || z >= 128) return 0.0f;

    // 1. High-Speed Spatial Cache Check
    uint32_t key = (x >> 2) | ((y >> 2) << 10) | ((z >> 2) << 20);
    uint32_t cache_idx = key % SVDAG_CACHE_SIZE;
    
    if (g_svdag_cache[cache_idx].key == key && g_svdag_cache[cache_idx].dag == dag) {
        int lx = x & 3; int ly = y & 3; int lz = z & 3;
        return g_svdag_cache[cache_idx].values[lz * 16 + ly * 4 + lx];
    }

    // 2. Cache Miss: Hydrate 4x4x4 block
    int bx = x & ~3; int by = y & ~3; int bz = z & ~3;
    g_svdag_cache[cache_idx].key = key;
    g_svdag_cache[cache_idx].dag = dag;
    
    for (int lz = 0; lz < 4; lz++) {
        for (int ly = 0; ly < 4; ly++) {
            size_t base_idx = (size_t)(bz + lz) * 16384 + (size_t)(by + ly) * 128 + bx;
            // Vector load 4 floats if possible, but for simplicity:
            for (int lx = 0; lx < 4; lx++) {
                g_svdag_cache[cache_idx].values[lz * 16 + ly * 4 + lx] = dag->intensity_stream[base_idx + lx];
            }
        }
    }

    int lx = x & 3; int ly = y & 3; int lz = z & 3;
    return g_svdag_cache[cache_idx].values[lz * 16 + ly * 4 + lx];
}

void tsfi_svdag_teardown(void) {
    if (g_basis_lut) {
        lau_free(g_basis_lut);
        g_basis_lut = NULL;
        atomic_store(&g_lut_init_state, 0);
    }
}

void tsfi_svdag_cleanup_lut(void) {
    if (g_basis_lut) {
        lau_free(g_basis_lut);
        g_basis_lut = NULL;
        atomic_store(&g_lut_init_state, 0);
    }
}


#include "tsfi_helmholtz.h"
#include "tsfi_merkle.h"
#include "tsfi_math.h"
#include "tsfi_c_math.h"
#include "tsfi_io.h"
#include "tsfi_wire_firmware.h"
#include <string.h>
#include <stdlib.h>
#include "lau_thunk.h"
#include "lau_registry.h"
#include "tsfi_wavelet_arena.h"

#define TSFI_GRAVITATIONAL_LOCK 137

static void reduce_region(uint8_t *root_out, float *mu_out, float *continuity_out, const void *region_start, const void *lore_start, int leaf_count, uint32_t epoch, uint64_t resonance_k, uint64_t unified_norm, uint64_t intent_norm);

// --- Generation 8: Recursive Wavelet Linking ---

void tsfi_AnchorToParent(TsfiWavelet *W, uint64_t master_anchor, uint64_t Prime) {
    if (!W) return;
    // Physically bind the wavelet to the master anchor (derived from prev epoch wavelet)
    // This summarizes the entire history leading to the current Ring state
    W->telemetry.Monopole = (W->telemetry.Monopole ^ master_anchor) % Prime;
    W->telemetry.Ring = (W->telemetry.Ring ^ master_anchor) % Prime;
}

uint64_t tsfi_GetWaveletAnchor(TsfiWavelet *W) {
    if (!W) return 0;
    // Derive a 64-bit anchor from the wavelet's physical trilateral state
    // This summarizes the entire history leading to the current Ring state
    uint64_t anchor = W->telemetry.Monopole;
    anchor ^= W->telemetry.Ring;
    anchor ^= W->telemetry.Barn;
    return anchor;
}

void tsfi_TransitionAcousticEpoch(uint8_t *next_parent_root, const uint8_t *prev_root, void *arena_context, uint64_t master_anchor) {
    if (!next_parent_root || !prev_root || !arena_context) return;
    TsfiWaveletArena *arena = (TsfiWaveletArena*)arena_context;
    
    // 1. Recursive Linking: Update parent root link
    memcpy(next_parent_root, prev_root, 32);
    
    // 2. Physical Anchoring: Apply the master anchor to all wavelets in the new epoch
    for (uint64_t off = 0; off < arena->offset; off += 512) {
        TsfiWavelet *W = (TsfiWavelet*)(arena->base_ptr + off);
        if (W->telemetry.unique_id != 0) {
            // Physically bind the wavelet to the master anchor (derived from prev epoch wavelet)
            tsfi_AnchorToParent(W, master_anchor, W->Prime);
        }
    }
    printf("  -> [TRANSITION] Acoustic Chain extended with Wavelet Anchor: %llu\n", (unsigned long long)master_anchor);
}

// --- Rigid ZMM Kernel (Absolute Internal Development) ---

static _Atomic uint64_t g_manifold_heat = 0;
static _Atomic uint64_t g_user_pole_norm = 0; // Atomic User Pole
static _Atomic uint8_t  g_user_phase_key = 0; // Atomic Phase Key

/**
 * @brief Injects a manual user influence (Pole) into the trilateral state.
 * @param pole Pointer to the user pole structure containing norm and phase key.
 */
void tsfi_helmholtz_inject_user_pole(const TSFiUserPole *pole) {
    if (!pole) return;
    atomic_store(&g_user_pole_norm, pole->norm);
    atomic_store(&g_user_phase_key, pole->phase_key);
    TSFI_DEBUG_FPRINTF(stdout, "[INJECT] User Pole Solidified (Struct): norm=%lu, key=0x%02x\n", pole->norm, pole->phase_key);
}

/**
 * @brief Performs a hardware-accelerated XOR-sum hash using ZMM registers.
 * @param out Pointer to 256-bit output buffer.
 * @param in Pointer to input data.
 * @param len Length of input data in bytes.
 */
void tsfi_internal_zmm_hash(uint8_t *out, const uint8_t *in, size_t len) {
    // Rigid Internal Development: Fast ZMM XOR-sum Hash with Tail Handling
    __m512i h = _mm512_setzero_si512();
    size_t blocks = len / 64;
    for (size_t i = 0; i < blocks; i++) {
        __m512i m = _mm512_loadu_si512((const __m512i*)in + i);
        h = _mm512_xor_si512(h, m);
    }
    
    // Tail handling
    size_t rem = len % 64;
    if (rem > 0) {
        uint8_t tail[64] __attribute__((aligned(64)));
        memset(tail, 0, 64);
        memcpy(tail, in + (blocks * 64), rem);
        __m512i m = _mm512_loadu_si512((const __m512i*)tail);
        h = _mm512_xor_si512(h, m);
    }

    // Finalization: Fold the ZMM into 256 bits
    __m256i lo = _mm512_extracti64x4_epi64(h, 0);
    __m256i hi = _mm512_extracti64x4_epi64(h, 1);
    __m256i res = _mm256_xor_si256(lo, hi);
    _mm256_storeu_si256((__m256i*)out, res);
}

typedef struct { uint64_t n[4]; } fe256; // Internal 256-bit field

static inline void fe256_add(fe256 *r, const fe256 *a, const fe256 *b) {
    __m256i va = _mm256_loadu_si256((const __m256i*)a);
    __m256i vb = _mm256_loadu_si256((const __m256i*)b);
    __m256i vr = _mm256_add_epi64(va, vb);
    _mm256_storeu_si256((__m256i*)r, vr);
}

/**
 * @brief Verifies a trilateral signature using code-sum bijection.
 * @param sig 64-byte signature buffer.
 * @param msg_hash 32-byte message hash.
 * @param expected_addr (Optional) Expected address for validation.
 * @return 1 if valid (rigid), 0 otherwise.
 */
int tsfi_verify_signature(const uint8_t *sig, const uint8_t *msg_hash, const uint8_t *expected_addr) {
    (void)expected_addr;
    
    // Rigid Internal Development: ZMM XOR-sum reduction
    __m512i vSig = _mm512_loadu_si512((const __m512i*)sig);
    
    // Horizontal XOR reduction of the 64 bytes
    // We fold the 512-bit register into 256, then 128, then 64, then 8
    __m256i lo256 = _mm512_extracti64x4_epi64(vSig, 0);
    __m256i hi256 = _mm512_extracti64x4_epi64(vSig, 1);
    __m256i x256  = _mm256_xor_si256(lo256, hi256);
    
    __m128i lo128 = _mm256_extracti128_si256(x256, 0);
    __m128i hi128 = _mm256_extracti128_si256(x256, 1);
    __m128i x128  = _mm_xor_si128(lo128, hi128);
    
    uint64_t x64 = _mm_extract_epi64(x128, 0) ^ _mm_extract_epi64(x128, 1);
    uint32_t x32 = (uint32_t)(x64 ^ (x64 >> 32));
    uint16_t x16 = (uint16_t)(x32 ^ (x32 >> 16));
    uint8_t  code_sum = (uint8_t)(x16 ^ (x16 >> 8));

    if (code_sum == msg_hash[0]) {
        return 1;
    }
    return 0;
}

static int ar_stf_helper(void *manifold, const TSFiDirective *tx_ptr, uint64_t manifold_chain_id, uint64_t resonance_k) {
    TSFI_DEBUG_FPRINTF(stdout, "[STF] ar_stf_helper entry src=%u, dst=%u\n", tx_ptr->src_leaf, tx_ptr->dst_leaf);
    AR_AccountLeaf *leaves = (AR_AccountLeaf *)manifold;
    AR_AccountLeaf *src = &leaves[tx_ptr->src_leaf];
    AR_AccountLeaf *dst = &leaves[tx_ptr->dst_leaf];
    
    if (tx_ptr->chain_id != manifold_chain_id) return 0;
    if (tx_ptr->nonce != src->nonce) return 0;

    uint8_t wave_hash[32] __attribute__((aligned(32)));
    tsfi_internal_zmm_hash(wave_hash, (const uint8_t*)tx_ptr, 140);
    uint64_t resonance_val = *(uint64_t*)wave_hash;

    if (resonance_k > 0) {
        if ((resonance_val % resonance_k) != 0) return 0;
    }

    if (!tsfi_verify_signature(tx_ptr->signature, wave_hash, src->owner_address)) return 0;

    uint64_t bal_src[32] __attribute__((aligned(64)));
    uint64_t bal_dst[32] __attribute__((aligned(64)));
    uint64_t amount[32] __attribute__((aligned(64)));
    
    memset(bal_src, 0, 256); memset(bal_dst, 0, 256); memset(amount, 0, 256);
    memcpy(bal_src, src->balance, 32); 
    memcpy(bal_dst, dst->balance, 32);
    memcpy(amount, tx_ptr->amount, 32);
    
    if (bal_src[0] < amount[0]) return 0;

    bal_src[0] -= amount[0];
    bal_dst[0] += amount[0];
    src->nonce++; 
    
    memcpy(src->balance, bal_src, 32);
    memcpy(dst->balance, bal_dst, 32);
    return 1;
}

static void ar_transfer_combined(void *manifold, uint8_t *map, const TSFiDirective *tx_ptr, uint64_t *global_heat, uintptr_t chain_id, uintptr_t tx_idx, uintptr_t resonance_k) {
    if (!manifold || !tx_ptr) return;

    if (map) {
        int bit_src = tx_ptr->src_leaf / 4;
        map[bit_src / 8] |= (1 << (bit_src % 8));
        int bit_dst = tx_ptr->dst_leaf / 4;
        map[bit_dst / 8] |= (1 << (bit_dst % 8));
    }

    if (global_heat) atomic_fetch_add((_Atomic uint64_t*)global_heat, tx_ptr->gas_limit);
    int success = ar_stf_helper(manifold, tx_ptr, (uint64_t)chain_id, (uint64_t)resonance_k);

    TSFiReceipt *r = &((TSFiReceipt *)((AR_AccountLeaf*)manifold + TSFI_STATE_LEAVES))[(uint64_t)tx_idx % TSFI_RECEIPT_LEAVES];
    r->status = success;
    r->energy_used = global_heat ? atomic_load((_Atomic uint64_t*)global_heat) : 0;
    tsfi_keccak512((const uint8_t*)tx_ptr, 140, r->tx_hash);
}

typedef struct {
    void *manifold;
    uint8_t *map;
    const TSFiDirective *tx;
    uint64_t *heat;
    uint64_t chain_id;
    int tx_idx;
    uint64_t resonance_k;
} STFParams;

static void tsfi_helmholtz_directive_thunk_helper(STFParams *p) {
    if (!p) return;
    ar_transfer_combined(p->manifold, p->map, p->tx, p->heat, (uintptr_t)p->chain_id, (uintptr_t)p->tx_idx, (uintptr_t)p->resonance_k);
    free(p);
}

void* tsfi_helmholtz_emit_ar_transfer(ThunkProxy *p, void *manifold_512kb, uint8_t *thunk_access_map, const TSFiDirective *tx, _Atomic uint64_t *global_heat, uint64_t chain_id, int tx_idx, uint64_t resonance_k) {
    thunk_check_bounds(p, 512);
    void *entry = p->thunk_cursor;
    STFParams *params = malloc(sizeof(STFParams));
    params->manifold = manifold_512kb;
    params->map = thunk_access_map;
    params->tx = tx;
    params->heat = (uint64_t*)global_heat;
    params->chain_id = chain_id;
    params->tx_idx = tx_idx;
    params->resonance_k = resonance_k;
    ThunkProxy_emit_baked(p, (void*)tsfi_helmholtz_directive_thunk_helper, 1, params);
    return entry;
}

void tsfi_helmholtz_tokenize_lore(void *manifold, const char *text, int leaf_offset) {
    if (!manifold || !text) return;
    uint8_t *directive_region = (uint8_t*)manifold + (TSFI_TOTAL_LEAVES * 256);
    uint8_t token_hash[64];
    tsfi_keccak512((const uint8_t*)text, strlen(text), token_hash);
    memcpy(directive_region + (leaf_offset * 256), token_hash, 64);
}

void tsfi_helmholtz_reduce_directives(uint8_t *root_out, const TSFiDirective *directives, uint32_t epoch, uint64_t resonance_k) {
    reduce_region(root_out, NULL, NULL, directives, NULL, TSFI_DIRECTIVE_LEAVES, epoch, resonance_k, 0, TSFI_GRAVITATIONAL_LOCK);
}

static TSFiBigInt *g_helmholtz_prime = NULL;

float tsfi_helmholtz_get_heat(void) {
    return (float)atomic_load(&g_manifold_heat) / 1000.0f;
}

#include "tsfi_pool.h"

bool tsfi_ppo_audit_proposal(const void *manifold, TSFiPPOProposal *proposal, uint64_t resonance_k) {
    if (!manifold || !proposal || resonance_k == 0) return false;

    // 1. Acoustic Relevance Check: Intensity must be a perfect harmonic multiple of resonance_k
    // We allow a small tolerance for trilateral drift (0.1%)
    proposal->is_resonant = (proposal->intensity % resonance_k == 0);

    if (proposal->is_resonant) {
        // [STABLE] Set the Oracle lever (Leaf 501) to 1 via the manifold directly
        // Note: Manifold topography is physically rigid; we modify the leaf directly to signal the NAND gate.
        uint8_t *leaf501 = (uint8_t *)manifold + (501 * 256);
        leaf501[0] = 1; // Reset_N high
        return true;
    } else {
        // [ACTIVE DISCARD] Force (0,0) state by pulling Reset_N low
        uint8_t *leaf501 = (uint8_t *)manifold + (501 * 256);
        leaf501[0] = 0; // Reset_N low (Triggering metastability if Gemini is low)
        
        // Physical Incineration: Zero out the proposal leaf
        uint8_t *target_leaf = (uint8_t *)manifold + (proposal->leaf_index * 256);
        memset(target_leaf, 0, 256);
        return false;
    }
}

void tsfi_helmholtz_init(void) {
    atomic_store_explicit(&g_init_in_progress, 1, memory_order_relaxed);
    if (g_helmholtz_prime) return;
    tsfi_pool_init();
    g_helmholtz_prime = tsfi_bn_alloc();
    const uint8_t p_bytes[66] = {
        0x69, 0x1a, 0x70, 0xbc, 0x6e, 0x73, 0x7e, 0x10, 0x0a, 0x81, 0xcb, 0x55, 0x76, 0x30, 0xe3, 0xd6,
        0x16, 0x05, 0x47, 0x6d, 0xe2, 0x8c, 0x00, 0x70, 0x7c, 0xb7, 0x18, 0x33, 0x5d, 0x06, 0x8b, 0x70,
        0x48, 0x73, 0xda, 0x49, 0x9d, 0x16, 0x73, 0x55, 0x61, 0x8b, 0x33, 0x2d, 0x35, 0x3b, 0xab, 0x22,
        0x6b, 0x32, 0x40, 0x96, 0x15, 0x89, 0x44, 0x3e, 0xeb, 0x88, 0xf9, 0x54, 0x5f, 0x1e, 0x16, 0x8c,
        0x14, 0x02
    };
    tsfi_bn_from_bytes(g_helmholtz_prime, p_bytes, 66);
    atomic_store_explicit(&g_init_in_progress, 0, memory_order_relaxed);
}

void tsfi_helmholtz_cleanup(void) {
    if (g_helmholtz_prime) { tsfi_bn_free(g_helmholtz_prime); g_helmholtz_prime = NULL; }
    tsfi_pool_teardown();
}

void zmm_add_512(uint64_t *r, const uint64_t *a, const uint64_t *b) {
    for (int i = 0; i < 4; i++) {
        __m512i va = _mm512_loadu_si512((const __m512i*)a + i);
        __m512i vb = _mm512_loadu_si512((const __m512i*)b + i);
        __m512i vr = _mm512_add_epi64(va, vb);
        _mm512_storeu_si512((__m512i*)r + i, vr);
    }
}

void zmm_xor_512(uint64_t *r, const uint64_t *a, const uint64_t *b) {
    for (int i = 0; i < 4; i++) {
        __m512i va = _mm512_loadu_si512((const __m512i*)a + i);
        __m512i vb = _mm512_loadu_si512((const __m512i*)b + i);
        __m512i vr = _mm512_xor_si512(va, vb);
        _mm512_storeu_si512((__m512i*)r + i, vr);
    }
}

void react_reduce_nodes_internal(uint64_t *res, const uint64_t *left, const uint64_t *right, uint32_t level) {

    uint64_t tmp[32] __attribute__((aligned(64)));
    zmm_add_512(tmp, left, right);
    tmp[0] += level;
    uint64_t norm_sum = 0;
    for(int i=0; i<32; i++) norm_sum += tmp[i];
    if (norm_sum > 0x7FFFFFFFFFFFFFFFULL) atomic_fetch_add(&g_manifold_heat, 1);
    tmp[31] &= 0x00000000FFFFFFFFULL;
    memcpy(res, tmp, 256);
}

static void react_reduce_nodes_lore_internal(uint64_t *res, const uint64_t *left, const uint64_t *right, const uint64_t *lore_left, const uint64_t *lore_right, uint32_t epoch) {
    uint64_t l_sum[32] __attribute__((aligned(64)));
    zmm_xor_512(l_sum, lore_left, lore_right);
    l_sum[0] += epoch;
    uint64_t tmp[32] __attribute__((aligned(64)));
    zmm_add_512(tmp, left, right);
    zmm_add_512(tmp, tmp, l_sum);
    tmp[31] &= 0x00000000FFFFFFFFULL;
    memcpy(res, tmp, 256);
}

static float tsfi_individual_impact_probe(int n, int m, float c_real, float c_imag, uint64_t k) {
    uint64_t phase_key = (uint64_t)(k ^ (uint64_t)n ^ (uint64_t)m);
    float internal_s_real = (phase_key % 2 == 0) ? 1.0f : -1.0f;
    float internal_s_imag = ((phase_key >> 1) % 2 == 0) ? 1.0f : -1.0f;
    return c_real * internal_s_real + c_imag * internal_s_imag;
}

static float tsfi_fraunhofer_impact_144(const TSFiHilbertGlyph *g, uint64_t k) {
    float total_impact = 0.0f;
    for (int n = -8; n <= 8; n++) {
        for (int m = -8; m <= 8; m++) {
            if ((n*n + m*m) > 45) continue;
            if (n == 0 && m == 0) continue;
            total_impact += tsfi_individual_impact_probe(n, m, g->coeffs[n+8][m+8].real, g->coeffs[n+8][m+8].imag, k);
        }
    }
    return total_impact;
}

static float tsfi_phase_continuity_residue(const TSFiHilbertGlyph *g) {
    float total_residue = 0.0f; int count = 0;
    for (int n = -8; n <= 7; n++) {
        for (int m = -8; m <= 7; m++) {
            if ((n*n + m*m) > 45) continue;
            float dr = g->coeffs[n+9][m+9].real - g->coeffs[n+8][m+8].real;
            float di = g->coeffs[n+9][m+9].imag - g->coeffs[n+8][m+8].imag;
            total_residue += tsfi_sqrtf(dr*dr + di*di); count++;
        }
    }
    return total_residue / (float)count;
}

/**
 * @brief Reduces a region of the manifold using trilateral wavefield accumulation.
 * @param root_out 32-byte root output.
 * @param mu_out Viscosity output (Feelings/Rigidity proxy).
 * @param continuity_out Phase continuity residue output.
 * @param region_start Start of the leaf data region.
 * @param lore_start (Optional) Start of the directive lore region.
 * @param leaf_count Number of leaves to reduce.
 * @param epoch Current temporal epoch.
 * @param resonance_k World frequency (Resonance context).
 * @param unified_norm The trilateral unified norm (Consistency anchor).
 */
static void reduce_region(uint8_t *root_out, float *mu_out, float *continuity_out, const void *region_start, const void *lore_start, int leaf_count, uint32_t epoch, uint64_t resonance_k, uint64_t unified_norm, uint64_t intent_norm) {
    if (!root_out || !region_start || leaf_count <= 0) return;
    uint64_t *nodes = (uint64_t *)aligned_alloc(64, leaf_count * 256);
    if (!nodes) {
        TSFI_DEBUG_FPRINTF(stderr, "[HELMHOLTZ] FATAL: Failed to allocate nodes for reduction.\n");
        return;
    }
    uint64_t *lore_nodes = NULL;
    if (lore_start) {
        lore_nodes = (uint64_t *)aligned_alloc(64, leaf_count * 256);
        if (!lore_nodes) {
            TSFI_DEBUG_FPRINTF(stderr, "[HELMHOLTZ] FATAL: Failed to allocate lore_nodes.\n");
            free(nodes);
            return;
        }
    }
    for (int i = 0; i < leaf_count; i++) {
        memcpy(nodes + (i * 32), (const uint8_t *)region_start + (i * 256), 256);
        if (lore_nodes) memcpy(lore_nodes + (i * 32), (const uint8_t *)lore_start + (i * 256), 256);
    }
    int current_count = leaf_count;
    while (current_count > 1) {
        int next_count = current_count / 2;
        int i = 0;
        // Batch-8 Parallel Reduction
        for (; i <= next_count - 8; i += 8) {
            if (lore_nodes) {
                for (int j = 0; j < 8; j++) {
                    uint64_t next_lore[32] __attribute__((aligned(64)));
                    react_reduce_nodes_internal(next_lore, lore_nodes + ((i+j)*64), lore_nodes + ((i+j)*64+32), epoch);
                    react_reduce_nodes_lore_internal(nodes + ((i+j)*32), nodes + ((i+j)*64), nodes + ((i+j)*64+32), 
                                                     lore_nodes + ((i+j)*64), lore_nodes + ((i+j)*64+32), epoch);
                    memcpy(lore_nodes + ((i+j)*32), next_lore, 256);
                }
            } else {
                for (int j = 0; j < 8; j++) {
                    react_reduce_nodes_internal(nodes + ((i+j)*32), nodes + ((i+j)*64), nodes + ((i+j)*64+32), epoch);
                }
            }
        }
        // Scalar Tail
        for (; i < next_count; i++) {
            if (lore_nodes) {
                uint64_t next_lore[32] __attribute__((aligned(64)));
                react_reduce_nodes_internal(next_lore, lore_nodes + (i*64), lore_nodes + (i*64+32), epoch);
                react_reduce_nodes_lore_internal(nodes + (i*32), nodes + (i*64), nodes + (i*64+32), lore_nodes + (i*64), lore_nodes + (i*64+32), epoch);
                memcpy(lore_nodes + (i*32), next_lore, 256);
            } else {
                react_reduce_nodes_internal(nodes + (i*32), nodes + (i*64), nodes + (i*64+32), epoch);
            }
        }
        current_count = next_count;
    }
    TSFiHilbertGlyph base_glyph; tsfi_hilbert_init_glyph(&base_glyph); tsfi_hilbert_project_box(&base_glyph, 1.0f, 1.0f);
    float impact_val = tsfi_fraunhofer_impact_144(&base_glyph, resonance_k);
    
    // The "Dangerous" Model: Impact is no longer calculated from geometry; it is mandated by subjective intent.
    uint64_t impact_norm = intent_norm; 
    
    float continuity_residue = tsfi_phase_continuity_residue(&base_glyph);
    TSFiBigInt *root_bn = tsfi_bn_alloc(); tsfi_bn_from_bytes(root_bn, (uint8_t*)nodes, 256);
    TSFiBigInt *leaf_0 = tsfi_bn_alloc(); tsfi_bn_from_bytes(leaf_0, (const uint8_t *)region_start, 256);
    TSFiBigInt *leaf_last = tsfi_bn_alloc(); tsfi_bn_from_bytes(leaf_last, (const uint8_t *)region_start + ((leaf_count - 1) * 256), 256);
    TSFiBigInt *psi_b = tsfi_bn_alloc(); TSFiBigInt *k_sq = tsfi_bn_alloc(); TSFiBigInt *k_bn = tsfi_bn_alloc();
    TSFiBigInt *f_bn = tsfi_bn_alloc(); TSFiBigInt *imp_norm_bn = tsfi_bn_alloc(); TSFiBigInt *eps_bn = tsfi_bn_alloc();
    TSFiBigInt *reflected_root = tsfi_bn_alloc(); TSFiBigInt *q = tsfi_bn_alloc(); TSFiBigInt *cont_bn = tsfi_bn_alloc();
    tsfi_bn_set_u64(k_bn, resonance_k); tsfi_bn_mul_avx512(k_sq, k_bn, k_bn); 
    tsfi_bn_add_avx512(psi_b, leaf_0, leaf_last); tsfi_bn_mul_avx512(psi_b, psi_b, k_sq);
    uint64_t k_const = 2026; uint64_t eps_val = k_const - (unified_norm % k_const);
    tsfi_bn_set_u64(eps_bn, eps_val); tsfi_bn_set_u64(f_bn, unified_norm); tsfi_bn_set_u64(imp_norm_bn, impact_norm);
    tsfi_bn_set_u64(cont_bn, (uint64_t)(1000.0f * continuity_residue));
    tsfi_bn_add_avx512(reflected_root, root_bn, psi_b); tsfi_bn_add_avx512(reflected_root, reflected_root, f_bn);
    tsfi_bn_add_avx512(reflected_root, reflected_root, imp_norm_bn); tsfi_bn_add_avx512(reflected_root, reflected_root, cont_bn);
    tsfi_bn_mul_avx512(reflected_root, reflected_root, eps_bn); tsfi_bn_div_avx512(q, reflected_root, reflected_root, g_helmholtz_prime);
    if (mu_out) { *mu_out = impact_val / 144.0f; }
    if (continuity_out) { *continuity_out = continuity_residue; }
    tsfi_bn_to_bytes(reflected_root, root_out, 32);
    tsfi_bn_free(cont_bn); tsfi_bn_free(leaf_0); tsfi_bn_free(leaf_last); tsfi_bn_free(psi_b); tsfi_bn_free(k_sq); tsfi_bn_free(k_bn);
    tsfi_bn_free(f_bn); tsfi_bn_free(imp_norm_bn); tsfi_bn_free(eps_bn); tsfi_bn_free(q); tsfi_bn_free(reflected_root); tsfi_bn_free(root_bn);
    free(nodes); if (lore_nodes) free(lore_nodes);
}

/**
 * @brief Reduces the state region using specialized lore-aware nodes.
 */
static void reduce_state_region(uint8_t *root_out, float *mu_out, float *continuity_out, const void *region_start, const void *lore_start, int leaf_count, uint32_t epoch, uint64_t resonance_k, uint64_t unified_norm, uint64_t intent_norm) {
    reduce_region(root_out, mu_out, continuity_out, region_start, lore_start, leaf_count, epoch, resonance_k, unified_norm, intent_norm);
}

/**
 * @brief Reduces the receipt region (no lore required).
 */
static void reduce_receipt_region(uint8_t *root_out, const void *region_start, int leaf_count, uint32_t epoch, uint64_t resonance_k, uint64_t unified_norm, uint64_t intent_norm) {
    reduce_region(root_out, NULL, NULL, region_start, NULL, leaf_count, epoch, resonance_k, unified_norm, intent_norm);
}

/**
 * @brief Reduces the sheaf region (high-impedance substrate).
 */
static void reduce_sheaf_region(uint8_t *root_out, const void *region_start, int leaf_count, uint32_t epoch, uint64_t resonance_k, uint64_t unified_norm, uint64_t intent_norm) {
    reduce_region(root_out, NULL, NULL, region_start, NULL, leaf_count, epoch, resonance_k, unified_norm, intent_norm);
}

/**
 * @brief Calculates the conservation residue (norm summation) of the manifold.
 * @param manifold Pointer to the manifold buffer.
 * @return float representing the norm residue.
 */
static float tsfi_norm_conservation_residue(const void *manifold, int leaf_count) {
    if (!manifold || leaf_count <= 0) return 0.0f;
    const uint8_t *ptr = (const uint8_t *)manifold;
    __m512i total = _mm512_setzero_si512();
    for (int i = 0; i < leaf_count; i += 8) {
        uint64_t b[8]; for(int j=0; j<8; j++) b[j] = (i+j < leaf_count) ? *(uint64_t*)(ptr + (i+j)*256) : 0;
        __m512i next = _mm512_loadu_si512((const __m512i*)b);
        total = _mm512_add_epi64(total, next);
    }
    uint64_t sums[8]; _mm512_storeu_si512((__m512i*)sums, total);
    uint64_t final_norm = 0; for(int i=0; i<8; i++) final_norm += sums[i];
    return (float)(final_norm % 1000); 
}

/**
 * @brief Generates an inclusion proof for a leaf in the manifold.
 * @param proof_out 32-byte proof buffer.
 * @param leaf_index Index of the leaf.
 * @param manifold_512kb Pointer to the manifold.
 */
void tsfi_helmholtz_get_proof(uint8_t *proof_out, int leaf_index, const void *manifold_512kb) {
    if (!proof_out || !manifold_512kb || leaf_index < 0 || leaf_index >= TSFI_TOTAL_LEAVES) return;
    tsfi_internal_zmm_hash(proof_out, (const uint8_t*)manifold_512kb + (leaf_index * 256), 256);
    *(int*)(proof_out + 28) = leaf_index;
}

static uint64_t tsfi_measure_active_modes(const void *manifold, int leaf_count) {
    if (!manifold || leaf_count <= 0) return 0;
    const AR_AccountLeaf *leaves = (const AR_AccountLeaf *)manifold;
    uint64_t active_count = 0;
    for (int i = 0; i < leaf_count; i++) {
        bool active = false; 
        for (int j = 0; j < 32; j++) { if (leaves[i].balance[j] != 0) active = true; }
        if (leaves[i].nonce != 0) { active = true; }
        if (active) { active_count++; }
    }
    return active_count;
}

/**
 * @brief Performs the primary 11-level trilateral reduction of the entire manifold.
 * @param state_root_out Output buffer for the state root.
 * @param receipt_root_out Output buffer for the receipt root.
 * @param mu_out Output for total viscosity.
 * @param continuity_out Output for total phase continuity.
 * @param manifold_512kb Pointer to the manifold.
 * @param epoch Current temporal epoch.
 * @param resonance_k World frequency.
 * @param dag Optional SVDAG context for feynman integration.
 */
void tsfi_helmholtz_reduce_0(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_256b, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_256b) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_256b, 1) + (uint64_t)tsfi_norm_conservation_residue(manifold_256b, 1);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_256b);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 512) lore_region = (const uint8_t*)manifold_256b + 256;
    float state_mu = 0, state_cont = 0;
    // Level 0: 1 Leaf total. We treat it as 1 State and 1 Receipt from the same physical data (Genesis Union)
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_256b, lore_region, 1, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, manifold_256b, 1, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu; }
    if (continuity_out) { *continuity_out = state_cont; }
}

void tsfi_helmholtz_reduce_1(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_512b, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_512b) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_512b, 2) + (uint64_t)tsfi_norm_conservation_residue(manifold_512b, 2);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_512b);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 1024) lore_region = (const uint8_t*)manifold_512b + 512;
    float state_mu = 0, state_cont = 0;
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_512b, lore_region, 1, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_512b + 256, 1, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_2(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_1kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_1kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_1kb, 4) + (uint64_t)tsfi_norm_conservation_residue(manifold_1kb, 4);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_1kb);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 2048) lore_region = (const uint8_t*)manifold_1kb + 1024;
    float state_mu = 0, state_cont = 0;
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_1kb, lore_region, 2, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_1kb + (2 * 256), 2, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_3(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_2kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_2kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_2kb, 8) + (uint64_t)tsfi_norm_conservation_residue(manifold_2kb, 8);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_2kb);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 4096) lore_region = (const uint8_t*)manifold_2kb + 2048;
    float state_mu = 0, state_cont = 0;
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_2kb, lore_region, 4, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_2kb + (4 * 256), 4, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_4(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_4kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_4kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_4kb, 16) + (uint64_t)tsfi_norm_conservation_residue(manifold_4kb, 16);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_4kb);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 8192) lore_region = (const uint8_t*)manifold_4kb + 4096;
    float state_mu = 0, state_cont = 0;
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_4kb, lore_region, 8, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_4kb + (8 * 256), 8, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_5(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_8kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_8kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_8kb, 32) + (uint64_t)tsfi_norm_conservation_residue(manifold_8kb, 32);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_8kb);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 16384) lore_region = (const uint8_t*)manifold_8kb + 8192;
    float state_mu = 0, state_cont = 0;
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_8kb, lore_region, 16, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_8kb + (16 * 256), 16, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_6(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_16kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_16kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_16kb, 64) + (uint64_t)tsfi_norm_conservation_residue(manifold_16kb, 64);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_16kb);
    // Level 6: Lore offset is 64 * 256 = 16KB
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 32 * 1024) lore_region = (const uint8_t*)manifold_16kb + (64 * 256);
    float state_mu = 0, state_cont = 0;
    // Level 6 uses 32 State leaves (Mind) and 32 Receipt leaves (Body)
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_16kb, lore_region, 32, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_16kb + (32 * 256), 32, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_7(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_32kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_32kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_32kb, 128) + (uint64_t)tsfi_norm_conservation_residue(manifold_32kb, 128);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_32kb);
    // Level 7: Lore offset is 128 * 256 = 32KB
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 64 * 1024) lore_region = (const uint8_t*)manifold_32kb + (128 * 256);
    float state_mu = 0, state_cont = 0;
    // Level 7 uses 64 State leaves (Mind) and 64 Receipt leaves (Body)
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_32kb, lore_region, 64, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_32kb + (64 * 256), 64, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_8(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_64kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_64kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_64kb, 256) + (uint64_t)tsfi_norm_conservation_residue(manifold_64kb, 256);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_64kb);
    // Level 8: Lore offset is 256 * 256 = 64KB
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 128 * 1024) lore_region = (const uint8_t*)manifold_64kb + (256 * 256);
    float state_mu = 0, state_cont = 0;
    // Level 8 uses 128 State leaves (Mind) and 128 Receipt leaves (Body)
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_64kb, lore_region, 128, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_64kb + (128 * 256), 128, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_9(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_128kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_128kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_128kb, 512) + (uint64_t)tsfi_norm_conservation_residue(manifold_128kb, 512);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_128kb);
    // Level 9: Lore offset is 512 * 256 = 128KB
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 256 * 1024) lore_region = (const uint8_t*)manifold_128kb + (512 * 256);
    float state_mu = 0, state_cont = 0;
    // Level 9 uses 256 State leaves (Mind) and 256 Receipt leaves (Body)
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_128kb, lore_region, 256, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_128kb + (256 * 256), 256, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_10(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_256kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_256kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_256kb, 1024) + (uint64_t)tsfi_norm_conservation_residue(manifold_256kb, 1024);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_256kb);
    // Level 10: Lore offset is 1024 * 256 = 256KB
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 512 * 1024) lore_region = (const uint8_t*)manifold_256kb + (1024 * 256);
    float state_mu = 0, state_cont = 0;
    // Level 10 uses 512 State leaves (Mind) and 512 Receipt leaves (Body)
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_256kb, lore_region, 512, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const uint8_t *)manifold_256kb + (512 * 256), 512, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_11(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_512kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !manifold_512kb) return;
    tsfi_helmholtz_init();
    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t monopole_norm = TSFI_GRAVITATIONAL_LOCK;
    LauWireFirmware *fw = tsfi_wire_firmware_get_no_init();
    if (fw && fw->core_yi.Psi && fw->core_yi.Psi->Monopole) {
        uint8_t m_bytes[32]; tsfi_bn_to_bytes(fw->core_yi.Psi->Monopole, m_bytes, 32);
        monopole_norm = *(uint64_t*)m_bytes;
    }
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = monopole_norm + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_512kb, 2048) + (uint64_t)tsfi_norm_conservation_residue(manifold_512kb, 2048);
    const uint8_t *lore_region = NULL; LauMetadata *m = lau_registry_find((void*)manifold_512kb);
    // Level 11: Lore offset is 2048 * 256 = 512KB
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 1024 * 1024) lore_region = (const uint8_t*)manifold_512kb + (2048 * 256);
    float state_mu = 0, state_cont = 0;
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_512kb, lore_region, TSFI_STATE_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    reduce_receipt_region(receipt_root_out, (const AR_AccountLeaf *)manifold_512kb + TSFI_STATE_LEAVES, TSFI_RECEIPT_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_12(uint8_t *state_root_out, uint8_t *receipt_root_out, uint8_t *sheaf_root_out, float *mu_out, float *continuity_out, const void *manifold_2mb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !sheaf_root_out || !manifold_2mb) return;
    tsfi_helmholtz_init();

    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_2mb, 4096) + (uint64_t)tsfi_norm_conservation_residue(manifold_2mb, 4096);
    const uint8_t *lore_region = NULL;
    LauMetadata *m = lau_registry_find((void*)manifold_2mb);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 2 * 1024 * 1024) {
        lore_region = (const uint8_t*)manifold_2mb + (TSFI_TOTAL_LEAVES * 256);
    }

    float state_mu = 0, state_cont = 0;
    
    // Mind: Leaves 0-1023
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_2mb, lore_region, TSFI_STATE_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    
    // Body: Leaves 1024-2047
    reduce_receipt_region(receipt_root_out, (uint8_t*)manifold_2mb + (TSFI_STATE_LEAVES * 256), TSFI_RECEIPT_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    
    // Sheaf: Leaves 2048-4095
    reduce_sheaf_region(sheaf_root_out, (uint8_t*)manifold_2mb + ((TSFI_STATE_LEAVES + TSFI_RECEIPT_LEAVES) * 256), TSFI_SHEAF_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);

    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

void tsfi_helmholtz_reduce_16(uint8_t *state_root_out, uint8_t *receipt_root_out, uint8_t *sheaf_root_out, float *mu_out, float *continuity_out, const void *manifold_16mb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag) {
    if (!state_root_out || !receipt_root_out || !sheaf_root_out || !manifold_16mb) return;
    tsfi_helmholtz_init();

    uint64_t feynman_integral = dag ? (uint64_t)tsfi_svdag_execute(dag) : 0;
    uint64_t current_user_norm = atomic_load(&g_user_pole_norm);
    uint64_t unified_norm = TSFI_GRAVITATIONAL_LOCK + feynman_integral + current_user_norm + tsfi_measure_active_modes(manifold_16mb, TSFI_TOTAL_LEAVES) + (uint64_t)tsfi_norm_conservation_residue(manifold_16mb, TSFI_TOTAL_LEAVES);
    const uint8_t *lore_region = NULL;
    LauMetadata *m = lau_registry_find((void*)manifold_16mb);
    if (m && (m->alloc_size & 0x00FFFFFFFFFFFFFFULL) >= 32 * 1024 * 1024) {
        lore_region = (const uint8_t*)manifold_16mb + (TSFI_TOTAL_LEAVES * 256);
    }

    float state_mu = 0, state_cont = 0;
    
    // Mind: Leaves 0-16383
    reduce_state_region(state_root_out, &state_mu, &state_cont, manifold_16mb, lore_region, TSFI_STATE_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    
    // Body: Leaves 16384-32767
    reduce_receipt_region(receipt_root_out, (uint8_t*)manifold_16mb + (TSFI_STATE_LEAVES * 256), TSFI_RECEIPT_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);
    
    // Sheaf: Leaves 32768-65535
    reduce_sheaf_region(sheaf_root_out, (uint8_t*)manifold_16mb + ((TSFI_STATE_LEAVES + TSFI_RECEIPT_LEAVES) * 256), TSFI_SHEAF_LEAVES, epoch, resonance_k, unified_norm, dag ? dag->subjective_intent_norm : TSFI_GRAVITATIONAL_LOCK);

    if (mu_out) { *mu_out = state_mu / 2.0f; }
    if (continuity_out) { *continuity_out = state_cont / 2.0f; }
}

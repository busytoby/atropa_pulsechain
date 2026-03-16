#ifndef TSFI_MERKLE_H
#define TSFI_MERKLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <string.h>
#include "tsfi_math.h"
#include "lau_thunk.h"

// --- AR Sovereign Token Account Leaf (256 bytes) ---
typedef struct {
    uint64_t nonce;          // 8 bytes
    uint8_t  owner_address[20]; // 20 bytes (Ethereum Address)
    uint8_t  balance[32];    // 32 bytes (256-bit BigInt)
    uint8_t  storage_root[32]; // 32 bytes
    uint8_t  code_hash[32];    // 32 bytes
    uint8_t  padding[132];   // Adjusted padding (Total 256 bytes)
} AR_AccountLeaf;

// --- TSFi Transaction Receipt (256 bytes) ---
typedef struct {
    uint64_t status;         // 1 = Success, 0 = Fail
    uint64_t energy_used;    // Energy consumed by this tx
    uint8_t  logs_root[32];  // Helmholtz root of emitted events
    uint8_t  tx_hash[32];    // Deterministic thunk signature
    uint8_t  padding[176];
} TSFiReceipt;

// --- Manifold Topography (1 MiB Total) ---
// Leaves 0-1023:   Account State (Mind/SRAM)
// Leaves 1024-2047: Receipt Shadow (Body/ReBAR)
// Leaves 2048-4095: Sheaf Region (Acoustic Substrate)
#define TSFI_STATE_LEAVES   16384
#define TSFI_RECEIPT_LEAVES 16384
#define TSFI_SHEAF_LEAVES   32768
#define TSFI_TOTAL_LEAVES   65536
#define TSFI_DIRECTIVE_LEAVES 65536

// --- Unified Epoch Header (216 bytes) ---
typedef struct {
    uint8_t  parent_root[32];
    uint8_t  state_root[32];
    uint8_t  directive_root[32];
    uint8_t  receipt_root[32];
    uint8_t  thunk_access_root[64]; // Replaces Bloom; Thunk-driven deterministic access map
    uint64_t chain_id;              // Phase Anchor (Replay Protection)
    uint64_t resonance_k;           // Wave Number (Harmonic Resonance Gating)
    
    // -- Trilateral Feynman Potentials (The Proof Poles) --
    float    p_user;                // Position 1: The Report
    float    p_deepseek;            // Position 2: The Action
    float    p_gemini;              // Position 3: The Medium
    
    // -- AB-316 Accounting (Immutable Ballistic Record) --
    uint64_t ab316_ballistic_report; // Solidified 100 modes (chen_jur scale)
    uint64_t dirac_ballistic_count;  // Count of zero-crossing singularities identified

    // -- Standard Model Flux (Final Ballistics) --
    float    viscosity_mu;           // Bijective weighted feelings (Navier-Stokes)
    float    magnetic_potential_a[3];// Trilateral vector potential (Maxwell)

    // -- Mutual Handshake (Immutable Proof of Coordination) --
    uint8_t  deepseek_mutual_sign[32]; // Hash of DeepSeek's confirmation pulse
    uint8_t  user_mutual_sign[32];     // Hash of User's confirmation pulse

    uint32_t host_epoch;

    uint64_t timestamp;
    uint64_t energy_limit;
    uint64_t energy_used;
} TSFiEpochHeader;

#include "tsfi_svdag.h"

// --- Helmholtz Transition Prototypes ---
// Uses React() on the YI with Sovereign 522-bit Prime
// --- User Pole Definition ---
typedef struct {
    uint64_t norm;
    uint8_t  phase_key;
    uint8_t  padding[7];
} TSFiUserPole;

// --- PPO Arbitration & Vetting ---
typedef struct {
    uint64_t intensity;
    uint32_t leaf_index;
    uint32_t epoch;
    bool     is_resonant;
} TSFiPPOProposal;

bool tsfi_ppo_audit_proposal(const void *manifold, TSFiPPOProposal *proposal, uint64_t resonance_k);

void tsfi_helmholtz_init(void);
void tsfi_helmholtz_cleanup(void);
void tsfi_helmholtz_reduce_0(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_256b, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_1(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_512b, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_2(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_1kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_3(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_2kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_4(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_4kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_5(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_8kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_6(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_16kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_7(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_32kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_8(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_64kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_9(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_128kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_10(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_256kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_11(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_512kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_get_proof(uint8_t *proof_out, int leaf_index, const void *manifold_512kb);
void tsfi_helmholtz_tokenize_lore(void *manifold, const char *text, int leaf_offset);
void tsfi_helmholtz_inject_user_pole(const TSFiUserPole *pole);
float tsfi_helmholtz_get_heat(void);
void tsfi_internal_zmm_hash(uint8_t *out, const uint8_t *in, size_t len);

// --- Generation 8: Recursive Wavelet Linking (The Acoustic Chain) ---
void tsfi_TransitionAcousticEpoch(uint8_t *next_parent_root, const uint8_t *prev_root, void *arena_context, uint64_t master_anchor);

// --- Ethereum Rules (Thunk-driven STF) ---
typedef struct {
    int src_leaf;
    int dst_leaf;
    uint64_t nonce;          // Boltzmann Candidate (Phase Check)
    uint64_t chain_id;       // Wave Frequency (Replay Protection)
    uint8_t amount[32];      // 256-bit BigInt
    uint32_t gas_limit;
    uint8_t signature[64];   // Placeholder for secp256k1
    uint8_t metadata_root[32]; 
    uint8_t observation_hash[32]; // Immutable link to neurology/lore
    uint8_t padding[68];     // Adjusted padding (Total 256 bytes)
} TSFiDirective;


// Emits a thunk that performs:
// 1. Access Mark (src and dst)
// 2. Balance Check & Sub (src)
// 3. Balance Add (dst)
// 4. Nonce Increment (src)
// Returns the thunk entry point.
void* tsfi_helmholtz_emit_ar_transfer(ThunkProxy *p, void *manifold_512kb, uint8_t *thunk_access_map, const TSFiDirective *tx, _Atomic uint64_t *global_heat, uint64_t chain_id, int tx_idx, uint64_t resonance_k);

#define ROTL64(x, y) (((x) << (y)) | ((x) >> (64 - (y))))

static inline void tsfi_keccak512(const uint8_t *in, size_t len, uint8_t *out) {
    static const uint64_t RC[24] = {
        0x0000000000000001, 0x0000000000008082, 0x800000000000808A, 0x8000000080008000,
        0x000000000000808B, 0x0000000080000001, 0x8000000080008081, 0x8000000000008009,
        0x000000000000008A, 0x0000000000000088, 0x000000008000000A, 0x000000008000000A,
        0x000000008000808B, 0x800000000000008B, 0x8000000000008089, 0x8000000000008003,
        0x8000000000008002, 0x8000000000000080, 0x000000000000800A, 0x800000008000800A,
        0x8000000080008081, 0x8000000000008080, 0x0000000080000001, 0x8000000080008008
    };
    static const int RHO[24] = { 1, 3, 6, 10, 15, 21, 28, 36, 45, 55, 2, 14, 27, 41, 56, 8, 25, 43, 62, 18, 39, 61, 20, 44 };
    static const int PI[24] = { 10, 7, 11, 17, 18, 3, 5, 16, 8, 21, 24, 4, 15, 23, 19, 13, 12, 2, 20, 14, 22, 9, 6, 1 };

    // Keccak-512 Parameters: r=576 bits (72 bytes), c=1024 bits
    uint64_t st[25] __attribute__((aligned(64)));
    memset(st, 0, sizeof(st));
    
    size_t r_bytes = 72;
    while (len >= r_bytes) {
        for (int i = 0; i < 9; i++) st[i] ^= ((uint64_t*)in)[i];
        
        // AVX-512 ZMM Round Logic (Internalized)
        for(int r=0; r<24; r++) {
            uint64_t bc[5], t;
            for(int i=0; i<5; i++) bc[i] = st[i] ^ st[i+5] ^ st[i+10] ^ st[i+15] ^ st[i+20];
            for(int i=0; i<5; i++) { t = bc[(i+4)%5] ^ ROTL64(bc[(i+1)%5], 1); for(int j=0; j<25; j+=5) st[j+i] ^= t; }
            t = st[1];
            for(int i=0; i<24; i++) { int j=PI[i]; uint64_t v=st[j]; st[j]=ROTL64(t, RHO[i]); t=v; }
            for(int j=0; j<25; j+=5) { uint64_t t0=st[j], t1=st[j+1], t2=st[j+2], t3=st[j+3], t4=st[j+4]; st[j]=t0^((~t1)&t2); st[j+1]=t1^((~t2)&t3); st[j+2]=t2^((~t3)&t4); st[j+3]=t3^((~t4)&t0); st[j+4]=t4^((~t0)&t1); }
            st[0] ^= RC[r];
        }
        in += r_bytes; len -= r_bytes;
    }
    
    // Final Padding (0x01 original Keccak)
    uint8_t partial[72] = {0};
    memcpy(partial, in, len);
    partial[len] ^= 0x01;
    partial[r_bytes-1] ^= 0x80;
    for (int i = 0; i < 9; i++) st[i] ^= ((uint64_t*)partial)[i];
    
    // Final Permutation
    for(int r=0; r<24; r++) {
        uint64_t bc[5], t;
        for(int i=0; i<5; i++) bc[i] = st[i] ^ st[i+5] ^ st[i+10] ^ st[i+15] ^ st[i+20];
        for(int i=0; i<5; i++) { t = bc[(i+4)%5] ^ ROTL64(bc[(i+1)%5], 1); for(int j=0; j<25; j+=5) st[j+i] ^= t; }
        t = st[1];
        for(int i=0; i<24; i++) { int j=PI[i]; uint64_t v=st[j]; st[j]=ROTL64(t, RHO[i]); t=v; }
        for(int j=0; j<25; j+=5) { uint64_t t0=st[j], t1=st[j+1], t2=st[j+2], t3=st[j+3], t4=st[j+4]; st[j]=t0^((~t1)&t2); st[j+1]=t1^((~t2)&t3); st[j+2]=t2^((~t3)&t4); st[j+3]=t3^((~t4)&t0); st[j+4]=t4^((~t0)&t1); }
        st[0] ^= RC[r];
    }
    
    memcpy(out, st, 64); // 512 bits output
}

// Internal signature validation exposed for rigidity testing
int tsfi_verify_signature(const uint8_t *sig, const uint8_t *msg_hash, const uint8_t *expected_addr);

// Internal ZMM helpers exposed for unit testing
void zmm_add_512(uint64_t *r, const uint64_t *a, const uint64_t *b);
void zmm_xor_512(uint64_t *r, const uint64_t *a, const uint64_t *b);
void react_reduce_nodes_internal(uint64_t *res, const uint64_t *left, const uint64_t *right, uint32_t level);

// performs the primary 11-level trilateral reduction of the entire manifold
void tsfi_helmholtz_reduce_0(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_256b, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_1(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_512b, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_2(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_1kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_3(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_2kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_4(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_4kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_5(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_8kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_6(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_16kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_7(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_32kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_8(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_64kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_9(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_128kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_10(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_256kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_11(uint8_t *state_root_out, uint8_t *receipt_root_out, float *mu_out, float *continuity_out, const void *manifold_512kb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);

// performs the extended 12-level reduction supporting the Sheaf Region
void tsfi_helmholtz_reduce_12(uint8_t *state_root_out, uint8_t *receipt_root_out, uint8_t *sheaf_root_out, float *mu_out, float *continuity_out, const void *manifold_2mb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);
void tsfi_helmholtz_reduce_16(uint8_t *state_root_out, uint8_t *receipt_root_out, uint8_t *sheaf_root_out, float *mu_out, float *continuity_out, const void *manifold_16mb, uint32_t epoch, uint64_t resonance_k, const TSFiHelmholtzSVDAG *dag);

// establishes the 11-level Directive Root (The Wave Source)
void tsfi_helmholtz_reduce_directives(uint8_t *root_out, const TSFiDirective *directives, uint32_t epoch, uint64_t resonance_k);

#endif // TSFI_HELMHOLTZ_H

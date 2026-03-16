#ifndef TSFI_OPT_ZMM_H
#define TSFI_OPT_ZMM_H

#include <stdint.h>
#include <stddef.h>

// ZMM Register Types
typedef enum {
    ZMM_TYPE_UNUSED = 0,
    ZMM_TYPE_VGPR   = 1, // Float Data
    ZMM_TYPE_SGPR   = 2, // Int Data
    ZMM_TYPE_WMMA   = 3, // Matrix Accumulator
    ZMM_TYPE_LDS    = 4, // Shared Memory
    ZMM_TYPE_MASS   = 5  // Rendering Mass
} ZmmType;

// Configuration for a single ZMM register
typedef struct {
    ZmmType type;
    void *data_ptr; // Pointer to 64-byte aligned data
} ZmmSlot;

// Synaptic Feedback State
typedef struct {
    float mass_density;       // Current accumulated density
    float convergence_rate;   // How fast solution is stabilizing
    uint32_t vgpr_pressure;   // Register utilization hints
    int request_kernel_swap;  // 0 = No, 1 = Upgrade, -1 = Downgrade
} ZmmSynapse;

// Full 32-Register Configuration Block
typedef struct {
    volatile uint32_t lock;     // Atomic Lock
    uint32_t padding[15];
    
    ZmmSlot slots[32];          // Map for ZMM0-ZMM31
    uint32_t active_mask;       // Bitmask of active ZMM registers (1 = active)
    uint8_t cache_policy;       // ZmmCachePolicy hint
    uint8_t _pad_cache[7];
    void *contiguous_rf;        // 2KB Contiguous Register File (ZMM0-31)
    ZmmSynapse synapse;         // Feedback Channel
    
    // The Micro-Kernel to execute
    void (*micro_kernel)(void*, ZmmSynapse*); 

    // The Density Analysis Kernel (Thunked)
    // Responsible for updating synapse->mass_density based on ZMM state
    void (*density_kernel)(void*, ZmmSynapse*);

    // Persistent Execution Control
    // If > 0, the micro_kernel is executed this many times BEFORE storing registers back.
    // This simulates GPU occupancy (registers stay hot) vs CPU visitation (load/store).
    uint64_t persistent_cycles;

    // Alternate Kernels for Hot-Swapping
    void (*kernel_low_density)(void*, ZmmSynapse*);
    void (*kernel_high_density)(void*, ZmmSynapse*);

} TsfiZmmManifest;

// Dynamic Dispatcher with Synaptic Adaptation
// 1. Locks
// 2. Loads ZMM0-31
// 3. Calls micro_kernel(regs, &synapse)
// 4. Checks synapse.request_kernel_swap -> Updates micro_kernel ptr
// 5. Stores ZMM0-31
// 6. Unlocks
void tsfi_dispatch_zmm_dynamic(TsfiZmmManifest *manifest);

// Unity Dispatcher (Enforces LauWiredHeader Bijection)
// Verifies memory provenance before executing dispatch.
void tsfi_dispatch_unity(TsfiZmmManifest *manifest);

// High-Bandwidth Scramble (Wave512 Mnemonic PRNG)
// Uses AVX-512 to fill memory with cryptographic-strength noise at line speed (20GB/s+).
// ptr must be 64-byte aligned (ideally 512-byte aligned).
void tsfi_scramble_wave512(void *ptr, size_t size);

// --- High-Speed SD Worker Kernels ---
void tsfi_kernel_blue_teddy_bear(void *regs, ZmmSynapse *syn);

#endif // TSFI_OPT_ZMM_H
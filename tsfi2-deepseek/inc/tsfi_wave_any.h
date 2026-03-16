#ifndef TSFI_WAVE_ANY_H
#define TSFI_WAVE_ANY_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdalign.h> // Added for alignas
#include <immintrin.h> // Added for AVX-512 types in wave_store_any_avx512

// --- Universal Wave Descriptor ---
// Realizes the WAVE(Any_Number) architecture.
// Decouples the logical data (Atom) from the hardware-optimized layout (Stride).
// Design is strictly bijective with RDNA 4 Compute Models (GPU Compatible).
typedef struct {
    void*    data;          // Pointer to the start of the stream
    uint32_t count;         // Number of atoms in the stream
    uint32_t atom_size;     // Logical size (VE1 - VE512+)
    uint32_t stride;        // Physical distance between atoms
    uint32_t flags;         // Bit 0: Is Device Local, Bit 1: Is Host Visible
} WaveStream;

#define WAVE_FLAG_CHECKSUMMED 0x4
#define WAVE_FLAG_BACKPRESSURE 0x8
#define WAVE_FLAG_VERIFY       0x10

// --- Accessor Macros ---
#define WAVE_SECRET(stream, i) ((size_t)(i) * (stream)->stride)
#define WAVE_PTR(stream, i) ((void*)((uint8_t*)(stream)->data + WAVE_SECRET(stream, i)))

// --- Simple Bijective Hash for Segment Integrity ---
static inline uint64_t wave_hash_segment(const void* data, size_t size, uint32_t salt) {
    uint64_t hash = salt;
    uint8_t* p = (uint8_t*)data;
    for (size_t i = 0; i < size; i++) {
        hash = (hash ^ p[i]) * 0xBF58476D1CE4E5B9ULL;
    }
    return hash;
}

// --- Robust OOB-Aware Write (Generic / GPU Compatible) ---
// Mimics VK_KHR_robustness2 behavior with active splitting.
// If 'size' exceeds 'atom_size', the write is partitioned across atoms starting at index 'i'.
static inline void wave_store_any(const WaveStream* s, uint32_t i, const void* src, size_t size) {
    if (!s || !s->data || i >= s->count) return;

    uint8_t* src_ptr = (uint8_t*)src;
    size_t remaining = size;
    uint32_t current_idx = i;

    while (current_idx < s->count) {
        uint8_t* dst = (uint8_t*)WAVE_PTR(s, current_idx);
        size_t usable_atom = s->atom_size;
        if ((s->flags & WAVE_FLAG_CHECKSUMMED) && usable_atom > 8) usable_atom -= 8;

        if (remaining > 0) {
            size_t chunk = (remaining > usable_atom) ? usable_atom : remaining;
            memcpy(dst, src_ptr, chunk);
            
            // If checksumming is enabled, calculate hash on the usable portion
            if ((s->flags & WAVE_FLAG_CHECKSUMMED) && s->atom_size > 8) {
                // Clear any intra-atom padding before hashing
                if (chunk < usable_atom) {
                    memset(dst + chunk, 0, usable_atom - chunk);
                }
                uint64_t h = wave_hash_segment(dst, s->atom_size - 8, current_idx ^ 0xACE);
                *(uint64_t*)(dst + s->atom_size - 8) = h;
            } else if (chunk < s->atom_size) {
                memset(dst + chunk, 0, s->atom_size - chunk);
            }

            src_ptr += chunk;
            remaining -= chunk;
        } else {
            memset(dst, 0, s->atom_size);
        }

        if (s->stride > s->atom_size) {
            memset(dst + s->atom_size, 0, s->stride - s->atom_size);
        }
        current_idx++;
    }
}

// --- Robust OOB-Aware Read (Generic / GPU Compatible) ---
// Ensures reads are bounded by atom_size and stream count.
static inline int wave_load_any(const WaveStream* s, uint32_t i, void* dst, size_t size) {
    if (!s || !s->data || i >= s->count) {
        memset(dst, 0, size);
        return 0; // Fail
    }

    uint8_t* dst_ptr = (uint8_t*)dst;
    size_t remaining = size;
    uint32_t current_idx = i;

    while (remaining > 0 && current_idx < s->count) {
        uint8_t* src = (uint8_t*)WAVE_PTR(s, current_idx);
        size_t usable_atom = s->atom_size;
        
        // Strict Validation: Check segment integrity BEFORE reassembly
        if ((s->flags & WAVE_FLAG_CHECKSUMMED) && s->atom_size > 8) {
            usable_atom -= 8;
            uint64_t stored_h = *(uint64_t*)(src + s->atom_size - 8);
            uint64_t calc_h = wave_hash_segment(src, s->atom_size - 8, current_idx ^ 0xACE);
            if (stored_h != calc_h) {
                // BIJECTIVE BREACH: Data was coerced or corrupted
                memset(dst, 0, size);
                return -1; 
            }
        }

        size_t chunk = (remaining > usable_atom) ? usable_atom : remaining;
        memcpy(dst_ptr, src, chunk);
        
        dst_ptr += chunk;
        remaining -= chunk;
        current_idx++;
    }

    if (remaining > 0) {
        memset(dst_ptr, 0, remaining);
    }
    return 1; // Success
}

// --- Universal Packing Helper ---

// Explicitly uses wave_store_any to ensure consistent logic.

static inline void wave_pack_buffer(WaveStream* dst_stream, const void* src_buffer, size_t src_len) {

    wave_store_any(dst_stream, 0, src_buffer, src_len);

}



// --- AVX-512 Accelerated Segmented Store ---

// Stores a 512-bit vector (64 bytes) into the stream at logical byte index 'idx'.

// Handles splitting across atoms if necessary.

static inline void wave_store_any_avx512(const WaveStream* s, uint32_t idx, __m512i vData) {

    // 1. Calculate Logical Atom Index and Secret

    uint32_t atom_idx = idx / s->atom_size;

    uint32_t atom_secret = idx % s->atom_size;

    

    // 2. Check OOB

    if (atom_idx >= s->count) return;



    // 3. Fast Path: Contiguous Write (Fits in one atom)

    // Must also respect Checksum area (atom_size - 8) if enabled.

    size_t effective_size = s->atom_size;

    if ((s->flags & WAVE_FLAG_CHECKSUMMED) && s->atom_size > 8) effective_size -= 8;



    if (atom_secret + 64 <= effective_size) {

        uint8_t* dst = (uint8_t*)WAVE_PTR(s, atom_idx) + atom_secret;

        _mm512_storeu_si512((void*)dst, vData);

        // If checksummed, we need to update the hash immediately? 

        // No, partial updates are expensive. We assume atomic writes or post-pass hashing.

        // For strict correctness, we SHOULD update the hash.

        if (s->flags & WAVE_FLAG_CHECKSUMMED) {

             uint64_t h = wave_hash_segment(WAVE_PTR(s, atom_idx), effective_size, atom_idx ^ 0xACE);

             *(uint64_t*)((uint8_t*)WAVE_PTR(s, atom_idx) + effective_size) = h;

        }

    } else {

        // 4. Slow Path: Split Write (Crosses atom boundary)

        alignas(64) uint8_t temp[64];

        _mm512_store_si512((void*)temp, vData);

        wave_store_any(s, atom_idx, temp, 64); // Use the robust scalar splitter

    }

}



#endif // TSFI_WAVE_ANY_H

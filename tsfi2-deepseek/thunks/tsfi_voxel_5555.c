#include <immintrin.h>
#include <stdint.h>
#include <math.h>

// AVX-512 Thunk for 5555 -> 64-bit Expansion
// Symmetrical to shaders/tsfi_voxel_5555.comp

typedef struct {
    float global_intensity;
    uint32_t flags;
} VoxelUniforms;

void tsfi_voxel_5555_thunk(const void* input, void* output, size_t count, const void* uniforms) {
    const uint32_t* in = (const uint32_t*)input;
    uint16_t* out = (uint16_t*)output;
    const VoxelUniforms* u = (const VoxelUniforms*)uniforms;
    
    // Divisor for 5-bit normalization (31.0f)
    __m512 v_div = _mm512_set1_ps(1.0f / 31.0f);
    __m512 v_inten = _mm512_set1_ps(u->global_intensity);
    
    // Process 16 voxels per iteration
    size_t i = 0;
    for (; i + 16 <= count; i += 16) {
        // Load 16x 32-bit packed values
        __m512i v_packed = _mm512_loadu_si512(&in[i]);
        
        // Unpack R, G, B, S (5 bits each) -> 32-bit Integers
        // Masks
        __m512i mask5 = _mm512_set1_epi32(0x1F);
        
        // R: bits 0-4
        __m512i v_r_i = _mm512_and_si512(v_packed, mask5);
        // G: bits 5-9
        __m512i v_g_i = _mm512_and_si512(_mm512_srli_epi32(v_packed, 5), mask5);
        // B: bits 10-14
        __m512i v_b_i = _mm512_and_si512(_mm512_srli_epi32(v_packed, 10), mask5);
        // S: bits 15-19
        __m512i v_s_i = _mm512_and_si512(_mm512_srli_epi32(v_packed, 15), mask5);
        
        // Convert to Float
        __m512 v_r = _mm512_cvtepi32_ps(v_r_i);
        __m512 v_g = _mm512_cvtepi32_ps(v_g_i);
        __m512 v_b = _mm512_cvtepi32_ps(v_b_i);
        __m512 v_s = _mm512_cvtepi32_ps(v_s_i);
        
        // Normalize ( / 31.0 )
        v_r = _mm512_mul_ps(v_r, v_div);
        v_g = _mm512_mul_ps(v_g, v_div);
        v_b = _mm512_mul_ps(v_b, v_div);
        v_s = _mm512_mul_ps(v_s, v_div);
        
        // Apply Smoothing
        if (u->flags & 1) {
            v_r = _mm512_mul_ps(v_r, v_s);
            v_g = _mm512_mul_ps(v_g, v_s);
            v_b = _mm512_mul_ps(v_b, v_s);
            
            v_r = _mm512_mul_ps(v_r, v_inten);
            v_g = _mm512_mul_ps(v_g, v_inten);
            v_b = _mm512_mul_ps(v_b, v_inten);
        }
        
        // Convert to Half (Float16)
        // _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC
        __m256i h_r = _mm512_cvtps_ph(v_r, 0);
        __m256i h_g = _mm512_cvtps_ph(v_g, 0);
        __m256i h_b = _mm512_cvtps_ph(v_b, 0);
        __m256i h_s = _mm512_cvtps_ph(v_s, 0);
        
        // Interleave for Output (RGBA RGBA...)
        // We have 16 elements of R, 16 of G, etc.
        // We need to store 16 * 4 * 2 bytes = 128 bytes.
        // Structure: R0 G0 B0 A0, R1 G1 B1 A1...
        
        // Unpack Lo/Hi 16-bit
        // This unpack is tricky because h_r is 256-bit.
        // We need to cast up to 512 for the instruction, but conceptually we are merging 256s.
        
        // Correct approach with AVX-512 BW:
        // We have 16 half-floats in h_r (256-bit register).
        // We want to merge R and G.
        
        // Expand to 512? Or use 256-bit unpacks.
        __m256i h_rg_lo = _mm256_unpacklo_epi16(h_r, h_g); // R0 G0 R1 G1 ... R7 G7
        __m256i h_rg_hi = _mm256_unpackhi_epi16(h_r, h_g); // R8 G8 ... R15 G15
        
        __m256i h_ba_lo = _mm256_unpacklo_epi16(h_b, h_s); // B0 A0 ...
        __m256i h_ba_hi = _mm256_unpackhi_epi16(h_b, h_s); 
        
        // Final Merge (RGBA) - 32-bit unpack (RG + BA)
        __m256i h_rgba_lo = _mm256_unpacklo_epi32(h_rg_lo, h_ba_lo); // R0 G0 B0 A0 ...
        __m256i h_rgba_mid1 = _mm256_unpackhi_epi32(h_rg_lo, h_ba_lo); 
        
        __m256i h_rgba_mid2 = _mm256_unpacklo_epi32(h_rg_hi, h_ba_hi);
        __m256i h_rgba_hi   = _mm256_unpackhi_epi32(h_rg_hi, h_ba_hi);
        
        // Store 4x 256-bit registers (128 bytes total)
        _mm256_storeu_si256((__m256i*)&out[i*4 + 0],  h_rgba_lo);
        _mm256_storeu_si256((__m256i*)&out[i*4 + 16], h_rgba_mid1);
        _mm256_storeu_si256((__m256i*)&out[i*4 + 32], h_rgba_mid2);
        _mm256_storeu_si256((__m256i*)&out[i*4 + 48], h_rgba_hi);
    }
}

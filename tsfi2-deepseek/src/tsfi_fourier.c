#include "tsfi_c_math.h"
#include "tsfi_fourier.h"
#include "lau_memory.h"
#include <string.h>
#include <stdio.h>
#include <immintrin.h>
#include <math.h>

#define sinf tsfi_sinf
#define cosf tsfi_cosf

void tsfi_fourier_init_basis(TSFiFourierBasis *basis) {
    if (!basis) return;
    
    // Fill Basis Matrix M (K x 2N)
    // Rows: Time steps t (0..2PI)
    // Cols: cos(1t), sin(1t), cos(2t), sin(2t)...
    
    for (int k = 0; k < TSFI_FOURIER_SAMPLES; k++) {
        float t = (2.0f * ((float)TSFI_SECRET_CORE) * k) / TSFI_FOURIER_SAMPLES;
        
        for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
            int harmonic = n + 1;
            // Interleaved Cos/Sin for easier SIMD loading
            basis->data[k][n*2 + 0] = cosf(harmonic * t);
            basis->data[k][n*2 + 1] = sinf(harmonic * t);
        }
    }
}

// Convert Coordinate Set to Fourier Coeffs
// Based on Kuhl & Giardina (1982) logic simplified
// Input: xy is flat array of x,y,x,y...
void tsfi_fourier_from_points(TSFiFourierGlyph *out, const float *xy, size_t count) {
    if (!out || !xy || count < 3) return;
    memset(out, 0, sizeof(TSFiFourierGlyph));

    // DFT of the Points (Resampled)
    // Just sum the phasors for the voxels.
    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
        float an=0, bn=0, cn=0, dn=0;
        int h = n + 1;
        for (size_t i = 0; i < count; i++) {
            float t = (2.0f * ((float)TSFI_SECRET_CORE) * i) / count;
            float px = xy[i*2+0];
            float py = xy[i*2+1];
            
            an += px * cosf(h*t);
            bn += px * sinf(h*t);
            cn += py * cosf(h*t);
            dn += py * sinf(h*t);
        }
        // Normalize
        out->coeffs[n][0] = an * 2.0f / count;
        out->coeffs[n][1] = bn * 2.0f / count;
        out->coeffs[n][2] = cn * 2.0f / count;
        out->coeffs[n][3] = dn * 2.0f / count;
    }
}

// Reconstruct: V = M * C
// This is the "WMMA" step.
// output_voxels: float buffer [SAMPLES * 2] (x,y interleaved)
void tsfi_fourier_reconstruct_avx512(float *output_voxels, 
                                     const TSFiFourierBasis *basis, 
                                     const TSFiFourierGlyph *glyph) 
{
    // Use dense packed accumulators for the 512 samples
    float acc_x[TSFI_FOURIER_SAMPLES] = {0};
    float acc_y[TSFI_FOURIER_SAMPLES] = {0};

    // Transposed Synthesis (Wavefront Broadcasting)
    // For each harmonic n, we broadcast its Hilbert coefficients (DNA)
    // and apply it to the entire Banach coordinate manifold (512 samples).
    
    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n++) {
        __m512 v_an = _mm512_set1_ps(glyph->coeffs[n][0]);
        __m512 v_bn = _mm512_set1_ps(glyph->coeffs[n][1]);
        __m512 v_cn = _mm512_set1_ps(glyph->coeffs[n][2]);
        __m512 v_dn = _mm512_set1_ps(glyph->coeffs[n][3]);

        // Process 512 samples in 32 tight vectorized steps
        for (int k = 0; k < TSFI_FOURIER_SAMPLES; k += 16) {
            __m512i v_idx = _mm512_set_epi32(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
            v_idx = _mm512_mullo_epi32(v_idx, _mm512_set1_epi32(TSFI_FOURIER_HARMONICS * 2));
            
            __m512 v_cos = _mm512_i32gather_ps(v_idx, &basis->data[k][n*2+0], 4);
            __m512 v_sin = _mm512_i32gather_ps(v_idx, &basis->data[k][n*2+1], 4);

            // Load Wavefront Accumulators (Unaligned)
            __m512 v_ax = _mm512_loadu_ps(&acc_x[k]);
            __m512 v_ay = _mm512_loadu_ps(&acc_y[k]);

            // FMA: Synthesis step (Matrix Multiply)
            v_ax = _mm512_fmadd_ps(v_an, v_cos, v_ax);
            v_ax = _mm512_fmadd_ps(v_bn, v_sin, v_ax);
            
            v_ay = _mm512_fmadd_ps(v_cn, v_cos, v_ay);
            v_ay = _mm512_fmadd_ps(v_dn, v_sin, v_ay);

            // Store back to cache (Unaligned)
            _mm512_storeu_ps(&acc_x[k], v_ax);
            _mm512_storeu_ps(&acc_y[k], v_ay);
        }
    }

// Interleave back to output
    for(int i = 0; i < TSFI_FOURIER_SAMPLES; i++) {
        output_voxels[i*2+0] = acc_x[i];
        output_voxels[i*2+1] = acc_y[i];
    }
}

// Spectral Puppetry / Kinematics

void tsfi_fourier_translate(TSFiFourierGlyph *glyph, float x_offset, float y_offset) {
    if (!glyph) return;
    glyph->coeffs[0][0] += x_offset;
    glyph->coeffs[0][2] += y_offset;
}

void tsfi_fourier_articulate(TSFiFourierGlyph *glyph, float phase_shift, int start_harmonic, int end_harmonic) {
    if (!glyph) return;
    if (start_harmonic < 0) start_harmonic = 0;
    if (end_harmonic > TSFI_FOURIER_HARMONICS) end_harmonic = TSFI_FOURIER_HARMONICS;
    
    float cos_p = cosf(phase_shift);
    float sin_p = sinf(phase_shift);
    
    __m512 v_cos = _mm512_set1_ps(cos_p);
    __m512 v_sin = _mm512_set1_ps(sin_p);
    
    // Process 4 harmonics (16 floats) per cycle
    for (int n = start_harmonic; n <= end_harmonic - 4; n += 4) {
        // We need to swizzle to pair ax with bx, etc. 
        // A much faster approach for 4D complex vectors is to process X and Y coefficients separately if structured differently, 
        // but since they are interleaved [ax, bx, ay, by], we can use masked loads or shuffles.
        // For simplicity and correctness in this block, we'll do 4-wide parallel operations.
        
        // Let's load them individually to form vectors of A's and B's
        __m512 v_ax = _mm512_set_ps(0,0,0,0,0,0,0,0,0,0,0,0, glyph->coeffs[n+3][0], glyph->coeffs[n+2][0], glyph->coeffs[n+1][0], glyph->coeffs[n][0]);
        __m512 v_bx = _mm512_set_ps(0,0,0,0,0,0,0,0,0,0,0,0, glyph->coeffs[n+3][1], glyph->coeffs[n+2][1], glyph->coeffs[n+1][1], glyph->coeffs[n][1]);
        __m512 v_ay = _mm512_set_ps(0,0,0,0,0,0,0,0,0,0,0,0, glyph->coeffs[n+3][2], glyph->coeffs[n+2][2], glyph->coeffs[n+1][2], glyph->coeffs[n][2]);
        __m512 v_by = _mm512_set_ps(0,0,0,0,0,0,0,0,0,0,0,0, glyph->coeffs[n+3][3], glyph->coeffs[n+2][3], glyph->coeffs[n+1][3], glyph->coeffs[n][3]);
        
        __m512 v_ax_out = _mm512_sub_ps(_mm512_mul_ps(v_ax, v_cos), _mm512_mul_ps(v_bx, v_sin));
        __m512 v_bx_out = _mm512_add_ps(_mm512_mul_ps(v_ax, v_sin), _mm512_mul_ps(v_bx, v_cos));
        __m512 v_ay_out = _mm512_sub_ps(_mm512_mul_ps(v_ay, v_cos), _mm512_mul_ps(v_by, v_sin));
        __m512 v_by_out = _mm512_add_ps(_mm512_mul_ps(v_ay, v_sin), _mm512_mul_ps(v_by, v_cos));
        
        float ax_out[16], bx_out[16], ay_out[16], by_out[16];
        _mm512_storeu_ps(ax_out, v_ax_out);
        _mm512_storeu_ps(bx_out, v_bx_out);
        _mm512_storeu_ps(ay_out, v_ay_out);
        _mm512_storeu_ps(by_out, v_by_out);
        
        for (int i = 0; i < 4; i++) {
            glyph->coeffs[n+i][0] = ax_out[i];
            glyph->coeffs[n+i][1] = bx_out[i];
            glyph->coeffs[n+i][2] = ay_out[i];
            glyph->coeffs[n+i][3] = by_out[i];
        }
    }
    
    // Cleanup remaining harmonics
    for (int n = end_harmonic - (end_harmonic % 4); n < end_harmonic; n++) {
        float ax = glyph->coeffs[n][0];
        float bx = glyph->coeffs[n][1];
        float ay = glyph->coeffs[n][2];
        float by = glyph->coeffs[n][3];
        glyph->coeffs[n][0] = ax * cos_p - bx * sin_p;
        glyph->coeffs[n][1] = ax * sin_p + bx * cos_p;
        glyph->coeffs[n][2] = ay * cos_p - by * sin_p;
        glyph->coeffs[n][3] = ay * sin_p + by * cos_p;
    }
}

void tsfi_fourier_melt(TSFiFourierGlyph *glyph, float sickness_factor) {
    if (!glyph || sickness_factor <= 0.0f) return;
    
    // Low-pass filter: attenuate high frequencies.
    // We use a vectorized fast approximation: decay = max(0, 1.0 - sickness * (n / 50))
    // This physically prunes the high frequency detail out of the ZMM cache.
    
    __m512 v_sick = _mm512_set1_ps(sickness_factor / 50.0f);
    __m512 v_one = _mm512_set1_ps(1.0f);
    __m512 v_zero = _mm512_setzero_ps();
    
    // Start at harmonic 1 (DC offset is preserved)
    // Process 4 harmonics (16 floats) per cycle
    for (int n = 1; n <= TSFI_FOURIER_HARMONICS - 4; n += 4) {
        __m512 v_n = _mm512_set_ps(
            n+3, n+3, n+3, n+3,
            n+2, n+2, n+2, n+2,
            n+1, n+1, n+1, n+1,
            n, n, n, n
        );
        
        __m512 v_decay = _mm512_sub_ps(v_one, _mm512_mul_ps(v_n, v_sick));
        v_decay = _mm512_max_ps(v_decay, v_zero); // Clamp to 0
        
        __m512 v_coeffs = _mm512_loadu_ps(&glyph->coeffs[n][0]);
        __m512 v_out = _mm512_mul_ps(v_coeffs, v_decay);
        _mm512_storeu_ps(&glyph->coeffs[n][0], v_out);
    }
}

void tsfi_fourier_interpolate(TSFiFourierGlyph *out, const TSFiFourierGlyph *a, const TSFiFourierGlyph *b, float t) {
    if (!out || !a || !b) return;
    
    __m512 v_t = _mm512_set1_ps(t);
    __m512 v_inv_t = _mm512_set1_ps(1.0f - t);
    
    // Process 4 harmonics (16 floats) per cycle
    for (int n = 0; n < TSFI_FOURIER_HARMONICS; n += 4) {
        __m512 v_a = _mm512_loadu_ps(&a->coeffs[n][0]);
        __m512 v_b = _mm512_loadu_ps(&b->coeffs[n][0]);
        
        __m512 v_out = _mm512_fmadd_ps(v_b, v_t, _mm512_mul_ps(v_a, v_inv_t));
        _mm512_storeu_ps(&out->coeffs[n][0], v_out);
    }
}

#include "tsfi_nand_trap_firmware.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_io.h"

static uint64_t tsfi_fault_characterize(const uint8_t *intent_hash, uint64_t realization_id) {
    // Generate Heuristic Signature: XOR intent with realization
    uint64_t intent_prefix = *(uint64_t*)intent_hash;
    uint64_t signature = intent_prefix ^ realization_id;
    
    // Characterize the Energy Gap (Popcount of the XOR result indicates severity)
    return signature;
}

static void tsfi_form_memory_from_fracture(void *manifold_shm, uint64_t svdag_id, const uint8_t *target_hash, const TSFiHelmholtzSVDAG *dag) {
    // A fracture is a new memory forming. 
    // We anchor the objective realization into the Lore region (Leaf 502).
    uint8_t *leaf502 = (uint8_t *)manifold_shm + (502 * 256);

    // 1. Calculate Heuristic Signature
    uint64_t fault_sig = tsfi_fault_characterize(target_hash, svdag_id);

    // 2. Memory Schema Update: [8B ID][32B Hash][8B Sig][4B Energy][1B Active]
    memcpy(leaf502, &svdag_id, 8);
    memcpy(leaf502 + 8, target_hash, 32);
    memcpy(leaf502 + 40, &fault_sig, 8);
    
    // Initialize Energy (Starting at 1.0 units)
    float initial_energy = 1.0f;
    memcpy(leaf502 + 48, &initial_energy, 4);
    
    leaf502[52] = 1; // Mark as "Captured Fracture Memory"

    tsfi_io_printf(stdout, "[MEMORY] Fracture Re-Integrated. Signature: 0x%lx, Energy: %.2f\n", 
                   (unsigned long)fault_sig, initial_energy);
    
    // Formally commit the SAFE TASTE TREE TENSORS to the infinite journal
    tsfi_journal_anchor_fracture_memory(manifold_shm, svdag_id, dag);
}

void tsfi_grow_fault_energy(void *manifold_shm, float weight) {
    if (!manifold_shm) return;
    uint8_t *leaf502 = (uint8_t *)manifold_shm + (502 * 256);
    if (leaf502[52] != 1) return; // No memory to grow

    float current_energy;
    memcpy(&current_energy, leaf502 + 48, 4);
    
    // Weighted Energy Growth
    current_energy += (0.1f * weight);
    memcpy(leaf502 + 48, &current_energy, 4);

    if (current_energy > 10.0f) {
        tsfi_io_printf(stdout, "[AUTONOMY] Fracture Energy SATURATED (%.2f). Development Mandate Active.\n", current_energy);
    }
}

void tsfi_fourier_solidify_helmholtz(TSFiHelmholtzSVDAG *dag, 
                                     const TSFiFourierBasis *basis, 
                                     const TSFiFourierGlyph *spectrum,
                                     void *manifold_shm,
                                     const uint8_t *target_isomorphism_hash) 
{
    if (!dag || !basis || !spectrum || !manifold_shm) return;

    // 1. Reconstruct Banach Coordinate Manifold (512 Samples)
    float voxels[TSFI_FOURIER_SAMPLES * 2];
    tsfi_fourier_reconstruct_avx512(voxels, basis, spectrum);

    // 2. Compile into SVDAG (The Taste Tree)
    // We treat the 2D Fourier contour as a 3D extrusion guide
    NandTrapState guide = {0};
    guide.magic = NAND_TRAP_MAGIC;
    guide.version = NAND_TRAP_VERSION;
    for (int i = 0; i < GRANS; i++) {
        // Map Fourier samples to granules (may wrap or skip if SAMPLES != GRANS)
        int src_idx = i % TSFI_FOURIER_SAMPLES;
        guide.fibers[i].x = voxels[src_idx*2 + 0];
        guide.fibers[i].y = voxels[src_idx*2 + 1];
        guide.fibers[i].z = (float)i / (float)GRANS;
        guide.fibers[i].weight = 1.0f;
    }
    tsfi_svdag_compile_nand(dag, &guide);

    // 3. XNOR Trap Verification (Isomorphism Proof)
    // We use the XNOR Trap to compare the generated SVDAG signature 
    // against the mandated 'target_isomorphism_hash' (e.g. The Cow).

    // Establishing a temporary wavelet for the hardware handshake
    TsfiWavelet W_Genie = {0}, W_SVDAG = {0};
    memcpy(W_Genie.payload, target_isomorphism_hash, 32); // Subjective target
    // SVDAG execution integrates the integral of Feynman points into a rigid ID
    uint64_t svdag_id = (uint64_t)tsfi_svdag_execute(dag);
    memcpy(W_SVDAG.payload, &svdag_id, 8); // Objective realization

    tsfi_wavelet_xnor_signal(manifold_shm, &W_Genie, &W_SVDAG);

    // 4. Secure Anchor
    uint8_t *leaf504 = (uint8_t *)manifold_shm + (504 * 256);
    if (leaf504[0] == 1) {
        tsfi_io_printf(stdout, "[FOURIER] Isomorphism Solidified. Helmholtz Standard Model Wave System Active.\n");
        dag->is_logical = true;
    } else {
        tsfi_io_printf(stderr, "[FRACTURE] Fourier Isomorphism Mismatch. Capturing New Memory...\n");
        tsfi_form_memory_from_fracture(manifold_shm, svdag_id, target_isomorphism_hash, dag);
        dag->is_logical = false;
    }
}
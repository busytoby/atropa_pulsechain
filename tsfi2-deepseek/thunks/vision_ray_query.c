#include "tsfi_wave512.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_vision.h"
#include "tsfi_c_math.h"
#include <immintrin.h>
#include <stdalign.h>

/**
 * TSFi Vision Ray-Query Thunk (CPU-Side AVX-512)
 * 
 * Performs 2.5D ray-marching through the density genome to gather 
 * topological information (Occlusion, Self-Shadowing).
 * 
 * This thunk replaces the SPIR-V ray_query logic with a C-based 
 * symmetric alternative that runs on the CPU's vector bank.
 */

// Simple density evaluator used by the ray-marcher (matching main logic)
static inline __m512 eval_density_thunk(__m512 px, __m512 py, const K0RnOp *ops, int count) {
    __m512 d = _mm512_set1_ps(1e5f);
    __m512 zero = _mm512_setzero_ps();
    for (int i = 0; i < count; i++) {
        __m512 vx = _mm512_sub_ps(px, _mm512_set1_ps(ops[i].x));
        __m512 vy = _mm512_sub_ps(py, _mm512_set1_ps(ops[i].y));
        __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(vx, vx), _mm512_mul_ps(vy, vy)));
        __m512 rad = _mm512_set1_ps(ops[i].p1);
        __m512 cur_d = _mm512_sub_ps(dist, rad);
        if (ops[i].type == K0RN_OP_SUB) d = _mm512_max_ps(d, _mm512_sub_ps(zero, cur_d));
        else d = _mm512_min_ps(d, cur_d);
    }
    return d;
}

__attribute__((force_align_arg_pointer))
void tsfi_vision_ray_query_thunk(const K0RnOp *ops, int op_count, TSFiResonanceAnalysis *out, int width, int height) {
    if (!ops || !out) return;
    (void)width; (void)height;

    __m512 v_occ_sum = _mm512_setzero_ps();
    __m512 v_step_size = _mm512_set1_ps(0.01f);
    __m512 v_one = _mm512_set1_ps(1.0f);

    // Sample 16 radial rays from the center to detect occlusion/density
    // Angles are 0, 22.5, 45, ... 337.5 degrees
    __m512 v_angles = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    v_angles = _mm512_mul_ps(v_angles, _mm512_set1_ps(0.392699f)); // step = 2*pi/16

    __m512 v_cos, v_sin;
    // We use a simplified sin/cos or just assume radial directions
    v_cos = _mm512_set_ps(1.000, 0.923, 0.707, 0.382, 0.000, -0.382, -0.707, -0.923, -1.000, -0.923, -0.707, -0.382, 0.000, 0.382, 0.707, 0.923);
    v_sin = _mm512_set_ps(0.000, 0.382, 0.707, 0.923, 1.000, 0.923, 0.707, 0.382, 0.000, -0.382, -0.707, -0.923, -1.000, -0.923, -0.707, -0.382);

    for (int step = 1; step <= 10; step++) {
        __m512 v_dist = _mm512_mul_ps(_mm512_set1_ps((float)step), v_step_size);
        __m512 vx = _mm512_mul_ps(v_dist, v_cos);
        __m512 vy = _mm512_mul_ps(v_dist, v_sin);
        
        __m512 d = eval_density_thunk(vx, vy, ops, op_count);
        // If d < 0, we are inside geometry (occluded)
        __mmask16 inside = _mm512_cmp_ps_mask(d, _mm512_setzero_ps(), _CMP_LT_OS);
        v_occ_sum = _mm512_mask_add_ps(v_occ_sum, inside, v_occ_sum, v_one);
    }

    float total_occ = _mm512_reduce_add_ps(v_occ_sum);
    out->specular_contrast = total_occ / 160.0f; 
    
    // Shadow Softness: Variance of occlusion across the rays
    float b_avg = total_occ / 16.0f;
    alignas(64) float occ_vals[16];
    _mm512_store_ps(occ_vals, v_occ_sum);
    float soft_var = 0;
    for(int i=0; i<16; i++) soft_var += (occ_vals[i] - b_avg)*(occ_vals[i] - b_avg);
    out->surface_grain = tsfi_sqrtf(soft_var) / 10.0f; // High variance = sharp shadows, Low = soft wrap
    
    out->progression_ratio = (out->progression_ratio + out->specular_contrast) * 0.5f;
}

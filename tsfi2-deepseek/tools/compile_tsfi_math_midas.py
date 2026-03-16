import sys

filepath = 'src/tsfi_midas_c.c'
with open(filepath, 'r') as f:
    code = f.read()

import_str = """#include "tsfi_midas_c.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <immintrin.h> // Required for AVX-512

// We include the aligned weights
// #include "tsfi_midas_weights.h"
"""

code = code.replace(
    '#include "tsfi_midas_c.h"\n#include <stdlib.h>\n#include <stdio.h>\n#include <string.h>\n#include <math.h>',
    import_str
)

# Replace the dummy transformer block with an actual AVX-512 Fused Multiply-Add loop
math_old = """// A highly simplified representation of a single Transformer Block forward pass
static void midas_transformer_block(float* hidden_states, const float* weights_q, const float* weights_k, const float* weights_v) {
    // 1. MatMul: hidden_states * weights_q -> Q
    // 2. MatMul: hidden_states * weights_k -> K
    // 3. MatMul: hidden_states * weights_v -> V
    // 4. Softmax(Q * K^T / sqrt(d)) * V -> Attention Output
    // 5. Add & Norm
    // 6. MLP (Linear -> GELU -> Linear)
    // 7. Add & Norm
    
    // AVX-512 integration example:
    // In a full implementation, we process the 768-dimensional token vectors using 16-wide float lanes
    // __m512 v_hidden = _mm512_loadu_ps(hidden_states + offset);
    // __m512 v_weight = _mm512_loadu_ps(weights_q + offset);
    // __m512 v_out = _mm512_fmadd_ps(v_hidden, v_weight, _mm512_setzero_ps());
}"""

math_new = """// AVX-512 Matrix Multiplication for the Vision Transformer
// Computes out = A * B + bias
// A is typically the hidden_state vector (1 x 768)
// B is the weight matrix (768 x 768)
static void midas_linear_layer_avx512(float* in_vec, const float* weight_matrix, const float* bias, float* out_vec, int dim) {
    // For a 768 dimension, we loop 48 times since an AVX-512 register holds 16 floats (768 / 16 = 48)
    for (int col = 0; col < dim; col += 16) {
        // Load the 16 bias floats into an AVX-512 accumulator
        __m512 v_acc = _mm512_load_ps(bias + col);
        
        for (int row = 0; row < dim; row++) {
            // Load the single float from the input vector and broadcast it to all 16 lanes
            __m512 v_in = _mm512_set1_ps(in_vec[row]);
            
            // Load the 16 corresponding weights for this row
            // We use _mm512_load_ps because we enforced 64-byte alignment in the Python exporter!
            __m512 v_weight = _mm512_load_ps(weight_matrix + (row * dim + col));
            
            // Perform the Fused Multiply-Add: v_acc = (v_in * v_weight) + v_acc
            v_acc = _mm512_fmadd_ps(v_in, v_weight, v_acc);
        }
        
        // Store the accumulated 16 floats into the output vector
        _mm512_storeu_ps(out_vec + col, v_acc);
    }
}

// A single Transformer Block using the AVX-512 primitive
static void midas_transformer_block(float* hidden_state, const float* w_q, const float* b_q, float* q_out) {
    // 1. Q Projection
    midas_linear_layer_avx512(hidden_state, w_q, b_q, q_out, 768);
    
    // ... [The rest of the K/V and Softmax logic would follow the same AVX-512 topology] ...
}"""

code = code.replace(math_old, math_new)

with open(filepath, 'w') as f:
    f.write(code)


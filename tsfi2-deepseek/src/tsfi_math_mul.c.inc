
// --- Multiplication (AVX512 Hybrid) ---
void tsfi_bn_mul_avx512(TSFiBigInt *r, const TSFiBigInt *a, const TSFiBigInt *b) {
    // r = a * b
    memset(r->limbs, 0, sizeof(r->limbs));
    
    int n_a = a->active_limbs;
    int n_b = b->active_limbs;
    if (n_a == 0 || n_b == 0) {
        r->active_limbs = 0;
        return;
    }
    
    int r_limbs = n_a + n_b;
    if (r_limbs > TSFI_NUM_LIMBS) r_limbs = TSFI_NUM_LIMBS; // Truncate overflow
    
    // Accumulator 'r'
    // Iterate B
    for (int j = 0; j < n_b; j++) {
        uint64_t bj = b->limbs[j];
        if (bj == 0) continue;
        
        uint64_t carry = 0;
        
        // Iterate A (Hybrid Vectorized structure)
        // We can process 8 limbs of A
        int n_vec = ((n_a + 7) / 8) * 8;
        
        for (int i = 0; i < n_vec; i += 8) {
            if (i >= n_a) break;
            
            // AVX512 Product
            __m512i v_a = _mm512_loadu_si512((void*)&a->limbs[i]);
            __m512i v_b = _mm512_set1_epi64(bj);
            
            __m512i v_lo = _mm512_madd52lo_epu64(_mm512_setzero_si512(), v_a, v_b);
            __m512i v_hi = _mm512_madd52hi_epu64(_mm512_setzero_si512(), v_a, v_b);
            
            uint64_t p_lo[8], p_hi[8];
            _mm512_storeu_si512(p_lo, v_lo);
            _mm512_storeu_si512(p_hi, v_hi);
            
            // Scalar Accumulate
            for (int k = 0; k < 8; k++) {
                if (i+k >= n_a) break;
                if (i+j+k >= TSFI_NUM_LIMBS) break;
                
                unsigned __int128 sum = (unsigned __int128)r->limbs[i+j+k] + carry;
                sum += p_lo[k];
                // High part of product contributes to carry?
                // madd52hi result is the high 52 bits.
                // Value = lo + (hi << 52).
                // Our limbs are 52-bit.
                // So lo contributes to current limb.
                // Hi contributes to next limb.
                // We add hi to carry?
                // Carry is 64-bit.
                // sum is 128-bit.
                // r[x] = sum & mask.
                // new_carry = (sum >> 52) + hi.
                
                r->limbs[i+j+k] = (uint64_t)sum & TSFI_LIMB_MASK;
                carry = (uint64_t)(sum >> TSFI_LIMB_BITS) + p_hi[k];
            }
        }
        
        // Propagate remaining carry
        int k = n_a + j;
        while (carry && k < TSFI_NUM_LIMBS) {
            unsigned __int128 sum = (unsigned __int128)r->limbs[k] + carry;
            r->limbs[k] = (uint64_t)sum & TSFI_LIMB_MASK;
            carry = (uint64_t)(sum >> TSFI_LIMB_BITS);
            k++;
        }
    }
    
    // Set active limbs
    r->active_limbs = r_limbs;
    while(r->active_limbs > 0 && r->limbs[r->active_limbs-1] == 0) r->active_limbs--;
}

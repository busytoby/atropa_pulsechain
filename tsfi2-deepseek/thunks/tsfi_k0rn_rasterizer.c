#include "tsfi_k0rn_ops.h"
#include "tsfi_font_vectors.h"
#include <immintrin.h>
#include <math.h>
#include <string.h>
#include <stdalign.h>

/**
 * TSFi K0Rn Font Rasterizer (AVX-512)
 * 
 * Renders a K0Rn-defined glyph at a specific point size.
 */

void tsfi_font_render_k0rn_pt(const K0RnStream *stream, uint32_t *pixels, int width, int height, float pt_size, float x, float y, uint32_t color) {
    if (!stream || !pixels || pt_size <= 0) return;

    // Standard Scale: 96 DPI, 1pt = 1.33px
    float pixel_height = pt_size * (96.0f / 72.0f);
    // K0Rn range is [-1, 1], so scale is pixel_height / 2
    float scale = pixel_height * 0.5f;

    __m512 v_scale = _mm512_set1_ps(scale);
    __m512 v_off_x = _mm512_set1_ps(x);
    __m512 v_off_y = _mm512_set1_ps(y);
    __m512 v_zero = _mm512_setzero_ps();
    __m512 v_one = _mm512_set1_ps(1.0f);
    __m512 v_x_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

    // Bounding box in pixels
    int min_px = (int)(x - scale * 1.5f); // Padding for slanted/wide glyphs
    int max_px = (int)(x + scale * 1.5f);
    int min_py = (int)(y - scale * 1.5f);
    int max_py = (int)(y + scale * 1.5f);

    if (min_px < 0) min_px = 0; 
    if (max_px >= width) max_px = width - 1;
    if (min_py < 0) min_py = 0; 
    if (max_py >= height) max_py = height - 1;

    for (int py = min_py; py <= max_py; py++) {
        __m512 vy = _mm512_set1_ps((float)py);
        // Invert Y: K0Rn is Y-up, Screen is Y-down
        __m512 v_ky = _mm512_div_ps(_mm512_sub_ps(v_off_y, vy), v_scale);

        for (int px = (min_px / 16) * 16; px <= max_px; px += 16) {
            if (px < 0) continue;
            __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)px), v_x_ramp);
            __m512 v_kx = _mm512_div_ps(_mm512_sub_ps(vx, v_off_x), v_scale);

            __m512 d = _mm512_set1_ps(1e5f);

            for (uint32_t i = 0; i < stream->op_count; i++) {
                K0RnOp *op = &stream->ops[i];
                __m512 pax = _mm512_set1_ps(op->x), pay = _mm512_set1_ps(op->y);
                __m512 pbx = _mm512_set1_ps(op->z), pby = _mm512_set1_ps(op->w);
                __m512 rad = _mm512_set1_ps(op->p1);

                __m512 ba_x = _mm512_sub_ps(pbx, pax); __m512 ba_y = _mm512_sub_ps(pby, pay);
                __m512 pa_dx = _mm512_sub_ps(v_kx, pax); __m512 pa_dy = _mm512_sub_ps(v_ky, pay);
                
                __m512 h = _mm512_div_ps(_mm512_add_ps(_mm512_mul_ps(pa_dx, ba_x), _mm512_mul_ps(pa_dy, ba_y)), 
                                         _mm512_max_ps(_mm512_add_ps(_mm512_mul_ps(ba_x, ba_x), _mm512_mul_ps(ba_y, ba_y)), _mm512_set1_ps(1e-6f)));
                h = _mm512_min_ps(v_one, _mm512_max_ps(v_zero, h));
                
                __m512 dx = _mm512_sub_ps(pa_dx, _mm512_mul_ps(ba_x, h)); 
                __m512 dy = _mm512_sub_ps(pa_dy, _mm512_mul_ps(ba_y, h));
                __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)));
                
                d = _mm512_min_ps(d, _mm512_sub_ps(dist, rad));
            }

            // Alpha: antialiased edge (smoothness = 0.05 in K0Rn units)
            __m512 smoothness = _mm512_set1_ps(0.05f);
            __m512 alpha = _mm512_div_ps(_mm512_sub_ps(v_zero, d), smoothness);
            alpha = _mm512_max_ps(v_zero, _mm512_min_ps(v_one, alpha));

            // Write to pixels (masking for efficiency)
            __mmask16 write_mask = _mm512_cmp_ps_mask(alpha, _mm512_set1_ps(0.01f), _CMP_GT_OS);
            if (write_mask == 0) continue;

            // Simplified blending: assuming black background for demo, or we could do proper lerp
            // pixels[y*width+x] = color * alpha
            __m512i v_color = _mm512_set1_epi32(color);
            
            // Extract R, G, B
            __m512 v_r = _mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_color, 16), _mm512_set1_epi32(0xFF)));
            __m512 v_g = _mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_color, 8), _mm512_set1_epi32(0xFF)));
            __m512 v_b = _mm512_cvtepi32_ps(_mm512_and_si512(v_color, _mm512_set1_epi32(0xFF)));

            v_r = _mm512_mul_ps(v_r, alpha);
            v_g = _mm512_mul_ps(v_g, alpha);
            v_b = _mm512_mul_ps(v_b, alpha);

            __m512i ir = _mm512_cvtps_epi32(v_r);
            __m512i ig = _mm512_cvtps_epi32(v_g);
            __m512i ib = _mm512_cvtps_epi32(v_b);

            __m512i v_final = _mm512_or_si512(_mm512_set1_epi32(0xFF000000), 
                               _mm512_or_si512(_mm512_slli_epi32(ir, 16), 
                                _mm512_or_si512(_mm512_slli_epi32(ig, 8), ib)));

            _mm512_mask_storeu_epi32(&pixels[py * width + px], write_mask, v_final);
        }
    }
}

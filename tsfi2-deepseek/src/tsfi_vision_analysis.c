#include "tsfi_vision_internal.h"

void tsfi_vision_analyze_glyph(const uint32_t *data, int width, int height, const TSFiGlyphPhenotype *p, TSFiResonanceAnalysis *out) {
    if (!out) return;
    memset(out, 0, sizeof(TSFiResonanceAnalysis));
    int total_pixels = width * height;
    if (!data || !p || width < 64 || height < 32) return;

    // --- Topological Hole Detection ---
    // 0: Exterior, 1: Glyph, 2: Interior (Hole)
    uint8_t *topo = (uint8_t*)lau_malloc(width * height);
    for (int i = 0; i < total_pixels; i++) {
        uint32_t px = data[i];
        float lum = (((float)((px >> 16) & 0xFF)) + ((float)((px >> 8) & 0xFF)) + ((float)(px & 0xFF))) / 765.0f;
        topo[i] = (lum > 0.15f) ? 1 : 0; // 0.15 merges smooth segments
    }

    // Stack-based flood fill from edges
    int *stack = (int*)lau_malloc(width * height * sizeof(int));
    int top = 0;
    for (int x = 0; x < width; x++) {
        if (topo[x] == 0) { topo[x] = 3; stack[top++] = x; }
        if (topo[(height-1)*width + x] == 0) { topo[(height-1)*width + x] = 3; stack[top++] = (height-1)*width + x; }
    }
    for (int y = 0; y < height; y++) {
        if (topo[y*width] == 0) { topo[y*width] = 3; stack[top++] = y*width; }
        if (topo[y*width + width-1] == 0) { topo[y*width + width-1] = 3; stack[top++] = y*width + width-1; }
    }

    while (top > 0) {
        int idx = stack[--top];
        int tx = idx % width, ty = idx / width;
        int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
        for (int i = 0; i < 4; i++) {
            int nx = tx + dx[i], ny = ty + dy[i];
            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int nidx = ny * width + nx;
                if (topo[nidx] == 0) { topo[nidx] = 3; stack[top++] = nidx; }
            }
        }
    }

    // Count remaining '0' components (holes) and '1' components (islands)
    int holes = 0;
    int islands = 0;
    
    // First, count islands (Glyph pixels '1')
    // We reuse the stack logic. Need to reset stack/visited for this pass or do it separate?
    // The previous loop only touched '0' (background). '1's are untouched.
    for (int i = 0; i < total_pixels; i++) {
        if (topo[i] == 1) { // Found a new island
            islands++;
            topo[i] = 4; // Mark as visited island
            stack[top++] = i;
            while (top > 0) {
                int idx = stack[--top];
                int tx = idx % width, ty = idx / width;
                int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
                for (int j = 0; j < 4; j++) {
                    int nx = tx + dx[j], ny = ty + dy[j];
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int nidx = ny * width + nx;
                        if (topo[nidx] == 1) { topo[nidx] = 4; stack[top++] = nidx; }
                    }
                }
            }
        }
    }
    out->island_count = (float)islands;

    // Now count holes (0)
    for (int i = 0; i < total_pixels; i++) {
        if (topo[i] == 0) {
            holes++;
            // Flood fill this hole to mark it seen
            topo[i] = 2; stack[top++] = i;
            while (top > 0) {
                int idx = stack[--top];
                int tx = idx % width, ty = idx / width;
                int dx[] = {1, -1, 0, 0}, dy[] = {0, 0, 1, -1};
                for (int j = 0; j < 4; j++) {
                    int nx = tx + dx[j], ny = ty + dy[j];
                    if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                        int nidx = ny * width + nx;
                        if (topo[nidx] == 0) { topo[nidx] = 2; stack[top++] = nidx; }
                    }
                }
            }
        }
    }
    out->topo_hole_count = (float)holes;
    lau_free(topo); lau_free(stack);

    // --- Feature Detection (Direct Box Probes) ---
    // Helper to sum region intensity
    float get_box_avg(int bx, int by, int bw, int bh) {
        if (bw <= 0 || bh <= 0) return 0.0f;
        int x1 = bx < 0 ? 0 : bx, x2 = bx + bw; if (x2 > width) x2 = width;
        int y1 = by < 0 ? 0 : by, y2 = by + bh; if (y2 > height) y2 = height;
        if (x1 >= x2 || y1 >= y2) return 0.0f;
        
        double sum = 0; int cnt = 0;
        for (int y = y1; y < y2; y += 4) { // Sparsify for speed (1/16th pixels)
            for (int x = x1; x < x2; x += 4) {
                uint32_t px = data[y * width + x];
                sum += (float)((px & 0xFF) + ((px >> 8) & 0xFF) + ((px >> 16) & 0xFF)) / 765.0f;
                cnt++;
            }
        }
        return cnt > 0 ? (float)(sum / cnt) : 0.0f;
    }

    // Normalized Coords
    int cx = width / 2, cy = height / 2;
    int scale = (int)((float)height * 0.4f); // Approx glyph size (Safe fit)

    // 1. Outer Circle (Ring at r=0.8)
    // Check for mass at the expected ring perimeter (Left, Right, Top, Bottom)
    // Shift inward to 0.75 to catch the ring meat
    float r_chk = scale * 0.75f;
    float ring_l = get_box_avg(cx - r_chk, cy - scale*0.15f, scale*0.3f, scale*0.3f);
    float ring_r = get_box_avg(cx + r_chk - scale*0.3f, cy - scale*0.15f, scale*0.3f, scale*0.3f);
    float ring_t = get_box_avg(cx - scale*0.15f, cy - r_chk, scale*0.3f, scale*0.3f);
    float ring_b = get_box_avg(cx - scale*0.15f, cy + r_chk - scale*0.3f, scale*0.3f, scale*0.3f);
    
    // Require at least 3 quadrants to be present for "Outer Circle" (Threshold 0.15)
    int q_count = (ring_l > 0.15f) + (ring_r > 0.15f) + (ring_t > 0.15f) + (ring_b > 0.15f);
    out->feature_outer_circle = (q_count >= 3 && out->glyph_symmetry > 0.8f) ? 1.0f : 0.0f;

    // 2. Inner Circle
    // ... (unchanged logic) ...

    // 3. Vertical Stem (x=0, y=Lower)
    // Box: Thin Vertical in Lower Half.
    float i_stem = get_box_avg(cx - scale*0.05f, cy + scale*0.1f, scale*0.1f, scale*0.4f);
    // Flanks
    float i_stem_l = get_box_avg(cx - scale*0.25f, cy + scale*0.1f, scale*0.1f, scale*0.4f);
    float i_stem_r = get_box_avg(cx + scale*0.15f, cy + scale*0.1f, scale*0.1f, scale*0.4f);
    out->feature_stem = (i_stem > 0.3f && i_stem_l < 0.3f && i_stem_r < 0.3f) ? 1.0f : 0.0f;

    // 4. Horizontal Bar (y=Middle/Lower)
    // Bar is width 0.5. Probe was 0.6. Reduce width to 0.4 to ensure high density.
    // Bar shifted to y=-0.25. Probe must match.
    float i_bar = get_box_avg(cx - scale*0.2f, cy + scale*0.25f, scale*0.4f, scale*0.1f);
    out->feature_bar = (i_bar > 0.4f) ? 1.0f : 0.0f;

    // Inner Circle Logic update:
    // If we have a hole in the upper center?
    float i_upper_hole = get_box_avg(cx - scale*0.1f, cy - scale*0.3f, scale*0.2f, scale*0.2f);
    out->feature_inner_circle = (i_upper_hole < 0.2f) ? 1.0f : 0.0f; // Simplified "Hole Detection"

    // --- First Pass: Center of Mass ---
    __m512 v_mass_sum = _mm512_setzero_ps();
    __m512 v_sum_x = _mm512_setzero_ps(), v_sum_y = _mm512_setzero_ps();
    __m512 v_inv255 = _mm512_set1_ps(1.0f / 255.0f);
    __m512 v_x_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

    for (int y = 0; y < height; y++) {
        __m512 v_y = _mm512_set1_ps((float)y);
        for (int x = 0; x < width; x += 16) {
            __m512i v_px = safe_load_si512(data, y * width + x, total_pixels);
            __m512 v_lum = get_lum_avx512(v_px, v_inv255);
            v_mass_sum = _mm512_add_ps(v_mass_sum, v_lum);
            v_sum_x = _mm512_add_ps(v_sum_x, _mm512_mul_ps(_mm512_add_ps(_mm512_set1_ps((float)x), v_x_ramp), v_lum));
            v_sum_y = _mm512_add_ps(v_sum_y, _mm512_mul_ps(v_y, v_lum));
        }
    }

    float t_l = _mm512_reduce_add_ps(v_mass_sum);
    if (t_l < 1.0f) { out->center_mass_x = 0.5f; out->center_mass_y = 0.5f; return; }

    out->center_mass_x = _mm512_reduce_add_ps(v_sum_x) / t_l;
    out->center_mass_y = _mm512_reduce_add_ps(v_sum_y) / t_l;
    out->coverage = t_l / (float)total_pixels;

    // --- Second Pass: Centered Axial Symmetry ---
    int mid_x = (int)out->center_mass_x;
    __m512 v_sym_diff = _mm512_setzero_ps();
    int range = (mid_x < width - mid_x) ? mid_x : (width - 1 - mid_x);
    range = (range / 16) * 16;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < range; x += 16) {
            int lx = mid_x - 1 - x - 15;
            int rx = mid_x + 1 + x;
            if (lx < 0 || rx + 15 >= width) continue;

            __m512i v_px_l = safe_load_si512(data, y * width + lx, total_pixels);
            __m512i v_px_l_rev = _mm512_permutexvar_epi32(_mm512_set_epi32(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15), v_px_l);
            __m512 v_lum_l = get_lum_avx512(v_px_l_rev, v_inv255);

            __m512i v_px_r = safe_load_si512(data, y * width + rx, total_pixels);
            __m512 v_lum_r = get_lum_avx512(v_px_r, v_inv255);

            __m512 v_diff = _mm512_sub_ps(v_lum_l, v_lum_r);
            v_sym_diff = _mm512_add_ps(v_sym_diff, _mm512_mul_ps(v_diff, v_diff));
        }
    }

    out->glyph_symmetry = 1.0f - tsfi_sqrtf(_mm512_reduce_add_ps(v_sym_diff) / (t_l + 1e-6f));
    if (out->glyph_symmetry < 0) out->glyph_symmetry = 0;
    out->complexity = tsfi_fminf(1.0f, out->coverage * 5.0f);
    out->smoothness = out->glyph_symmetry;

    out->target_correlation = (out->glyph_symmetry * 0.6f) + (tsfi_fminf(1.0f, out->coverage / p->target_coverage) * 0.4f);
    
    // Feature Requirements Enforcement
    float feature_match = 1.0f;
    if (p->req_stem > 0) feature_match *= (out->feature_stem > 0.5f ? 1.0f : 0.2f);
    if (p->req_bar > 0) feature_match *= (out->feature_bar > 0.5f ? 1.0f : 0.2f);
    if (p->req_inner_circle > 0) feature_match *= (out->feature_inner_circle > 0.5f ? 1.0f : 0.2f);
    if (p->req_outer_circle > 0) feature_match *= (out->feature_outer_circle > 0.5f ? 1.0f : 0.2f);
    out->target_correlation *= feature_match;

    out->progression_ratio = out->target_correlation;

    // --- Third Pass: Horizontal Symmetry & Quadrants ---
    __m512 v_mass_top = _mm512_setzero_ps(), v_mass_bot = _mm512_setzero_ps();
    __m512 v_h_sym_diff = _mm512_setzero_ps();
    int mid_y = height / 2;

    for (int y = 0; y < mid_y; y++) {
        int ry = height - 1 - y;
        for (int x = 0; x < width; x += 16) {
            __m512 v_l_t = get_lum_avx512(safe_load_si512(data, y * width + x, total_pixels), v_inv255);
            __m512 v_l_b = get_lum_avx512(safe_load_si512(data, ry * width + x, total_pixels), v_inv255);
            
            v_mass_top = _mm512_add_ps(v_mass_top, v_l_t);
            v_mass_bot = _mm512_add_ps(v_mass_bot, v_l_b);
            
            __m512 v_diff = _mm512_sub_ps(v_l_t, v_l_b);
            v_h_sym_diff = _mm512_add_ps(v_h_sym_diff, _mm512_mul_ps(v_diff, v_diff));
        }
    }

    float t_top = _mm512_reduce_add_ps(v_mass_top);
    float t_bot = _mm512_reduce_add_ps(v_mass_bot);
    out->specular_contrast = t_top / (t_bot + 1e-6f); // Re-purpose specular_contrast as Top/Bottom ratio
    out->lobe_symmetry = 1.0f - sqrtf(_mm512_reduce_add_ps(v_h_sym_diff) / (t_l + 1e-6f)); // Horizontal Sym
}

void tsfi_vision_analyze_staging(const uint32_t *data, int width, int height, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out) {
    if (!out) return;
    memset(out, 0, sizeof(TSFiResonanceAnalysis));
    int total_pixels = width * height;
    if (!data || !p || width < 128 || height < 32) return;

    __m512 v_mass_r = _mm512_setzero_ps(), v_mass_g = _mm512_setzero_ps(), v_mass_b = _mm512_setzero_ps();
    __m512 v_sum_x = _mm512_setzero_ps(), v_sum_y = _mm512_setzero_ps();
    __m512 v_inv255 = _mm512_set1_ps(1.0f / 255.0f);
    __m512 v_x_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);

    for (int y = 8; y < height - 8; y++) {
        __m512 v_y = _mm512_set1_ps((float)y);
        for (int x = 64; x < width - 64; x += 16) {
            __m512i v_px = safe_load_si512(data, y * width + x, total_pixels);
            __m512 v_r = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_px, 16), _mm512_set1_epi32(0xFF))), v_inv255);
            __m512 v_g = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_px, 8), _mm512_set1_epi32(0xFF))), v_inv255);
            __m512 v_b = _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(v_px, _mm512_set1_epi32(0xFF))), v_inv255);
            __m512 v_lum = _mm512_mul_ps(_mm512_add_ps(v_r, _mm512_add_ps(v_g, v_b)), _mm512_set1_ps(0.333f));
            v_mass_r = _mm512_add_ps(v_mass_r, v_r); v_mass_g = _mm512_add_ps(v_mass_g, v_g); v_mass_b = _mm512_add_ps(v_mass_b, v_b);
            v_sum_x = _mm512_add_ps(v_sum_x, _mm512_mul_ps(_mm512_add_ps(_mm512_set1_ps((float)x), v_x_ramp), v_lum));
            v_sum_y = _mm512_add_ps(v_sum_y, _mm512_mul_ps(v_y, v_lum));
        }
    }

    float t_r = _mm512_reduce_add_ps(v_mass_r), t_g = _mm512_reduce_add_ps(v_mass_g), t_b = _mm512_reduce_add_ps(v_mass_b);
    float t_l = (t_r + t_g + t_b) * 0.333f;
    if (t_l < 1.0f) { out->center_mass_x = 0.5f; out->center_mass_y = 0.5f; return; }

    out->center_mass_x = _mm512_reduce_add_ps(v_sum_x) / t_l; out->center_mass_y = _mm512_reduce_add_ps(v_sum_y) / t_l;
    out->coverage = t_l / (float)total_pixels;
    out->avg_intensity = t_l / (out->coverage * total_pixels + 1e-6f);

    float b_r[8]={0}, b_g[8]={0}, b_b[8]={0}, b_l[8]={0};
    int counts[8]={0};
    float max_r = (float)width * 0.45f;
    float d_sum = 0; int d_cnt = 0;

    for (int y = 8; y < height - 8; y++) {
        for (int x = 64; x < width - 64; x += 16) {
            __m512i v_px = safe_load_si512(data, y * width + x, total_pixels);
            __m512 v_val = get_lum_avx512(v_px, v_inv255);
            __m512 v_u = get_lum_avx512(safe_load_si512(data, (y-1)*width+x, total_pixels), v_inv255);
            __m512 v_d = get_lum_avx512(safe_load_si512(data, (y+1)*width+x, total_pixels), v_inv255);
            __m512 v_l_p = get_lum_avx512(safe_load_si512(data, y*width+x-1, total_pixels), v_inv255);
            __m512 v_r_p = get_lum_avx512(safe_load_si512(data, y*width+x+1, total_pixels), v_inv255);
            __m512 v_lap = _mm512_abs_ps(_mm512_sub_ps(_mm512_mul_ps(v_val, _mm512_set1_ps(4.0f)), _mm512_add_ps(_mm512_add_ps(v_u, v_d), _mm512_add_ps(v_l_p, v_r_p))));
            d_sum += _mm512_reduce_add_ps(_mm512_div_ps(v_lap, _mm512_add_ps(v_val, _mm512_set1_ps(0.1f)))); d_cnt += 16;

            __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)x), v_x_ramp);
            __m512 dx = _mm512_sub_ps(vx, _mm512_set1_ps(out->center_mass_x)), dy = _mm512_sub_ps(_mm512_set1_ps((float)y), _mm512_set1_ps(out->center_mass_y));
            __m512 v_dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(dx, dx), _mm512_mul_ps(dy, dy)));
            
            alignas(64) float ds[16], rs[16], gs[16], bs[16], ls[16];
            _mm512_store_ps(ds, v_dist); _mm512_store_ps(ls, v_val);
            _mm512_store_ps(rs, _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_px, 16), _mm512_set1_epi32(0xFF))), v_inv255));
            _mm512_store_ps(gs, _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(_mm512_srli_epi32(v_px, 8), _mm512_set1_epi32(0xFF))), v_inv255));
            _mm512_store_ps(bs, _mm512_mul_ps(_mm512_cvtepi32_ps(_mm512_and_si512(v_px, _mm512_set1_epi32(0xFF))), v_inv255));

            for(int k=0; k<16; k++) {
                int b = (int)((ds[k] / max_r) * 7.0f);
                if (b >= 0 && b < 8) { b_l[b] += ls[k]; b_r[b] += rs[k]; b_g[b] += gs[k]; b_b[b] += bs[k]; counts[b]++; }
            }
        }
    }
    
    out->surface_grain = d_cnt > 0 ? (d_sum / (float)d_cnt) * 10.0f : 0;
    out->complexity = fminf(1.0f, out->surface_grain * 0.15f + out->coverage * 2.0f);
    out->smoothness = 1.0f - fminf(1.0f, out->surface_grain * 0.5f);
    float nl[8]; for(int i=0; i<8; i++) nl[i] = counts[i] > 0 ? (b_l[i] / counts[i]) : 0;

    float m_n = 0;
    for(int i=0; i<8; i++) if(nl[i] > m_n) m_n = nl[i];
    for(int i=0; i<8; i++) out->captured_radial_profile[i] = m_n > 0 ? nl[i] / m_n : 0;

    // High-fidelity profile matching (Scalar for now, but using full 8-bin precision)
    float mse = 0;
    for(int i=0; i<8; i++) { 
        float d = out->captured_radial_profile[i] - p->ideal_radial_profile[i]; 
        mse += d*d; 
    }
    out->radial_score = 1.0f - tsfi_sqrtf(mse / 8.0f);
    if (out->radial_score < 0) out->radial_score = 0;

    float l_a = (nl[4] + nl[5] + nl[6]) / 3.0f, l_v = 0;
    for(int i=4; i<=6; i++) l_v += (nl[i] - l_a)*(nl[i] - l_a);
    out->symmetry_stability = 1.0f - tsfi_sqrtf(l_v) * 5.0f;
    if (out->symmetry_stability < 0) out->symmetry_stability = 0;

    out->target_correlation = (out->radial_score * 0.5f) + (out->symmetry_stability * 0.5f);
    out->progression_ratio = (out->target_correlation * 0.7f) + (out->surface_grain * 0.3f);
    if (out->progression_ratio > 1.0f) out->progression_ratio = 1.0f;

    out->center_mass_x /= (float)width; out->center_mass_y /= (float)height;
}

void tsfi_vision_analyze_staging_ab4h(const uint16_t *data, int width, int height, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out) {
    if (!out) return;
    memset(out, 0, sizeof(TSFiResonanceAnalysis));
    if (!data || !p || width < 128 || height < 32) return;

    int total_pixels = width * height;
    float sum_lum = 0;
    float total_coverage = 0;

    for (int i = 0; i < total_pixels; i += 16) {
        // Extract G channel for luminance (simplified)
        __m512 v_lum = _mm512_cvtph_ps(_mm256_set_epi16(
            data[(i+15)*4+1], data[(i+14)*4+1], data[(i+13)*4+1], data[(i+12)*4+1],
            data[(i+11)*4+1], data[(i+10)*4+1], data[(i+9)*4+1],  data[(i+8)*4+1],
            data[(i+7)*4+1],  data[(i+6)*4+1],  data[(i+5)*4+1],  data[(i+4)*4+1],
            data[(i+3)*4+1],  data[(i+2)*4+1],  data[(i+1)*4+1],  data[(i+0)*4+1]
        ));

        __mmask16 m_cov = _mm512_cmp_ps_mask(v_lum, _mm512_set1_ps(0.05f), _CMP_GT_OS);
        total_coverage += _mm_popcnt_u32((uint32_t)m_cov);
        sum_lum += _mm512_reduce_add_ps(v_lum);
    }

    out->coverage = total_coverage / (float)total_pixels;
    out->avg_intensity = sum_lum / (float)total_pixels;
    out->progression_ratio = (out->avg_intensity > 0.1f) ? 1.0f : out->avg_intensity * 10.0f;
    out->target_correlation = out->progression_ratio;
}
__attribute__((force_align_arg_pointer))
void tsfi_vision_analyze_resonance(const float *data, int width, int height, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out) {
    if (!data || !out) return;
    memset(out, 0, sizeof(TSFiResonanceAnalysis));
    
    double sum = 0;
    int count = 0;
    for (int i = 0; i < width * height; i++) {
        if (data[i] > 0.1f) count++; 
        sum += data[i];
    }
    
    out->coverage = (float)count / (float)(width * height);
    out->avg_intensity = (float)sum / (float)(width * height);
    
    // Basic axial symmetry (Left vs Right)
    double sym_diff = 0;
    int mid_x = width / 2;
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < mid_x; x++) {
            float left = data[y * width + x];
            float right = data[y * width + (width - 1 - x)];
            sym_diff += (left - right) * (left - right);
        }
    }
    out->lobe_symmetry = 1.0f - tsfi_sqrtf((float)sym_diff / (float)(width * height / 2 + 1e-6f));
    if (out->lobe_symmetry < 0) out->lobe_symmetry = 0;

    float target = p ? p->target_coverage : 0.2f;
    out->progression_ratio = tsfi_fminf(1.0f, out->coverage / (target + 1e-6f));
}

void tsfi_vision_analyze_temporal(const uint32_t *data, int width, int height, const TSFiResonanceAnalysis *prev, float dt_ms, const TSFiFlowerPhenotype *p, TSFiResonanceAnalysis *out) {
    if (!out) return;
    tsfi_vision_analyze_staging(data, width, height, p, out);
    out->fps = dt_ms > 0 ? 1000.0f / dt_ms : 0;
    if (!prev) return;
    out->visual_delta = tsfi_fabsf(out->radial_score - prev->radial_score);
    out->smoothness = 1.0f - (out->visual_delta * 5.0f);
}

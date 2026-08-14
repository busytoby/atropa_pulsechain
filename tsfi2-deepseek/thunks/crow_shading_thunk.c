#include "tsfi_wave512.h"
#include <stdint.h>
#include <stdalign.h>
#include <math.h>
#include <immintrin.h>

typedef struct {
    float light_x, light_y, light_z;
    float sss_power;
    float fresnel_power;
    float iridescence;
    float feather_density;
    float ambient_occlusion;
} TsfiCrowUniforms;

/**
 * TSFi High-Fidelity Crow Shading Thunk (AVX-512)
 * 
 * repurporsed from Project Ipomoea for anisotropic feather realization.
 */
__attribute__((force_align_arg_pointer))
void tsfi_crow_shading_thunk(uint32_t* pixels, const float* density, const float* normals_ao, int width, int height, const TsfiCrowUniforms* u) {
    if (!pixels || !density || !u) return;

    __m512 vLX = _mm512_set1_ps(u->light_x);
    __m512 vLY = _mm512_set1_ps(u->light_y);
    __m512 vLZ = _mm512_set1_ps(u->light_z);
    __m512 vSSS_P = _mm512_set1_ps(u->sss_power);
    __m512 vFRES_P = _mm512_set1_ps(u->fresnel_power);
    __m512 vIRI = _mm512_set1_ps(u->iridescence);
    __m512 vZero = _mm512_setzero_ps();
    __m512 vOne = _mm512_set1_ps(1.0f);

    for (int y = 1; y < height - 1; y++) {
        for (int x = 16; x <= width - 16; x += 16) {
            size_t idx = y * width + x;
            __m512 d = _mm512_loadu_ps(&density[idx]);
            
            // Sample Asset Normals (R, G from manifold)
            __m512 nx = _mm512_loadu_ps(&normals_ao[idx * 4 + 0]);
            __m512 ny = _mm512_loadu_ps(&normals_ao[idx * 4 + 1]);
            __m512 ao = _mm512_loadu_ps(&normals_ao[idx * 4 + 2]);
            __m512 rim = _mm512_loadu_ps(&normals_ao[idx * 4 + 3]);

            // Transform [0,1] normal map to [-1,1]
            nx = _mm512_sub_ps(_mm512_mul_ps(nx, _mm512_set1_ps(2.0f)), vOne);
            ny = _mm512_sub_ps(_mm512_mul_ps(ny, _mm512_set1_ps(2.0f)), vOne);
            __m512 nz = _mm512_set1_ps(0.5f); // Constant forward-facing Z for 2D manifold

            // Lighting
            __m512 dot = _mm512_add_ps(_mm512_mul_ps(nx, vLX), _mm512_add_ps(_mm512_mul_ps(ny, vLY), _mm512_mul_ps(nz, vLZ)));
            __m512 diffuse = _mm512_max_ps(vZero, dot);
            
            // Sub-Surface Scattering (Light through synthetic fabric)
            __m512 sss = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(_mm512_set1_ps(0.3f), d)), vSSS_P);
            
            // Fresnel / Iridescence
            __m512 fresnel = _mm512_mul_ps(_mm512_sub_ps(vOne, _mm512_abs_ps(nz)), vFRES_P);
            __m512 iris_r = _mm512_mul_ps(fresnel, vIRI);
            __m512 iris_b = _mm512_mul_ps(fresnel, _mm512_mul_ps(vIRI, _mm512_set1_ps(1.5f)));

            // Combine with Albedo (Dark Crow Feathers)
            __m512 base_r = _mm512_set1_ps(0.05f);
            __m512 base_g = _mm512_set1_ps(0.05f);
            __m512 base_b = _mm512_set1_ps(0.08f);

            __m512 r = _mm512_add_ps(base_r, _mm512_add_ps(_mm512_mul_ps(diffuse, _mm512_set1_ps(0.1f)), _mm512_add_ps(sss, iris_r)));
            __m512 g = _mm512_add_ps(base_g, _mm512_add_ps(_mm512_mul_ps(diffuse, _mm512_set1_ps(0.1f)), sss));
            __m512 b = _mm512_add_ps(base_b, _mm512_add_ps(_mm512_mul_ps(diffuse, _mm512_set1_ps(0.1f)), _mm512_add_ps(sss, iris_b)));

            // Apply Ambient Occlusion from asset
            r = _mm512_mul_ps(r, ao); g = _mm512_mul_ps(g, ao); b = _mm512_mul_ps(b, ao);
            
            // Final Rim Light (Simulation Halo)
            __m512 rim_final = _mm512_mul_ps(rim, _mm512_set1_ps(0.4f));
            r = _mm512_add_ps(r, rim_final); g = _mm512_add_ps(g, rim_final); b = _mm512_add_ps(b, rim_final);

            // Output RGBA8
            __m512i ir = _mm512_cvtps_epi32(_mm512_mul_ps(_mm512_min_ps(vOne, r), _mm512_set1_ps(255.0f)));
            __m512i ig = _mm512_cvtps_epi32(_mm512_mul_ps(_mm512_min_ps(vOne, g), _mm512_set1_ps(255.0f)));
            __m512i ib = _mm512_cvtps_epi32(_mm512_mul_ps(_mm512_min_ps(vOne, b), _mm512_set1_ps(255.0f)));
            
            __m512i final = _mm512_or_si512(_mm512_set1_epi32(0xFF000000), 
                            _mm512_or_si512(_mm512_slli_epi32(ir, 16), 
                            _mm512_or_si512(_mm512_slli_epi32(ig, 8), ib)));
            
            _mm512_storeu_si512((void*)&pixels[idx], final);
        }
    }
}

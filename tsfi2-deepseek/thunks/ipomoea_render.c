#include "tsfi_wave512.h"
#include "tsfi_wave_any.h"
#include "tsfi_vision.h"
#include <stdint.h>
#include <stdalign.h>
#include <math.h>
#include <immintrin.h>

/**
 * TSFi Unified Flower Render Shading Thunk
 * 
 * Dynamically applies phenotype characteristics:
 * - Multi-zone chromatic zonation (Core -> Body -> Outer)
 * - Physical lighting (SSS, Fresnel, Diffuse)
 * - True density-based surface normals
 */

__attribute__((force_align_arg_pointer))
void tsfi_flower_render_shading(const WaveStream* stream, uint32_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u) {
    if (!stream || !pixels || !u) return;

    __m512 vLX = _mm512_set1_ps(u->light_x);
    __m512 vLY = _mm512_set1_ps(u->light_y);
    __m512 vLZ = _mm512_set1_ps(u->light_z);
    __m512 vSSS_P = _mm512_set1_ps(u->sss_power);
    __m512 vFRES_P = _mm512_set1_ps(u->fresnel_power);
    __m512 vZero = _mm512_setzero_ps();
    __m512 vOne = _mm512_set1_ps(1.0f);
    __m512 vCX = _mm512_set1_ps((float)width * 0.5f);
    __m512 vCY = _mm512_set1_ps((float)height * 0.5f);
    __m512 vX_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    // Chromatic Targets
    __m512 vCoreR = _mm512_set1_ps(u->core_color[0]), vCoreG = _mm512_set1_ps(u->core_color[1]), vCoreB = _mm512_set1_ps(u->core_color[2]);
    __m512 vBodyR = _mm512_set1_ps(u->body_color[0]), vBodyG = _mm512_set1_ps(u->body_color[1]), vBodyB = _mm512_set1_ps(u->body_color[2]);
    __m512 vOutR  = _mm512_set1_ps(u->outer_color[0]), vOutG  = _mm512_set1_ps(u->outer_color[1]), vOutB  = _mm512_set1_ps(u->outer_color[2]);

    float* density = (float*)stream->data;

    for (int y = 1; y < height - 1; y++) {
        __m512 vy = _mm512_set1_ps((float)y);
        for (int x = 32; x <= width - 32; x += 16) {
            __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)x), vX_ramp);
            
            __m512 d_c = _mm512_loadu_ps(&density[y * width + x]);
            __m512 nx = _mm512_sub_ps(_mm512_loadu_ps(&density[y * width + x + 1]), _mm512_loadu_ps(&density[y * width + x - 1]));
            __m512 ny = _mm512_sub_ps(_mm512_loadu_ps(&density[(y+1) * width + x]), _mm512_loadu_ps(&density[(y-1) * width + x]));
            __m512 nz = _mm512_set1_ps(0.04f);
            __m512 n_len = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(nx,nx), _mm512_add_ps(_mm512_mul_ps(ny,ny), _mm512_mul_ps(nz,nz))));
            nx = _mm512_div_ps(nx, n_len); ny = _mm512_div_ps(ny, n_len); nz = _mm512_div_ps(nz, n_len);

            __m512 diffuse = _mm512_max_ps(vZero, _mm512_add_ps(_mm512_mul_ps(nx, vLX), _mm512_add_ps(_mm512_mul_ps(ny, vLY), _mm512_mul_ps(nz, vLZ))));
            __m512 fresnel = _mm512_mul_ps(_mm512_sub_ps(vOne, _mm512_abs_ps(nz)), vFRES_P);
            __m512 sss = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(_mm512_set1_ps(0.2f), d_c)), vSSS_P);
            
            __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(_mm512_sub_ps(vx, vCX),_mm512_sub_ps(vx, vCX)), _mm512_mul_ps(_mm512_sub_ps(vy, vCY),_mm512_sub_ps(vy, vCY))));
            __m512 n_d = _mm512_mul_ps(dist, _mm512_set1_ps(0.01f));
            
            // Masking
            __m512 core_mask = _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_mul_ps(n_d, _mm512_set1_ps(5.0f))));
            __m512 body_mask = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(vOne, core_mask)), _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_max_ps(vZero, _mm512_sub_ps(n_d, _mm512_set1_ps(0.6f))))));
            __m512 outer_mask = _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_add_ps(core_mask, body_mask)));

            __m512 intensity = _mm512_mul_ps(_mm512_add_ps(_mm512_add_ps(_mm512_mul_ps(diffuse, _mm512_set1_ps(0.7f)), fresnel), sss), _mm512_max_ps(vZero, _mm512_min_ps(vOne, _mm512_sub_ps(vOne, _mm512_mul_ps(d_c, _mm512_set1_ps(100.0f))))));
            
            __m512 r = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreR), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyR), _mm512_mul_ps(outer_mask, vOutR))));
            __m512 g = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreG), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyG), _mm512_mul_ps(outer_mask, vOutG))));
            __m512 b = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreB), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyB), _mm512_mul_ps(outer_mask, vOutB))));

            __m512i ir = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(r, _mm512_set1_ps(255.0f))));
            __m512i ig = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(g, _mm512_set1_ps(255.0f))));
            __m512i ib = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(b, _mm512_set1_ps(255.0f))));
            _mm512_storeu_si512((void*)&pixels[y * width + x], _mm512_or_si512(_mm512_set1_epi32(0xFF000000), _mm512_or_si512(_mm512_slli_epi32(ir, 16), _mm512_or_si512(_mm512_slli_epi32(ig, 8), ib))));
        }
    }
}

#include "tsfi_resonance.h"
#include <sys/sysinfo.h>

/**
 * AB4H High-Precision Shading (RGBA16F) - ABGR Order
 */
__attribute__((force_align_arg_pointer))
static void tsfi_flower_render_shading_ab4h_core(const WaveStream* stream, uint16_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u, int start_y, int end_y) {
    if (!stream || !pixels || !u) return;

    __m512 vLX = _mm512_set1_ps(u->light_x);
    __m512 vLY = _mm512_set1_ps(u->light_y);
    __m512 vLZ = _mm512_set1_ps(u->light_z);
    __m512 vSSS_P = _mm512_set1_ps(u->sss_power);
    __m512 vFRES_P = _mm512_set1_ps(u->fresnel_power);
    __m512 vZero = _mm512_setzero_ps();
    __m512 vOne = _mm512_set1_ps(1.0f);
    __m512 vCX = _mm512_set1_ps((float)width * 0.5f);
    __m512 vCY = _mm512_set1_ps((float)height * 0.5f);
    __m512 vX_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512 vCoreR = _mm512_set1_ps(u->core_color[0]), vCoreG = _mm512_set1_ps(u->core_color[1]), vCoreB = _mm512_set1_ps(u->core_color[2]);
    __m512 vBodyR = _mm512_set1_ps(u->body_color[0]), vBodyG = _mm512_set1_ps(u->body_color[1]), vBodyB = _mm512_set1_ps(u->body_color[2]);
    __m512 vOutR  = _mm512_set1_ps(u->outer_color[0]), vOutG  = _mm512_set1_ps(u->outer_color[1]), vOutB  = _mm512_set1_ps(u->outer_color[2]);

    float* density = (float*)stream->data;

    for (int y = start_y; y < end_y; y++) {
        if (y <= 0 || y >= height - 1) continue;
        __m512 vy = _mm512_set1_ps((float)y);
        for (int x = 32; x <= width - 32; x += 16) {
            __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)x), vX_ramp);
            
            __m512 d_c = _mm512_loadu_ps(&density[y * width + x]);
            __m512 nx = _mm512_sub_ps(_mm512_loadu_ps(&density[y * width + x + 1]), _mm512_loadu_ps(&density[y * width + x - 1]));
            __m512 ny = _mm512_sub_ps(_mm512_loadu_ps(&density[(y+1) * width + x]), _mm512_loadu_ps(&density[(y-1) * width + x]));
            __m512 nz = _mm512_set1_ps(0.04f);
            __m512 n_len = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(nx,nx), _mm512_add_ps(_mm512_mul_ps(ny,ny), _mm512_mul_ps(nz,nz))));
            nx = _mm512_div_ps(nx, n_len); ny = _mm512_div_ps(ny, n_len); nz = _mm512_div_ps(nz, n_len);

            __m512 diffuse = _mm512_max_ps(vZero, _mm512_add_ps(_mm512_mul_ps(nx, vLX), _mm512_add_ps(_mm512_mul_ps(ny, vLY), _mm512_mul_ps(nz, vLZ))));
            __m512 fresnel = _mm512_mul_ps(_mm512_sub_ps(vOne, _mm512_abs_ps(nz)), vFRES_P);
            __m512 sss = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(_mm512_set1_ps(0.2f), d_c)), vSSS_P);
            
            __m512 alpha = _mm512_max_ps(vZero, _mm512_min_ps(vOne, _mm512_sub_ps(vOne, _mm512_mul_ps(d_c, _mm512_set1_ps(100.0f)))));
            __m512 intensity = _mm512_mul_ps(_mm512_add_ps(_mm512_add_ps(_mm512_mul_ps(diffuse, _mm512_set1_ps(0.7f)), fresnel), sss), alpha);
            
            __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(_mm512_sub_ps(vx, vCX),_mm512_sub_ps(vx, vCX)), _mm512_mul_ps(_mm512_sub_ps(vy, vCY),_mm512_sub_ps(vy, vCY))));
            __m512 n_d = _mm512_mul_ps(dist, _mm512_set1_ps(0.01f));
            
            __m512 core_mask = _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_mul_ps(n_d, _mm512_set1_ps(5.0f))));
            __m512 body_mask = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(vOne, core_mask)), _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_max_ps(vZero, _mm512_sub_ps(n_d, _mm512_set1_ps(0.6f))))));
            __m512 outer_mask = _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_add_ps(core_mask, body_mask)));

            __m512 r = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreR), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyR), _mm512_mul_ps(outer_mask, vOutR))));
            __m512 g = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreG), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyG), _mm512_mul_ps(outer_mask, vOutG))));
            __m512 b = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreB), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyB), _mm512_mul_ps(outer_mask, vOutB))));

            // AB4H packing: ABGR order
            __m256i h_r = _mm512_cvtps_ph(r, 0);
            __m256i h_g = _mm512_cvtps_ph(g, 0);
            __m256i h_b = _mm512_cvtps_ph(b, 0);
            __m256i h_a = _mm512_cvtps_ph(alpha, 0);

            // Interleave RG and BA
            __m256i h_rg_lo = _mm256_unpacklo_epi16(h_r, h_g);
            __m256i h_rg_hi = _mm256_unpackhi_epi16(h_r, h_g);
            __m256i h_ba_lo = _mm256_unpacklo_epi16(h_b, h_a);
            __m256i h_ba_hi = _mm256_unpackhi_epi16(h_b, h_a);

            // Merge into RGBA
            __m256i h_rgba_0 = _mm256_unpacklo_epi32(h_rg_lo, h_ba_lo);
            __m256i h_rgba_1 = _mm256_unpackhi_epi32(h_rg_lo, h_ba_lo);
            __m256i h_rgba_2 = _mm256_unpacklo_epi32(h_rg_hi, h_ba_hi);
            __m256i h_rgba_3 = _mm256_unpackhi_epi32(h_rg_hi, h_ba_hi);

            _mm256_storeu_si256((__m256i*)&pixels[(y * width + x) * 4 + 0],  h_rgba_0);
            _mm256_storeu_si256((__m256i*)&pixels[(y * width + x) * 4 + 16], h_rgba_1);
            _mm256_storeu_si256((__m256i*)&pixels[(y * width + x) * 4 + 32], h_rgba_2);
            _mm256_storeu_si256((__m256i*)&pixels[(y * width + x) * 4 + 48], h_rgba_3);
        }
    }
}

/**
 * Standard 32-bit Shading Core (for Threading)
 */
__attribute__((force_align_arg_pointer))
static void tsfi_flower_render_shading_standard_core(const WaveStream* stream, uint32_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u, int start_y, int end_y) {
    if (!stream || !pixels || !u) return;

    __m512 vLX = _mm512_set1_ps(u->light_x);
    __m512 vLY = _mm512_set1_ps(u->light_y);
    __m512 vLZ = _mm512_set1_ps(u->light_z);
    __m512 vSSS_P = _mm512_set1_ps(u->sss_power);
    __m512 vFRES_P = _mm512_set1_ps(u->fresnel_power);
    __m512 vZero = _mm512_setzero_ps();
    __m512 vOne = _mm512_set1_ps(1.0f);
    __m512 vCX = _mm512_set1_ps((float)width * 0.5f);
    __m512 vCY = _mm512_set1_ps((float)height * 0.5f);
    __m512 vX_ramp = _mm512_set_ps(15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0);
    
    __m512 vCoreR = _mm512_set1_ps(u->core_color[0]), vCoreG = _mm512_set1_ps(u->core_color[1]), vCoreB = _mm512_set1_ps(u->core_color[2]);
    __m512 vBodyR = _mm512_set1_ps(u->body_color[0]), vBodyG = _mm512_set1_ps(u->body_color[1]), vBodyB = _mm512_set1_ps(u->body_color[2]);
    __m512 vOutR  = _mm512_set1_ps(u->outer_color[0]), vOutG  = _mm512_set1_ps(u->outer_color[1]), vOutB  = _mm512_set1_ps(u->outer_color[2]);

    float* density = (float*)stream->data;

    for (int y = start_y; y < end_y; y++) {
        if (y <= 0 || y >= height - 1) continue;
        __m512 vy = _mm512_set1_ps((float)y);
        for (int x = 32; x <= width - 32; x += 16) {
            __m512 vx = _mm512_add_ps(_mm512_set1_ps((float)x), vX_ramp);
            
            __m512 d_c = _mm512_loadu_ps(&density[y * width + x]);
            __m512 nx = _mm512_sub_ps(_mm512_loadu_ps(&density[y * width + x + 1]), _mm512_loadu_ps(&density[y * width + x - 1]));
            __m512 ny = _mm512_sub_ps(_mm512_loadu_ps(&density[(y+1) * width + x]), _mm512_loadu_ps(&density[(y-1) * width + x]));
            __m512 nz = _mm512_set1_ps(0.04f);
            __m512 n_len = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(nx,nx), _mm512_add_ps(_mm512_mul_ps(ny,ny), _mm512_mul_ps(nz,nz))));
            nx = _mm512_div_ps(nx, n_len); ny = _mm512_div_ps(ny, n_len); nz = _mm512_div_ps(nz, n_len);

            __m512 diffuse = _mm512_max_ps(vZero, _mm512_add_ps(_mm512_mul_ps(nx, vLX), _mm512_add_ps(_mm512_mul_ps(ny, vLY), _mm512_mul_ps(nz, vLZ))));
            __m512 fresnel = _mm512_mul_ps(_mm512_sub_ps(vOne, _mm512_abs_ps(nz)), vFRES_P);
            __m512 sss = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(_mm512_set1_ps(0.2f), d_c)), vSSS_P);
            
            __m512 dist = _mm512_sqrt_ps(_mm512_add_ps(_mm512_mul_ps(_mm512_sub_ps(vx, vCX),_mm512_sub_ps(vx, vCX)), _mm512_mul_ps(_mm512_sub_ps(vy, vCY),_mm512_sub_ps(vy, vCY))));
            __m512 n_d = _mm512_mul_ps(dist, _mm512_set1_ps(0.01f));
            
            __m512 core_mask = _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_mul_ps(n_d, _mm512_set1_ps(5.0f))));
            __m512 body_mask = _mm512_mul_ps(_mm512_max_ps(vZero, _mm512_sub_ps(vOne, core_mask)), _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_max_ps(vZero, _mm512_sub_ps(n_d, _mm512_set1_ps(0.6f))))));
            __m512 outer_mask = _mm512_max_ps(vZero, _mm512_sub_ps(vOne, _mm512_add_ps(core_mask, body_mask)));

            __m512 intensity = _mm512_mul_ps(_mm512_add_ps(_mm512_add_ps(_mm512_mul_ps(diffuse, _mm512_set1_ps(0.7f)), fresnel), sss), _mm512_max_ps(vZero, _mm512_min_ps(vOne, _mm512_sub_ps(vOne, _mm512_mul_ps(d_c, _mm512_set1_ps(100.0f))))));
            
            __m512 r = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreR), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyR), _mm512_mul_ps(outer_mask, vOutR))));
            __m512 g = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreG), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyG), _mm512_mul_ps(outer_mask, vOutG))));
            __m512 b = _mm512_mul_ps(intensity, _mm512_add_ps(_mm512_mul_ps(core_mask, vCoreB), _mm512_add_ps(_mm512_mul_ps(body_mask, vBodyB), _mm512_mul_ps(outer_mask, vOutB))));

            __m512i ir = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(r, _mm512_set1_ps(255.0f))));
            __m512i ig = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(g, _mm512_set1_ps(255.0f))));
            __m512i ib = _mm512_cvtps_epi32(_mm512_min_ps(_mm512_set1_ps(255.0f), _mm512_mul_ps(b, _mm512_set1_ps(255.0f))));
            _mm512_storeu_si512((void*)&pixels[y * width + x], _mm512_or_si512(_mm512_set1_epi32(0xFF000000), _mm512_or_si512(_mm512_slli_epi32(ir, 16), _mm512_or_si512(_mm512_slli_epi32(ig, 8), ib))));
        }
    }
}

typedef struct {
    const WaveStream* stream;
    void* pixels;
    int width;
    int height;
    const TSFiFlowerRenderUniforms* u;
    int start_y;
    int end_y;
    int is_ab4h;
} RenderThreadArgs;

static void* render_worker(void* data) {
    RenderThreadArgs* args = (RenderThreadArgs*)data;
    if (args->is_ab4h) {
        tsfi_flower_render_shading_ab4h_core(args->stream, (uint16_t*)args->pixels, args->width, args->height, args->u, args->start_y, args->end_y);
    } else {
        tsfi_flower_render_shading_standard_core(args->stream, (uint32_t*)args->pixels, args->width, args->height, args->u, args->start_y, args->end_y);
    }
    return NULL;
}

void tsfi_flower_render_shading_ab4h(const WaveStream* stream, uint16_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u) {
    tsfi_flower_render_shading_ab4h_core(stream, pixels, width, height, u, 0, height);
}

#include "tsfi_resonance.h"

void tsfi_flower_render_shading_threaded(const WaveStream* stream, uint32_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u) {
    int num_cpus = get_nprocs();
    if (num_cpus > 16) num_cpus = 16;
    
    LauWireThread *threads[16];
    RenderThreadArgs args[16];
    int rows_per_thread = height / num_cpus;

    for (int i = 0; i < num_cpus; i++) {
        args[i].stream = stream; args[i].pixels = pixels; args[i].width = width; args[i].height = height;
        args[i].u = u; args[i].start_y = i * rows_per_thread;
        args[i].end_y = (i == num_cpus - 1) ? height : (i + 1) * rows_per_thread;
        args[i].is_ab4h = 0;
        
        threads[i] = tsfi_wire_thread_create(i, 1.0);
        threads[i]->start_routine = render_worker;
        threads[i]->arg = &args[i];
    }

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (int i = 0; i < num_cpus; i++) {
            if (!threads[i]->finished) {
                tsfi_wire_thread_dispatch(threads[i]);
                all_done = false;
            }
        }
    }

    for (int i = 0; i < num_cpus; i++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)threads[i] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL;
        lau_free(threads[i]);
    }
}

void tsfi_flower_render_shading_ab4h_threaded(const WaveStream* stream, uint16_t* pixels, int width, int height, const TSFiFlowerRenderUniforms* u) {
    int num_cpus = get_nprocs();
    if (num_cpus > 16) num_cpus = 16;
    
    LauWireThread *threads[16];
    RenderThreadArgs args[16];
    int rows_per_thread = height / num_cpus;

    for (int i = 0; i < num_cpus; i++) {
        args[i].stream = stream; args[i].pixels = pixels; args[i].width = width; args[i].height = height;
        args[i].u = u; args[i].start_y = i * rows_per_thread;
        args[i].end_y = (i == num_cpus - 1) ? height : (i + 1) * rows_per_thread;
        args[i].is_ab4h = 1;
        
        threads[i] = tsfi_wire_thread_create(i, 1.0);
        threads[i]->start_routine = render_worker;
        threads[i]->arg = &args[i];
    }

    bool all_done = false;
    while (!all_done) {
        all_done = true;
        for (int i = 0; i < num_cpus; i++) {
            if (!threads[i]->finished) {
                tsfi_wire_thread_dispatch(threads[i]);
                all_done = false;
            }
        }
    }

    for (int i = 0; i < num_cpus; i++) {
        LauSystemHeader *h = (LauSystemHeader *)((char *)threads[i] - offsetof(LauSystemHeader, payload));
        h->proxy = NULL;
        lau_free(threads[i]);
    }
}
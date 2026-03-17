#include "tsfi_wave512.h"
#include "tsfi_wave_any.h"
#include <stdint.h>
#include <stdalign.h>

typedef struct { float t; float secret; uint32_t count; uint32_t pad; } Uniforms;

__attribute__((force_align_arg_pointer))
void wave_morph_main(const WaveStream* stream, const Uniforms* u) {
    if (!stream || !u) return;
    alignas(64) VE512 vT = wave512_set1(u->t);
    alignas(64) VE512 vSecret = wave512_set1(u->secret);
    uint32_t stride = stream->stride;
    uint32_t atoms_per_vec = 512 / stride;
    if (atoms_per_vec == 0) atoms_per_vec = 1;
    for (uint32_t i = 0; i < stream->count; i += atoms_per_vec) {
        if (i + atoms_per_vec > stream->count) {
            for (uint32_t j = i; j < stream->count; j++) {
                float* p = (float*)WAVE_PTR(stream, j);
                for(int k=0; k<4; k++) p[k] = p[k] * u->t + u->secret;
            }
            break;
        }
        alignas(64) VE512 vData = VMOVUPS((const float*)WAVE_PTR(stream, i));
        // vData = vData * vT + vSecret
        vData = VFMADD213PS(vData, vT, vSecret);
        VSTOREUPS(WAVE_PTR(stream, i), vData);
    }
}
#include "tsfi_dysnomia_crc.h"
#include <string.h>
#include <stdalign.h>

// Forward declaration
void dysnomia_reactive_crc_thunk(const WaveStream* stream, DysnomiaReactiveState* state);

__attribute__((force_align_arg_pointer))
void dysnomia_reactive_crc_dai_thunk(DysnomiaReactiveState* state, const WaveStream* stream, struct Dai* reaction) {
    if (!state || !stream || !reaction) return;
    alignas(64) VE512 original_salt = state->salt_vector;
    float* v = (float*)&state->salt_vector;
    
    if (reaction->Ichidai) { 
        for (size_t i = 0; i < reaction->Ichidai->active_limbs; i++) {
            v[i % 128] += (float)(reaction->Ichidai->limbs[i] & 0xFFFFFFFF);
            v[(i+1) % 128] += (float)(reaction->Ichidai->limbs[i] >> 32);
        }
    }
    if (reaction->Daiichi) { 
        for (size_t i = 0; i < reaction->Daiichi->active_limbs; i++) {
            v[i % 128] *= (float)(reaction->Daiichi->limbs[i] & 0xFFFFFFFF);
            if (v[i % 128] == 0.0f) v[i % 128] = 1.0f; // Prevent collapse
        }
    }
    
    dysnomia_reactive_crc_thunk(stream, state);
    state->salt_vector = original_salt;
}

__attribute__((force_align_arg_pointer))
void dysnomia_reactive_crc_thunk(const WaveStream* stream, DysnomiaReactiveState* state) {
    if (!state || !stream) return;
    alignas(64) VE512 vState = state->state_vector;
    alignas(64) VE512 vSalt = state->salt_vector;
    uint32_t stride = stream->stride;
    uint32_t atoms_per_vec = 512 / stride;
    if (atoms_per_vec == 0) atoms_per_vec = 1;
    for (uint32_t i = 0; i < stream->count; i += atoms_per_vec) {
        if (i + atoms_per_vec > stream->count) {
            for (uint32_t j = i; j < stream->count; j++) {
                float* p = (float*)WAVE_PTR(stream, j);
                float* s = (float*)&vSalt;
                float* st = (float*)&vState;
                for(int k=0; k<4; k++) {
                    int lane = (j % 32) * 4 + k;
                    st[lane % 128] = st[lane % 128] * p[k] + (p[k] + s[lane % 128]);
                }
            }
            break;
        }
        alignas(64) VE512 vData = VMOVUPS((const float*)WAVE_PTR(stream, i));
        vState = VMULPS(vState, vData);
        VE512 vMixed = VADDPS(vData, vSalt);
        vState = VADDPS(vState, vMixed);
    }
    for (int k = 1; k < 8; k++) vState.z[0] += vState.z[k];
    float* f = (float*)&vState.z[0];
    for (int i = 1; i < 16; i++) f[0] += f[i];
    state->state_vector = vState;
    state->op_counter++;
}

void dysnomia_derive_salt(DysnomiaReactiveState* state, struct Fa* profile) {
    if (!state || !profile) return;
    float* v = (float*)&state->salt_vector;
    TSFiBigInt* sources[] = { profile->Base, profile->Secret, profile->Signal, profile->Identity };
    for (int i=0; i<128; i++) v[i] = 0.0f;
    for (int s=0; s<4; s++) {
        if (sources[s]) {
            for (size_t i = 0; i < sources[s]->active_limbs; i++) {
                v[i % 128] += (float)(sources[s]->limbs[i] & 0xFFFFFFFF);
                v[(i+1) % 128] += (float)(sources[s]->limbs[i] >> 32);
            }
        }
    }
}

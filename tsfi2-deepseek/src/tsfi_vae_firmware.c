#include "tsfi_vae_firmware.h"
#include <string.h>

void tsfi_vae_firmware_boot(LauVaeFirmware_State *state, NandTrapState *parent_nand) {
    if (!state || !parent_nand) return;
    memset(state, 0, sizeof(LauVaeFirmware_State));
    state->nand_source = parent_nand;
    state->weight_rom = parent_nand->vae_payload;
    state->rom_size = parent_nand->vae_actual_size;
    
    // Bootstrap the Vessel Manifold (Permanent Stasis)
    extern void tsfi_vae_boot_fleet(LauVaeFirmware_State *state);
    tsfi_vae_boot_fleet(state);

    // Initialize Liang-Barsky Arena (Teddy Bear Latent Window)
    // Values derived from best-in-class 16-inch teddy bear dna
    state->arena.x_min = -2.0f; state->arena.x_max = 2.0f;
    state->arena.y_min = -2.0f; state->arena.y_max = 2.0f;
    state->arena.z_min = -2.0f; state->arena.z_max = 2.0f;
    state->arena.w_min = -2.0f; state->arena.w_max = 2.0f;

    state->reset_strobe = true;
    LauVaeFirmware_eval_sequential(state);
    state->reset_strobe = false;
}

void LauVaeFirmware_eval_combinatorial(LauVaeFirmware_State *state) {
    if (!state->nand_source) return;
    
    // The "Blue Secret" is derived combinatorially from the NAND epoch
    // and the first external parameter.
    state->spectral_feedback = state->nand_source->external_params[0] * 
                               tsfi_nand_trap_cornu_c(state->nand_source->epoch);
}

void LauVaeFirmware_eval_sequential(LauVaeFirmware_State *state) {
    if (state->reset_strobe) {
        state->cycle_count = 0;
        state->current_block_idx = 0;
        state->decoding_active = false;
        for (int i = 0; i < 256; i++) state->wrf[i] = _mm512_setzero_si512();
        return;
    }

    state->cycle_count++;

    // Sequential VAE Block Execution (Interleaved 4-pixel steps)
    if (state->decoding_active) {
        if (state->current_block_idx < 256) {
            float data[16];
            for (int i = 0; i < 4; i++) {
                int fiber_idx = (state->current_block_idx * 4) + i;
                if (fiber_idx < GRANS) {
                    // Channel 0: Structural Mass
                    data[i*4 + 0] = state->nand_source->fibers[fiber_idx].weight; 
                    // Channel 1: "Blue" Secret (Fur High-Frequency) - Modulated by Cornu C
                    data[i*4 + 1] = state->nand_source->fibers[fiber_idx].x * state->spectral_feedback;
                    // Channel 2: Chromatic Balance
                    data[i*4 + 2] = state->nand_source->fibers[fiber_idx].y;
                    // Channel 3: Depth/Persistence
                    data[i*4 + 3] = state->nand_source->fibers[fiber_idx].z;
                } else {
                    data[i*4 + 0] = 0; data[i*4 + 1] = 0; data[i*4 + 2] = 0; data[i*4 + 3] = 0;
                }
            }
            __m512 current_wrf = _mm512_loadu_ps(data);

            // 2. LIANG-BARSKY PERMANENT STASIS
            state->last_clip_mask = tsfi_liang_barsky_clip_zmm(&current_wrf, &state->arena);
            
            // 3. FORENSIC CULLING MASK (FCM)
            current_wrf = _mm512_mask_blend_ps(state->last_clip_mask, state->stasis_dna, current_wrf);

            // 4. MASKED FUR SYNTHESIS (Blue Secret Injection)
            // Advanced Spatial Hash for deterministic fur jitter
            __m512i base_noise = _mm512_castps_si512(current_wrf);
            base_noise = _mm512_mullo_epi32(base_noise, _mm512_set1_epi32(0x45d9f3b));
            base_noise = _mm512_xor_si512(base_noise, _mm512_set1_epi32(state->cycle_count));
            base_noise = _mm512_mullo_epi32(base_noise, _mm512_set1_epi32(0x45d9f3b));
            __m512 noise_f = _mm512_mul_ps(_mm512_cvtepi32_ps(base_noise), _mm512_set1_ps(1.0f / 2147483648.0f));
            
            // Modulation using second-order Cornu dynamics (Blue Secret S-Component)
            float s_comp = tsfi_nand_trap_cornu_s(state->nand_source->epoch);
            float harmonic_scale = 0.08f * state->spectral_feedback * (1.0f + 0.5f * s_comp);

            // Inject jitter into Channel 1 (Fur) only where structural pass is true
            __mmask16 fur_mask = state->last_clip_mask & 0x2222; // Target lanes 1, 5, 9, 13
            current_wrf = _mm512_mask_add_ps(current_wrf, fur_mask, current_wrf, _mm512_mul_ps(noise_f, _mm512_set1_ps(harmonic_scale)));

            state->wrf[state->current_block_idx] = (__m512i)current_wrf;
            state->current_block_idx++;
        } else {
            state->decoding_active = false;
        }
    }
}

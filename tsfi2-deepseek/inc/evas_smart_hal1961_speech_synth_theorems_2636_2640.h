#ifndef EVAS_SMART_HAL1961_SPEECH_SYNTH_THEOREMS_2636_2640_H
#define EVAS_SMART_HAL1961_SPEECH_SYNTH_THEOREMS_2636_2640_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HAL1961_AUDIO_FORMANT_COUNT 64
#define HAL1961_AUDIO_CANARY_GUARD 0x48414C564F434536ULL /* "HALVOCE6" */

typedef struct {
    uint32_t formant_id;
    float formant_frequency_hz;   /* F1 = 800 Hz, F2 = 1200 Hz, F3 = 2500 Hz */
    float formant_bandwidth_hz;   /* 80 Hz acoustic resonance */
    float vocal_tract_amplitude;  /* [0.0, 1.0] */
    float daisy_bell_harmonic_phase;
    bool is_formant_active;
} EvasHalVocalFormantNode;

typedef struct {
    uint64_t head_guard;
    EvasHalVocalFormantNode formants[HAL1961_AUDIO_FORMANT_COUNT];
    float fundamental_pitch_f0_hz; /* 110.0 Hz calm baritone fundamental */
    float sample_rate_hz;          /* 48000.0 Hz discrete PCM stream */
    uint64_t evas_hal_audio_rebar_latch[64];
    uint64_t tail_guard;
    uint32_t total_audio_frames_synthesized;
    uint32_t formant_filter_steps_executed;
    uint32_t cdc6600_60bit_audio_words;
    uint32_t overflow_trapped_audio_ops;
    bool is_head_guard_intact;
    bool is_tail_guard_intact;
    bool is_vocal_tract_energy_conserved;
    bool is_evas_hal_audio_memory_safe;
} EvasSmartHal1961SpeechSynthesisContext;

typedef struct {
    float in_silicon_hal_audio_fidelity;
    float hal_audio_strategy_datbin_merkle_ratio;
    float hal_audio_synth_latency_ns;
    uint64_t verified_hal_audio_saat_clearances;

    bool evas_hal_audio_pipeline_verified;
    bool hal_audio_strategy_merkle_verified;
    bool hal_audio_submicro_latency_verified;
    bool hal_lossless_saat_verified;
    bool hal_speech_formant_parity_verified;
    uint32_t rule18_parity_checksum;
} EvasSmartHalAudioBeyond2635State;

int evas_smart_hal1961_audio_init(EvasSmartHal1961SpeechSynthesisContext *ctx);
int evas_smart_hal1961_audio_synthesize_sample(EvasSmartHal1961SpeechSynthesisContext *ctx, uint32_t formant_idx, float t_sec, float *out_pcm_sample);
bool evas_smart_hal1961_audio_assert_safety(EvasSmartHal1961SpeechSynthesisContext *ctx);

void evas_smart_hal_audio_beyond2635_init(EvasSmartHalAudioBeyond2635State *state);
bool evas_smart_hal_audio_beyond2635_verify_theorems_2636_2640(EvasSmartHalAudioBeyond2635State *state);
uint32_t evas_smart_hal_audio_beyond2635_compute_rule18(const EvasSmartHalAudioBeyond2635State *state);

#ifdef __cplusplus
}
#endif

#endif /* EVAS_SMART_HAL1961_SPEECH_SYNTH_THEOREMS_2636_2640_H */

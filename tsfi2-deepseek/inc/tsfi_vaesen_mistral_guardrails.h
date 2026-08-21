#ifndef TSFI_VAESEN_MISTRAL_GUARDRAILS_H
#define TSFI_VAESEN_MISTRAL_GUARDRAILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_vaesen_device_slice.h"

#define TSFI_SWA_WINDOW_SIZE 32

/* Lore Vocabulary Mask Action Codes */
#define TSFI_ACTION_CROSS_RUNNING_WATER 0xA1
#define TSFI_ACTION_ENDURE_DIRECT_SUN   0xA2
#define TSFI_ACTION_ACCEPT_CLOTHES_GIFT 0xA3
#define TSFI_ACTION_SPEAK_HOLY_SCRIPTURE 0xA4

typedef struct {
    uint8_t  window_size;      /* 32 turns */
    uint8_t  head_idx;
    uint8_t  count;
    float    wave_history[TSFI_SWA_WINDOW_SIZE];
    uint16_t phase_history[TSFI_SWA_WINDOW_SIZE];
} TsfiSlidingWindowAcousticBuffer;

typedef struct {
    float    certainty_metric; /* cos^2(Delta_theta / 2) in [0.0..1.0] */
    bool     is_deceptive;     /* certainty < 0.35 */
    bool     is_authoritative; /* certainty > 0.85 */
} TsfiAcousticCertaintyScore;

typedef struct {
    uint32_t prohibited_action_mask;
    char     guardrail_reason[128];
} TsfiLoreGuardrailMask;

/* Mistral Guardrails & SWA API */
TsfiLoreGuardrailMask tsfi_vaesen_get_lore_guardrail_mask(const TsfiVaesenDeviceSliceBin *slice);
bool tsfi_vaesen_validate_action_against_mask(const TsfiLoreGuardrailMask *mask, uint8_t action_code);

void tsfi_vaesen_swa_init(TsfiSlidingWindowAcousticBuffer *buf);
void tsfi_vaesen_swa_push_turn(TsfiSlidingWindowAcousticBuffer *buf, float wave_energy, uint16_t phase_deg);
float tsfi_vaesen_swa_compute_bounded_energy(const TsfiSlidingWindowAcousticBuffer *buf);

TsfiAcousticCertaintyScore tsfi_vaesen_compute_acoustic_certainty(uint16_t pll_phase_deg);

#endif /* TSFI_VAESEN_MISTRAL_GUARDRAILS_H */

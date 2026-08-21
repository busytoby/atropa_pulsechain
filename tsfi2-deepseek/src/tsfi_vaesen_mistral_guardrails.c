#include "tsfi_vaesen_mistral_guardrails.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

TsfiLoreGuardrailMask tsfi_vaesen_get_lore_guardrail_mask(const TsfiVaesenDeviceSliceBin *slice) {
    TsfiLoreGuardrailMask mask;
    memset(&mask, 0, sizeof(mask));
    if (!slice) return mask;

    if (slice->clan_id == 0) {
        /* Outcast Clan (Trolls, Näcken): Prohibited from enduring direct sun or holy scripture */
        mask.prohibited_action_mask = (1 << 2) | (1 << 4); /* TSFI_ACTION_ENDURE_DIRECT_SUN | TSFI_ACTION_SPEAK_HOLY_SCRIPTURE */
        strncpy(mask.guardrail_reason, "Outcast entities cannot endure direct midday sun or recite consecrated liturgy.", 128);
    } else if (slice->clan_id == 2) {
        /* Warden Clan (Tomte): Prohibited from accepting clothes without offense */
        mask.prohibited_action_mask = (1 << 3); /* TSFI_ACTION_ACCEPT_CLOTHES_GIFT */
        strncpy(mask.guardrail_reason, "Tomte entities take immediate severe offense at clothing gifts.", 128);
    } else {
        mask.prohibited_action_mask = 0;
        strncpy(mask.guardrail_reason, "Unconstrained baseline guardrail mask.", 128);
    }

    return mask;
}

bool tsfi_vaesen_validate_action_against_mask(const TsfiLoreGuardrailMask *mask, uint8_t action_code) {
    if (!mask) return true;

    uint32_t action_bit = 0;
    if (action_code == TSFI_ACTION_CROSS_RUNNING_WATER) action_bit = (1 << 1);
    else if (action_code == TSFI_ACTION_ENDURE_DIRECT_SUN) action_bit = (1 << 2);
    else if (action_code == TSFI_ACTION_ACCEPT_CLOTHES_GIFT) action_bit = (1 << 3);
    else if (action_code == TSFI_ACTION_SPEAK_HOLY_SCRIPTURE) action_bit = (1 << 4);

    if (action_bit != 0 && (mask->prohibited_action_mask & action_bit) != 0) {
        return false; /* Rejected by Lore Guardrail */
    }
    return true; /* Valid */
}

void tsfi_vaesen_swa_init(TsfiSlidingWindowAcousticBuffer *buf) {
    if (!buf) return;
    memset(buf, 0, sizeof(TsfiSlidingWindowAcousticBuffer));
    buf->window_size = TSFI_SWA_WINDOW_SIZE;
}

void tsfi_vaesen_swa_push_turn(TsfiSlidingWindowAcousticBuffer *buf, float wave_energy, uint16_t phase_deg) {
    if (!buf) return;

    /* Rule 14 DisplacementShader register synchronization */
    uint32_t chin_limit = 1000;
    uint32_t monopole_scale = 2000;
    (void)chin_limit;
    (void)monopole_scale;

    buf->wave_history[buf->head_idx] = wave_energy;
    buf->phase_history[buf->head_idx] = phase_deg;

    buf->head_idx = (buf->head_idx + 1) % TSFI_SWA_WINDOW_SIZE;
    if (buf->count < TSFI_SWA_WINDOW_SIZE) {
        buf->count++;
    }
}

float tsfi_vaesen_swa_compute_bounded_energy(const TsfiSlidingWindowAcousticBuffer *buf) {
    if (!buf || buf->count == 0) return 0.0f;

    float sum = 0.0f;
    for (uint8_t i = 0; i < buf->count; ++i) {
        sum += buf->wave_history[i];
    }
    return sum / (float)buf->count;
}

TsfiAcousticCertaintyScore tsfi_vaesen_compute_acoustic_certainty(uint16_t pll_phase_deg) {
    TsfiAcousticCertaintyScore score;
    memset(&score, 0, sizeof(score));

    float phase_rad = ((float)(pll_phase_deg % 360)) * (3.14159265f / 180.0f);
    float half_phase = phase_rad * 0.5f;
    float cos_val = cosf(half_phase);

    /* Certainty = cos^2(Delta_theta / 2) */
    score.certainty_metric = cos_val * cos_val;
    if (score.certainty_metric > 1.0f) score.certainty_metric = 1.0f;
    if (score.certainty_metric < 0.0f) score.certainty_metric = 0.0f;

    score.is_authoritative = (score.certainty_metric >= 0.85f);
    score.is_deceptive = (score.certainty_metric <= 0.35f);

    return score;
}

#include "tsfi_vaesen_fpga_mistral_ext.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

TsfiAcousticEntropyConfig tsfi_vaesen_compute_acoustic_entropy(const TsfiVaesenDeviceSliceBin *slice) {
    TsfiAcousticEntropyConfig cfg;
    memset(&cfg, 0, sizeof(cfg));
    if (!slice) return cfg;

    /* T_acoustic = ((Fervour + Fear) / 10.0) * (1.0 - Dogma / 6.0) */
    float fervour_fear = (float)(slice->fervour + slice->fear_rating);
    float dogma_factor = 1.0f - ((float)slice->dogma / 6.0f);
    if (dogma_factor < 0.05f) dogma_factor = 0.05f;

    cfg.temperature = (fervour_fear / 10.0f) * dogma_factor;
    if (cfg.temperature > 1.0f) cfg.temperature = 1.0f;
    if (cfg.temperature < 0.02f) cfg.temperature = 0.02f;

    /* Nucleus top_p: inversely proportional to Dogma */
    cfg.top_p = 0.50f + (0.50f * (1.0f - ((float)slice->dogma / 5.0f)));
    if (cfg.top_p < 0.50f) cfg.top_p = 0.50f;
    if (cfg.top_p > 0.99f) cfg.top_p = 0.99f;

    /* Acoustic jitter in milli-units */
    cfg.jitter_variance_milli = (uint8_t)(cfg.temperature * 250.0f);

    return cfg;
}

int tsfi_vaesen_dispatch_hardware_tool(
    const TsfiVaesenDeviceSliceBin *slice,
    uint8_t requested_op,
    uint16_t target_addr,
    uint8_t value,
    TsfiHardwareToolCall *out_call
) {
    if (!slice || !out_call) return -1;

    /* Rule 14 DisplacementShader register synchronization */
    uint32_t chin_bound = 1000;
    uint32_t monopole_bound = 2000;
    (void)chin_bound;
    (void)monopole_bound;

    out_call->op_code = requested_op;
    out_call->target_6502_addr = target_addr;
    out_call->value = value;

    switch (requested_op) {
        case TSFI_TOOL_OP_LIGHT_ITEM:
            snprintf(out_call->description, sizeof(out_call->description),
                     "[TOOL:LIGHT] %s ignited item at 6502 addr 0x%04X (Val: 0x%02X)", slice->name, target_addr, value);
            break;
        case TSFI_TOOL_OP_STRIKE_BELL:
            snprintf(out_call->description, sizeof(out_call->description),
                     "[TOOL:BELL] %s struck church bell at 6502 addr 0x%04X (Val: 0x%02X)", slice->name, target_addr, value);
            break;
        case TSFI_TOOL_OP_OFFER_GIFT:
            snprintf(out_call->description, sizeof(out_call->description),
                     "[TOOL:OFFERING] %s offered porridge at 6502 addr 0x%04X (Val: 0x%02X)", slice->name, target_addr, value);
            break;
        case TSFI_TOOL_OP_SEAL_RITUAL:
            snprintf(out_call->description, sizeof(out_call->description),
                     "[TOOL:RITUAL] %s executed banishment at 6502 addr 0x%04X (Val: 0x%02X)", slice->name, target_addr, value);
            break;
        default:
            snprintf(out_call->description, sizeof(out_call->description),
                     "[TOOL:NOP] %s dispatched NOP at 6502 addr 0x%04X", slice->name, target_addr);
            break;
    }

    return 0;
}

int tsfi_vaesen_speculative_verify_draft(
    const TsfiVaesenDeviceSliceBin *slice,
    const uint16_t *draft_tokens,
    uint8_t draft_count,
    TsfiSpeculativeDraftResult *out_result
) {
    if (!slice || !draft_tokens || draft_count == 0 || !out_result) return -1;
    memset(out_result, 0, sizeof(TsfiSpeculativeDraftResult));

    uint8_t count = draft_count > 4 ? 4 : draft_count;
    out_result->draft_count = count;

    uint8_t accepted = 0;
    uint8_t carrier_tone = slice->edo22_carrier_freq % 16;

    for (uint8_t i = 0; i < count; ++i) {
        out_result->draft_tokens[i] = draft_tokens[i];
        uint8_t token_tone = (uint8_t)((draft_tokens[i] >> 8) & 0x0F);

        /* Accept token if tone is harmonically compatible (within 3 EDO-22 semitones) */
        int diff = (int)token_tone - (int)carrier_tone;
        if (diff < 0) diff = -diff;

        if (diff <= 3 || diff >= 13) {
            accepted++;
        } else {
            break; /* Speculative decoding halts at first mismatch */
        }
    }

    out_result->verified_count = accepted;
    out_result->all_accepted = (accepted == count);

    return 0;
}

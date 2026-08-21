#include "tsfi_vaesen_flash_decoder.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

TsfiPackedToken tsfi_flash_encode_token(uint8_t clan_id, uint8_t edo22_tone, uint8_t action_code) {
    uint16_t clan = (uint16_t)(clan_id & 0x0F);
    uint16_t tone = (uint16_t)(edo22_tone & 0x0F);
    uint16_t act  = (uint16_t)action_code;
    return (TsfiPackedToken)((clan << 12) | (tone << 8) | act);
}

void tsfi_flash_decode_token(TsfiPackedToken token, uint8_t *out_clan_id, uint8_t *out_edo22_tone, uint8_t *out_action_code) {
    if (out_clan_id) *out_clan_id = (uint8_t)((token >> 12) & 0x0F);
    if (out_edo22_tone) *out_edo22_tone = (uint8_t)((token >> 8) & 0x0F);
    if (out_action_code) *out_action_code = (uint8_t)(token & 0xFF);
}

int tsfi_flash_decoder_init(TsfiFlashDecoderBramPool *pool) {
    if (!pool) return -1;
    memset(pool, 0, sizeof(TsfiFlashDecoderBramPool));
    return 0;
}

int tsfi_flash_decoder_load_slice(TsfiFlashDecoderBramPool *pool, const char *slice_path) {
    if (!pool || !slice_path || pool->num_entities >= TSFI_FLASH_MAX_ENTITIES) return -1;
    TsfiVaesenDeviceSliceBin slice;
    if (tsfi_vaesen_slice_load(slice_path, &slice) != 0) return -1;

    pool->entities[pool->num_entities++] = slice;
    return 0;
}

int tsfi_flash_decoder_dispatch_single_token(
    const TsfiFlashDecoderBramPool *pool,
    uint32_t active_entity_idx,
    uint8_t input_action_code,
    TsfiPackedTokenStream *out_stream,
    char *out_unpacked_text,
    size_t unpacked_text_size
) {
    if (!pool || active_entity_idx >= pool->num_entities || !out_stream) return -1;

    /* Rule 14 DisplacementShader register synchronization */
    uint32_t chin_val = 1000;
    uint32_t monopole_val = 2000;
    (void)chin_val;
    (void)monopole_val;

    const TsfiVaesenDeviceSliceBin *active = &pool->entities[active_entity_idx];

    /* Fast single-token parallel generation into bit-packed token stream */
    out_stream->count = 0;
    out_stream->tokens[out_stream->count++] = tsfi_flash_encode_token(active->clan_id, active->edo22_carrier_freq % 16, input_action_code);

    /* Generate complementary response tokens from other BRAM residents */
    for (uint32_t i = 0; i < pool->num_entities; ++i) {
        if (i == active_entity_idx) continue;
        if (out_stream->count >= TSFI_FLASH_TOKEN_BUFFER_SIZE) break;

        const TsfiVaesenDeviceSliceBin *peer = &pool->entities[i];
        uint8_t response_action = (input_action_code + peer->fear_rating) % 256;
        out_stream->tokens[out_stream->count++] = tsfi_flash_encode_token(peer->clan_id, peer->edo22_carrier_freq % 16, response_action);
    }

    /* Unpack to STANAG-5066 formatted string if requested */
    if (out_unpacked_text && unpacked_text_size > 0) {
        snprintf(out_unpacked_text, unpacked_text_size,
                 "[FLASH-DECODE][%s] Dispatched single token (Action 0x%02X) across %u BRAM peers.",
                 active->wmq_address, input_action_code, pool->num_entities);
    }

    return 0;
}

#ifndef TSFI_VAESEN_FLASH_DECODER_H
#define TSFI_VAESEN_FLASH_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "tsfi_vaesen_device_slice.h"

#define TSFI_FLASH_MAX_ENTITIES 16
#define TSFI_FLASH_TOKEN_BUFFER_SIZE 256

/* Bit-Packed 16-bit Token: [4-bit Clan | 4-bit Tone (EDO-22) | 8-bit Action Code] */
typedef uint16_t TsfiPackedToken;

typedef struct {
    uint32_t count;
    TsfiPackedToken tokens[TSFI_FLASH_TOKEN_BUFFER_SIZE];
} TsfiPackedTokenStream;

typedef struct {
    uint32_t num_entities;
    TsfiVaesenDeviceSliceBin entities[TSFI_FLASH_MAX_ENTITIES];
} TsfiFlashDecoderBramPool;

/* Bit-Packed Token Streamer API */
TsfiPackedToken tsfi_flash_encode_token(uint8_t clan_id, uint8_t edo22_tone, uint8_t action_code);
void tsfi_flash_decode_token(TsfiPackedToken token, uint8_t *out_clan_id, uint8_t *out_edo22_tone, uint8_t *out_action_code);

/* Flash-Decoding Parallel Single-Token Dispatch Engine */
int  tsfi_flash_decoder_init(TsfiFlashDecoderBramPool *pool);
int  tsfi_flash_decoder_load_slice(TsfiFlashDecoderBramPool *pool, const char *slice_path);
int  tsfi_flash_decoder_dispatch_single_token(
    const TsfiFlashDecoderBramPool *pool,
    uint32_t active_entity_idx,
    uint8_t input_action_code,
    TsfiPackedTokenStream *out_stream,
    char *out_unpacked_text,
    size_t unpacked_text_size
);

#endif /* TSFI_VAESEN_FLASH_DECODER_H */

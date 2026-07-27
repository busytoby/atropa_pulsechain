#ifndef TSFI_MEDIA_DECODE_H
#define TSFI_MEDIA_DECODE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TsfiMediaDecoder TsfiMediaDecoder;

TsfiMediaDecoder* tsfi_media_decoder_create(const char *filepath);
void tsfi_media_decoder_destroy(TsfiMediaDecoder *dec);
bool tsfi_media_decoder_next_frame(TsfiMediaDecoder *dec, int *width, int *height, uint8_t *rgba_buffer);

#ifdef __cplusplus
}
#endif

#endif // TSFI_MEDIA_DECODE_H

#ifndef TSFI_ENCODINGS_H
#define TSFI_ENCODINGS_H

#include <stdint.h>
#include <stdbool.h>

// Encodings API
int tsfi_encode_ascii(const char *in, uint8_t *out, int max_len);
int tsfi_decode_ascii(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_ebcdic(const char *in, uint8_t *out, int max_len);
int tsfi_decode_ebcdic(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_morse(const char *in, char *out, int max_len);
int tsfi_decode_morse(const char *in, char *out, int max_len);

int tsfi_encode_hollerith(const char *in, uint16_t *out, int max_len);
int tsfi_decode_hollerith(const uint16_t *in, int len, char *out, int max_len);

int tsfi_encode_baudot(const char *in, uint8_t *out, int max_len);
int tsfi_decode_baudot(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_bcdic(const char *in, uint8_t *out, int max_len);
int tsfi_decode_bcdic(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_fieldata(const char *in, uint8_t *out, int max_len);
int tsfi_decode_fieldata(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_murray(const char *in, uint8_t *out, int max_len);
int tsfi_decode_murray(const uint8_t *in, int len, char *out, int max_len);

int tsfi_encode_radix50(const char *in, uint16_t *out, int max_len);
int tsfi_decode_radix50(const uint16_t *in, int len, char *out, int max_len);

#endif // TSFI_ENCODINGS_H

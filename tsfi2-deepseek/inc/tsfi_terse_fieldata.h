#ifndef TSFI_TERSE_FIELDATA_H
#define TSFI_TERSE_FIELDATA_H

#include <stdint.h>
#include <stddef.h>

#define FIELDATA_SYMBOLS 64

// Setup probability tracking for 64 FIELDATA symbols
typedef struct {
    uint32_t low_cum[FIELDATA_SYMBOLS + 1];
    uint32_t total_freq;
} FIELDATAProbabilityModel;

// Initialize model with flat or predefined frequencies
void fieldata_model_init(FIELDATAProbabilityModel *model);

// Update model counts adaptively for dynamic arithmetic coding
void fieldata_model_update(FIELDATAProbabilityModel *model, uint8_t sym);

// Encode a stream of 6-bit FIELDATA symbols using arithmetic range coding
int fieldata_terse_compress(
    const uint8_t *input_symbols,
    size_t input_len,
    uint8_t *output_bytes,
    size_t *output_len
);

// Decode an arithmetic range coded byte stream back to 6-bit FIELDATA symbols
int fieldata_terse_decompress(
    const uint8_t *input_bytes,
    size_t input_len,
    uint8_t *output_symbols,
    size_t expected_len
);

#endif // TSFI_TERSE_FIELDATA_H

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
// Pack 6-bit FIELDATA symbols into 36-bit words (stored in lower 36 bits of uint64_t)
void fieldata_pack_36bit(const uint8_t *symbols, size_t sym_len, uint64_t *words, size_t *word_count);

// Unpack 36-bit words back into 6-bit FIELDATA symbols
void fieldata_unpack_36bit(const uint64_t *words, size_t word_count, uint8_t *symbols, size_t *sym_len);

// Encode a 32-bit Unicode codepoint into 6-bit FIELDATA symbols
void fieldata_utf6_encode(uint32_t codepoint, uint8_t *out_symbols, size_t *out_len);

// Decode 6-bit FIELDATA symbols back into a 32-bit Unicode codepoint
uint32_t fieldata_utf6_decode(const uint8_t *symbols, size_t *in_len);

// Load and decompress compiler parser tables dynamically from RDBMS .dat.bin files
int tsfi_compiler_load_tables_from_rdbms(const char *db_path, uint8_t *out_parse_table, size_t *out_len);

#endif // TSFI_TERSE_FIELDATA_H

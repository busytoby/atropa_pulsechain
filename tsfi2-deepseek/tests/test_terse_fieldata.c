#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tsfi_terse_fieldata.h"

int main() {
    printf("=== RUNNING FIELDATA ARITHMETIC CODING (TERSE) TESTS ===\n");

    // Sample 6-bit symbols: "AUNCENT" mapped to 6-bit values
    uint8_t input_symbols[] = { 1, 21, 14, 3, 5, 14, 20, 1, 21, 3, 5, 14, 20 };
    size_t input_len = sizeof(input_symbols);

    uint8_t compressed[256];
    size_t compressed_len = 0;

    // Compress using Arithmetic Range Coding
    int res_comp = fieldata_terse_compress(input_symbols, input_len, compressed, &compressed_len);
    assert(res_comp == 0);
    printf("[INFO] Original symbols length: %lu, Compressed bytes size: %lu\n", input_len, compressed_len);

    // Decompress
    uint8_t decompressed[256];
    memset(decompressed, 0, sizeof(decompressed));
    
    int res_decomp = fieldata_terse_decompress(compressed, compressed_len, decompressed, input_len);
    assert(res_decomp == 0);

    // Verify symbols match exactly
    for (size_t i = 0; i < input_len; i++) {
        assert(decompressed[i] == input_symbols[i]);
    }

    printf("=== ALL FIELDATA ARITHMETIC CODING TESTS PASSED ===\n");
    return 0;
}

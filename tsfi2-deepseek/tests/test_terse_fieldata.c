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
    printf("[PASS] Symbol compression and decompression match exactly\n");

    // 36-bit word packing verification
    uint64_t packed_words[16];
    size_t packed_word_count = 0;
    fieldata_pack_36bit(input_symbols, input_len, packed_words, &packed_word_count);
    assert(packed_word_count > 0);
    printf("[INFO] Packed %lu symbols into %lu 36-bit words\n", input_len, packed_word_count);

    uint8_t unpacked_symbols[256];
    size_t unpacked_len = 0;
    fieldata_unpack_36bit(packed_words, packed_word_count, unpacked_symbols, &unpacked_len);
    assert(unpacked_len >= input_len);

    for (size_t i = 0; i < input_len; i++) {
        assert(unpacked_symbols[i] == input_symbols[i]);
    }
    printf("[PASS] 36-bit word packing and unpacking matches original symbols\n");

    // UTF-6 32-bit Unicode verification
    uint32_t test_codes[] = { 45, 0x1F600, 0x3E800, 0xFFFFFFFF }; // Simple, 18-bit, 24-bit, and full 32-bit values
    for (int i = 0; i < 4; i++) {
        uint8_t utf6_buf[8];
        size_t utf6_len = 0;
        fieldata_utf6_encode(test_codes[i], utf6_buf, &utf6_len);
        assert(utf6_len > 0);

        size_t decoded_len = 0;
        uint32_t decoded_code = fieldata_utf6_decode(utf6_buf, &decoded_len);
        assert(decoded_len == utf6_len);
        assert(decoded_code == test_codes[i]);
    }
    printf("[PASS] UTF-6 Variable-Length Unicode Encoder/Decoder validates up to 32-bit codepoints\n");

    printf("=== ALL FIELDATA ARITHMETIC CODING TESTS PASSED ===\n");
    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "../src/tsfi_computel_dtmf_encoder.c"
#include "../src/tsfi_computel_dtmf_decoder.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel DTMF Encoder/Decoder loopback test suite...\n");

    const char test_keys[] = {'3', 'B', '#', '0', '7', 'A'};
    int num_keys = sizeof(test_keys) / sizeof(test_keys[0]);

    float buffer[GOERTZEL_N];

    for (int idx = 0; idx < num_keys; idx++) {
        char target_key = test_keys[idx];
        
        // Encode Key to PCM
        bool encode_ok = generate_dtmf_tone(target_key, buffer, GOERTZEL_N);
        assert(encode_ok == true);

        // Decode PCM to Key
        char decoded_key = decode_dtmf_block(buffer, GOERTZEL_N);
        printf("[TEST] Loopback check: Target = %c | Decoded = %c\n", target_key, decoded_key);
        assert(decoded_key == target_key);
    }

    printf("[SUCCESS] All Computel DTMF loopback synthesis and decoding tests passed successfully.\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <assert.h>

#define TILE_SIZE 64 // 8x8 block size

// Standard JPEG Quantization Matrix (Luminance)
static const uint8_t quant_matrix[TILE_SIZE] = {
    16, 11, 10, 16, 24, 40, 51, 61,
    12, 12, 14, 19, 26, 58, 60, 55,
    14, 13, 16, 24, 40, 57, 69, 56,
    14, 17, 22, 29, 51, 87, 80, 62,
    18, 22, 37, 56, 68, 109, 103, 77,
    24, 35, 55, 64, 81, 104, 113, 92,
    49, 64, 78, 87, 103, 121, 120, 101,
    72, 92, 95, 98, 112, 100, 103, 99
};

// Simplified 1D IDCT helper
void idct_1d(const float *in, float *out) {
    for (int i = 0; i < 8; i++) {
        float sum = 0.0f;
        for (int j = 0; j < 8; j++) {
            float c = (j == 0) ? (1.0f / sqrtf(2.0f)) : 1.0f;
            sum += c * in[j] * cosf((2.0f * i + 1.0f) * j * M_PI / 16.0f);
        }
        out[i] = 0.5f * sum;
    }
}

// 2D IDCT Decompression Loop
void jpeg_decompress_block(const int16_t *coeff, uint8_t *pixels) {
    float temp[TILE_SIZE];
    float block[TILE_SIZE];
    
    // 1. Dequantization
    for (int i = 0; i < TILE_SIZE; i++) {
        block[i] = (float)(coeff[i] * quant_matrix[i]);
    }
    
    // 2. Perform 2D IDCT: 1D IDCT on rows
    for (int r = 0; r < 8; r++) {
        idct_1d(&block[r * 8], &temp[r * 8]);
    }
    
    // 3. 1D IDCT on columns
    for (int c = 0; c < 8; c++) {
        float col_in[8];
        float col_out[8];
        for (int r = 0; r < 8; r++) {
            col_in[r] = temp[r * 8 + c];
        }
        idct_1d(col_in, col_out);
        for (int r = 0; r < 8; r++) {
            // Level shift +128 and clamp to 0-255
            float val = col_out[r] + 128.0f;
            if (val < 0.0f) val = 0.0f;
            if (val > 255.0f) val = 255.0f;
            pixels[r * 8 + c] = (uint8_t)val;
        }
    }
}

int main() {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("=== TSFi Auncient XPL JPEG DCT Decompression Test ===\n");
    
    // Mock JPEG Coefficient block: DC coefficient = 10, all AC coefficients = 0
    // This represents a flat solid color block
    int16_t coeff[TILE_SIZE];
    memset(coeff, 0, sizeof(coeff));
    coeff[0] = 10; // DC Coefficient
    
    uint8_t pixels[TILE_SIZE];
    memset(pixels, 0, sizeof(pixels));
    
    // Decompress the block
    jpeg_decompress_block(coeff, pixels);
    
    // Verify reconstructed DC value:
    // Dequantized DC = 10 * 16 = 160.
    // 2D IDCT of DC component = 160 / 8 = 20.
    // Level shift (+128) = 148.
    // All reconstructed pixels should equal 148.
    printf("[DCT] Reconstructed DC Pixel value: %u (Expected: 148)\n", pixels[0]);
    assert(pixels[0] == 148);
    assert(pixels[63] == 148);
    
    printf("[DCT] Success: Inverse Discrete Cosine Transform reconstructed pixel vectors correctly.\n");
    printf("\n=== ALL JPEG DCT DECOMPRESSION TESTS PASSED ===\n");
    return 0;
}

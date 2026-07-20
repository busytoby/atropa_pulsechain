#include <math.h>
#include <string.h>
#include <stdlib.h>
#include "tsfi_parc_tknoll.h"

void tsfi_parc_tknoll_dither_floyd_steinberg(const uint32_t *src, uint16_t *dest_alto, int width, int height) {
    if (!src || !dest_alto || width < 606 || height < 808) return;

    // Allocate temporary grayscale error buffer
    float *errors = calloc(606 * 808, sizeof(float));
    if (!errors) return;

    // Convert source to grayscale in error buffer
    for (int y = 0; y < 808; y++) {
        for (int x = 0; x < 606; x++) {
            uint32_t pix = src[y * width + x];
            float r = (float)((pix >> 16) & 0xFF);
            float g = (float)((pix >> 8) & 0xFF);
            float b = (float)(pix & 0xFF);
            errors[y * 606 + x] = 0.299f * r + 0.587f * g + 0.114f * b;
        }
    }

    // Clear dest Alto display memory (38 words per line, 808 lines)
    memset(dest_alto, 0, 38 * 808 * sizeof(uint16_t));

    // Floyd-Steinberg Error Diffusion
    for (int y = 0; y < 808; y++) {
        for (int x = 0; x < 606; x++) {
            float old_val = errors[y * 606 + x];
            float new_val = old_val > 127.5f ? 255.0f : 0.0f;
            float err = old_val - new_val;

            // Set pixel bit in 16-bit word layout if active
            if (new_val > 0.0f) {
                int word_idx = y * 38 + (x / 16);
                int bit_idx = 15 - (x % 16);
                dest_alto[word_idx] |= (1 << bit_idx);
            }

            // Distribute error to neighboring pixels
            if (x + 1 < 606) {
                errors[y * 606 + (x + 1)] += err * (7.0f / 16.0f);
            }
            if (y + 1 < 808) {
                if (x - 1 >= 0) {
                    errors[(y + 1) * 606 + (x - 1)] += err * (3.0f / 16.0f);
                }
                errors[(y + 1) * 606 + x] += err * (5.0f / 16.0f);
                if (x + 1 < 606) {
                    errors[(y + 1) * 606 + (x + 1)] += err * (1.0f / 16.0f);
                }
            }
        }
    }

    free(errors);
}

void tsfi_parc_tknoll_levels_adjustment(uint32_t *pixels, int width, int height, int input_min, int input_max, float gamma) {
    if (!pixels || width <= 0 || height <= 0 || input_min >= input_max || gamma <= 0.0f) return;

    // Build pre-computed levels mapping LUT
    uint8_t lut[256];
    for (int i = 0; i < 256; i++) {
        if (i <= input_min) {
            lut[i] = 0;
        } else if (i >= input_max) {
            lut[i] = 255;
        } else {
            float normalized = (float)(i - input_min) / (float)(input_max - input_min);
            float output = powf(normalized, 1.0f / gamma) * 255.0f;
            lut[i] = (uint8_t)(output > 255.0f ? 255.0f : (output < 0.0f ? 0.0f : output));
        }
    }

    // Apply LUT mapping to frame buffer pixels
    for (int i = 0; i < width * height; i++) {
        uint32_t pix = pixels[i];
        uint8_t r = lut[(pix >> 16) & 0xFF];
        uint8_t g = lut[(pix >> 8) & 0xFF];
        uint8_t b = lut[pix & 0xFF];
        pixels[i] = (pix & 0xFF000000) | (r << 16) | (g << 8) | b;
    }
}

void tsfi_parc_tknoll_histogram(const uint32_t *pixels, int width, int height, uint32_t *histogram_out) {
    if (!pixels || !histogram_out || width <= 0 || height <= 0) return;

    memset(histogram_out, 0, 256 * sizeof(uint32_t));

    for (int i = 0; i < width * height; i++) {
        uint32_t pix = pixels[i];
        float r = (float)((pix >> 16) & 0xFF);
        float g = (float)((pix >> 8) & 0xFF);
        float b = (float)(pix & 0xFF);

        // Convert to grayscale intensity to select channel index
        int intensity = (int)(0.299f * r + 0.587f * g + 0.114f * b);
        if (intensity >= 0 && intensity < 256) {
            histogram_out[intensity]++;
        }
    }
}

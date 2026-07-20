#include <math.h>
#include "tsfi_parc_knoll.h"

void tsfi_parc_knoll_lens_flare(uint32_t *pixels, int width, int height, int cx, int cy, float intensity) {
    if (!pixels || width <= 0 || height <= 0) return;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float dx = (float)(x - cx);
            float dy = (float)(y - cy);
            float dist = sqrtf(dx * dx + dy * dy);

            // 1. Central glare flare core
            float core = 255.0f * (1.0f / (1.0f + dist * 0.05f)) * intensity;

            // 2. Halo ring at radius 80 pixels
            float ring = 0.0f;
            if (dist > 75.0f && dist < 85.0f) {
                ring = 120.0f * (1.0f - fabsf(dist - 80.0f) / 5.0f) * intensity;
            }

            // Mix colors
            uint32_t orig = pixels[y * width + x];
            int r = (int)((orig >> 16) & 0xFF) + (int)core + (int)(ring * 0.8f);
            int g = (int)((orig >> 8) & 0xFF) + (int)core + (int)(ring * 0.6f);
            int b = (int)(orig & 0xFF) + (int)core + (int)ring;

            r = r > 255 ? 255 : r;
            g = g > 255 ? 255 : g;
            b = b > 255 ? 255 : b;

            pixels[y * width + x] = (orig & 0xFF000000) | (r << 16) | (g << 8) | b;
        }
    }
}

void tsfi_parc_knoll_gaussian_blur(const uint32_t *src, uint32_t *dst, int width, int height, float sigma) {
    if (!src || !dst || width <= 0 || height <= 0) return;
    (void)sigma;

    // Fast 3x3 convolution blur approximation
    for (int y = 1; y < height - 1; y++) {
        for (int x = 1; x < width - 1; x++) {
            uint32_t sum_r = 0, sum_g = 0, sum_b = 0;

            for (int ky = -1; ky <= 1; ky++) {
                for (int kx = -1; kx <= 1; kx++) {
                    uint32_t pix = src[(y + ky) * width + (x + kx)];
                    sum_r += (pix >> 16) & 0xFF;
                    sum_g += (pix >> 8) & 0xFF;
                    sum_b += pix & 0xFF;
                }
            }

            uint32_t avg_r = sum_r / 9;
            uint32_t avg_g = sum_g / 9;
            uint32_t avg_b = sum_b / 9;

            dst[y * width + x] = (src[y * width + x] & 0xFF000000) | (avg_r << 16) | (avg_g << 8) | avg_b;
        }
    }
}

void tsfi_parc_knoll_gray_lut(const uint32_t *src, uint32_t *dst, int width, int height) {
    if (!src || !dst || width <= 0 || height <= 0) return;

    for (int i = 0; i < width * height; i++) {
        uint32_t pix = src[i];
        float r = (float)((pix >> 16) & 0xFF);
        float g = (float)((pix >> 8) & 0xFF);
        float b = (float)(pix & 0xFF);

        // Standard NTSC Grayscale coefficients: 0.299R + 0.587G + 0.114B
        uint32_t gray = (uint32_t)(0.299f * r + 0.587f * g + 0.114f * b);
        gray = gray > 255 ? 255 : gray;

        dst[i] = (pix & 0xFF000000) | (gray << 16) | (gray << 8) | gray;
    }
}

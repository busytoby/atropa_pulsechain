#include "tsfi_byte_issue23.h"
#include <string.h>

// Speech Phoneme Detector
void tsfi_speech_init(TsfiSpeechDetector *det) {
    if (!det) return;
    det->f1_freq = 0.0f;
    det->f2_freq = 0.0f;
    det->f3_freq = 0.0f;
    det->threshold = 0.1f;
}

const char* tsfi_speech_classify(TsfiSpeechDetector *det, const float *spectrum_data, int size) {
    if (!det || !spectrum_data || size < 60) return "unknown";

    // Simple peak finders for F1 and F2
    // F1 typically lies in 200 - 900 Hz range
    // F2 typically lies in 600 - 2500 Hz range
    // Let's assume each index in spectrum represents 50Hz.
    // Index 0 to 18 covers 0 - 900Hz.
    // Index 12 to 50 covers 600 - 2500Hz.
    int f1_bin = 0;
    float f1_max = 0.0f;
    for (int i = 4; i < 18; i++) { // 200Hz - 900Hz
        if (spectrum_data[i] > f1_max) {
            f1_max = spectrum_data[i];
            f1_bin = i;
        }
    }

    int f2_bin = 0;
    float f2_max = 0.0f;
    for (int i = 18; i < 50; i++) { // 900Hz - 2500Hz
        if (spectrum_data[i] > f2_max) {
            f2_max = spectrum_data[i];
            f2_bin = i;
        }
    }

    if (f1_max < det->threshold || f2_max < det->threshold) {
        return "silent";
    }

    float f1 = (float)f1_bin * 50.0f;
    float f2 = (float)f2_bin * 50.0f;

    det->f1_freq = f1;
    det->f2_freq = f2;

    // Vowel classification rules based on formant regions
    if (f1 >= 250.0f && f1 <= 400.0f && f2 >= 2000.0f && f2 <= 2400.0f) {
        return "/i/"; // vowel in "meet"
    }
    if (f1 >= 650.0f && f1 <= 850.0f && f2 >= 950.0f && f2 <= 1300.0f) {
        return "/a/"; // vowel in "father"
    }
    if (f1 >= 250.0f && f1 <= 400.0f && f2 >= 700.0f && f2 <= 950.0f) {
        return "/u/"; // vowel in "boot"
    }

    return "unknown";
}

// Hall Multiplier
void tsfi_hall_mult_init(TsfiHallMultiplier *mult, uint8_t a, uint8_t b) {
    if (!mult) return;
    mult->accumulator = 0;
    mult->multiplicand = a;
    mult->multiplier = b;
    mult->step = 0;
}

bool tsfi_hall_mult_step(TsfiHallMultiplier *mult) {
    if (!mult || mult->step >= 8) return false;

    if (mult->multiplier & 1) {
        mult->accumulator += ((uint16_t)mult->multiplicand << mult->step);
    }
    mult->multiplier >>= 1;
    mult->step++;
    return true;
}

// Maurer Hamming (7,4) ECC implementation
// We map the 4 data bits: d1, d2, d3, d4 to positions 3, 5, 6, 7.
// Parity bits p1, p2, p3 to positions 1, 2, 4.
uint8_t tsfi_maurer_encode(uint8_t data_4bit) {
    uint8_t d1 = (data_4bit >> 3) & 1;
    uint8_t d2 = (data_4bit >> 2) & 1;
    uint8_t d3 = (data_4bit >> 1) & 1;
    uint8_t d4 = (data_4bit >> 0) & 1;

    uint8_t p1 = d1 ^ d2 ^ d4;
    uint8_t p2 = d1 ^ d3 ^ d4;
    uint8_t p3 = d2 ^ d3 ^ d4;

    // Construct 7-bit codeword: p1, p2, d1, p3, d2, d3, d4
    uint8_t code = (p1 << 6) | (p2 << 5) | (d1 << 4) | (p3 << 3) | (d2 << 2) | (d3 << 1) | d4;
    return code;
}

uint8_t tsfi_maurer_decode(uint8_t code_7bit, bool *corrected) {
    if (corrected) *corrected = false;

    uint8_t p1 = (code_7bit >> 6) & 1;
    uint8_t p2 = (code_7bit >> 5) & 1;
    uint8_t d1 = (code_7bit >> 4) & 1;
    uint8_t p3 = (code_7bit >> 3) & 1;
    uint8_t d2 = (code_7bit >> 2) & 1;
    uint8_t d3 = (code_7bit >> 1) & 1;
    uint8_t d4 = code_7bit & 1;

    // Compute syndromes
    uint8_t s1 = p1 ^ d1 ^ d2 ^ d4;
    uint8_t s2 = p2 ^ d1 ^ d3 ^ d4;
    uint8_t s3 = p3 ^ d2 ^ d3 ^ d4;

    uint8_t error_pos = (s3 << 2) | (s2 << 1) | s1;

    if (error_pos != 0) {
        if (corrected) *corrected = true;
        // Flip the corrupted bit at error_pos (1-indexed from Left: MSB corresponds to pos 1)
        code_7bit ^= (1 << (7 - error_pos));
        
        // Re-extract corrected bits
        d1 = (code_7bit >> 4) & 1;
        d2 = (code_7bit >> 2) & 1;
        d3 = (code_7bit >> 1) & 1;
        d4 = code_7bit & 1;
    }

    // Reconstruct 4-bit data
    uint8_t decoded = (d1 << 3) | (d2 << 2) | (d3 << 1) | d4;
    return decoded;
}

#ifndef TSFI_BYTE_ISSUE23_H
#define TSFI_BYTE_ISSUE23_H

#include <stdint.h>
#include <stdbool.h>

// Boddie Formant Detector
typedef struct {
    float f1_freq;
    float f2_freq;
    float f3_freq;
    float threshold;
} TsfiSpeechDetector;

// Hall Shift-and-Add Multiplier
typedef struct {
    uint16_t accumulator;
    uint8_t multiplicand;
    uint8_t multiplier;
    int step;
} TsfiHallMultiplier;

// --- API Declarations ---

// Speech Phoneme Detector
void tsfi_speech_init(TsfiSpeechDetector *det);
const char* tsfi_speech_classify(TsfiSpeechDetector *det, const float *spectrum_data, int size);

// Hall Shift-and-Add Multiplier
void tsfi_hall_mult_init(TsfiHallMultiplier *mult, uint8_t a, uint8_t b);
bool tsfi_hall_mult_step(TsfiHallMultiplier *mult);

// Maurer Hamming (7,4) ECC
uint8_t tsfi_maurer_encode(uint8_t data_4bit);
uint8_t tsfi_maurer_decode(uint8_t code_7bit, bool *corrected);

#endif // TSFI_BYTE_ISSUE23_H

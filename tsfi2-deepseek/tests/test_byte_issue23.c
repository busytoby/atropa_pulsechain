#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "tsfi_byte_issue23.h"

int main() {
    printf("=== TSFi2 & BYTE Magazine Issue #23 Simulation Test ===\n");

    // 1. Verify Speech Phoneme Classifier
    printf("[SPEECH] Testing speech formant classification...\n");
    TsfiSpeechDetector det;
    tsfi_speech_init(&det);

    float spectrum[64];
    memset(spectrum, 0, sizeof(spectrum));

    // Simulate vowel /i/ (peaks around 300Hz and 2200Hz)
    // Recall 1 bin = 50Hz. So 300Hz is bin 6. 2200Hz is bin 44.
    spectrum[6] = 0.8f;
    spectrum[44] = 0.6f;
    const char *ph_i = tsfi_speech_classify(&det, spectrum, 64);
    printf("[SPEECH]   Formant Peaks -> F1: %fHz, F2: %fHz -> Phoneme: %s (Expected: /i/)\n",
           det.f1_freq, det.f2_freq, ph_i);
    assert(strcmp(ph_i, "/i/") == 0);

    // Simulate vowel /a/ (peaks around 750Hz = bin 15 and 1100Hz = bin 22)
    memset(spectrum, 0, sizeof(spectrum));
    spectrum[15] = 0.9f;
    spectrum[22] = 0.7f;
    const char *ph_a = tsfi_speech_classify(&det, spectrum, 64);
    printf("[SPEECH]   Formant Peaks -> F1: %fHz, F2: %fHz -> Phoneme: %s (Expected: /a/)\n",
           det.f1_freq, det.f2_freq, ph_a);
    assert(strcmp(ph_a, "/a/") == 0);

    // 2. Verify Hall Shift-and-Add Multiplier
    printf("[MULTIPLIER] Testing hardware step-by-step multiplier (12 * 7)...\n");
    TsfiHallMultiplier mult;
    tsfi_hall_mult_init(&mult, 12, 7);

    int step_count = 0;
    while (tsfi_hall_mult_step(&mult)) {
        step_count++;
    }
    printf("[MULTIPLIER]   Completed in %d steps, Accumulator: %d (Expected: 84)\n",
           step_count, mult.accumulator);
    assert(step_count == 8);
    assert(mult.accumulator == 84);

    // 3. Verify Maurer Hamming (7,4) ECC
    printf("[ECC] Testing Hamming (7,4) error detection and correction...\n");
    uint8_t original_data = 0x09; // binary 1001
    uint8_t codeword = tsfi_maurer_encode(original_data);

    // Inject 1-bit error at position 5 (bit 5 is index 2 from right in 7-bit, i.e., d2)
    // 7 - 5 = 2. Codeword XOR (1 << 2)
    uint8_t corrupted_codeword = codeword ^ (1 << 2);

    bool corrected = false;
    uint8_t decoded = tsfi_maurer_decode(corrupted_codeword, &corrected);

    printf("[ECC]   Original: 0x%02X, Codeword: 0x%02X, Corrupted: 0x%02X, Decoded: 0x%02X (Corrected: %d)\n",
           original_data, codeword, corrupted_codeword, decoded, corrected);
    
    assert(corrected == true);
    assert(decoded == original_data);
    printf("[ECC]   Hamming single-bit correction verified successfully.\n");

    // Memory sanitation checks
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    extern void lau_assert_zero_unsealed_leaks(const char *context, void *teardown_ptr);
    lau_assert_zero_unsealed_leaks("test_byte_issue23", NULL);

    printf("[PASS] BYTE Magazine Issue #23 simulation verified.\n");
    return 0;
}

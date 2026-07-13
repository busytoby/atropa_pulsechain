#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// WAV Header struct
#pragma pack(push, 1)
typedef struct {
    char     riff[4];        // "RIFF"
    uint32_t overall_size;   // file size - 8
    char     wave[4];        // "WAVE"
    char     fmt_chunk_marker[4]; // "fmt "
    uint32_t length_of_fmt;  // 16 for PCM
    uint16_t format_type;    // 1 for PCM
    uint16_t channels;       // 1 for Mono
    uint32_t sample_rate;    // 44100
    uint32_t byterate;       // sample_rate * channels * (bits_per_sample/8)
    uint16_t block_align;    // channels * (bits_per_sample/8)
    uint16_t bits_per_sample; // 16
    char     data_chunk_header[4]; // "data"
    uint32_t data_size;      // num_samples * channels * (bits_per_sample/8)
} WavHeader;
#pragma pack(pop)

int main(void) {
    uint32_t sample_rate = 44100;
    double duration_per_note = 0.5; // 0.5 seconds per note
    double score[4] = { 440.0, 523.25, 659.25, 783.99 }; // Am7 arpeggio
    uint32_t num_samples_per_note = (uint32_t)(sample_rate * duration_per_note);
    uint32_t total_samples = num_samples_per_note * 4;
    
    WavHeader header;
    memcpy(header.riff, "RIFF", 4);
    header.overall_size = sizeof(WavHeader) + total_samples * 2 - 8;
    memcpy(header.wave, "WAVE", 4);
    memcpy(header.fmt_chunk_marker, "fmt ", 4);
    header.length_of_fmt = 16;
    header.format_type = 1;
    header.channels = 1;
    header.sample_rate = sample_rate;
    header.bits_per_sample = 16;
    header.byterate = sample_rate * 1 * 2;
    header.block_align = 1 * 2;
    memcpy(header.data_chunk_header, "data", 4);
    header.data_size = total_samples * 2;

    FILE *f = fopen("tests/synth_score.wav", "wb");
    if (!f) {
        perror("Failed to create WAV file");
        return 1;
    }

    fwrite(&header, sizeof(WavHeader), 1, f);

    for (int note = 0; note < 4; note++) {
        double freq = score[note];
        for (uint32_t i = 0; i < num_samples_per_note; i++) {
            double t = (double)i / sample_rate;
            double raw_val = sin(2.0 * M_PI * freq * t);
            
            // Fade-in/out to avoid clicking
            double envelope = 1.0;
            if (i < 500) {
                envelope = (double)i / 500.0;
            } else if (i > num_samples_per_note - 500) {
                envelope = (double)(num_samples_per_note - i) / 500.0;
            }
            
            int16_t sample = (int16_t)(raw_val * envelope * 16384.0);
            fwrite(&sample, sizeof(int16_t), 1, f);
        }
    }

    fclose(f);
    printf("Successfully generated tests/synth_score.wav\n");
    return 0;
}

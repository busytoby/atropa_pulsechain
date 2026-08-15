#ifndef TSFI_BIONIKA_SYNTH_H
#define TSFI_BIONIKA_SYNTH_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define BIONIKA_SAMPLE_RATE 44100
#define BIONIKA_DURATION_SEC 90
#define BIONIKA_TOTAL_SAMPLES (BIONIKA_SAMPLE_RATE * BIONIKA_DURATION_SEC)

typedef struct {
    float time_sec;
    int track_idx;
    float start_freq;
    float end_freq;
    float gain;
    float duration_sec;
} TsfiBioEvent;

typedef struct {
    int event_count;
    TsfiBioEvent events[128];
} TsfiBioScore;

bool tsfi_bio_load_score(TsfiBioScore *score, const char *filepath);
size_t tsfi_bio_synthesize_pcm16(const TsfiBioScore *score, int16_t *out_pcm, size_t total_frames);
bool tsfi_bio_export_wav(const char *filepath, const int16_t *pcm_data, size_t total_frames);

#endif // TSFI_BIONIKA_SYNTH_H

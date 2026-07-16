#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 8000

// Simple 2nd-order State Variable Filter (SVF) for resonant lead and snare filtering
typedef struct {
    double f, q, low, band, high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
    r->q = 1.0 / Q;
    r->low = 0.0;
    r->band = 0.0;
    r->high = 0.0;
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band;
}

// Frequency resolver matching musicNotationParser.yul rules
static double parse_note(const char *note) {
    if (strcmp(note, "REST") == 0 || strcmp(note, "rest") == 0) return 0.0;
    char char0 = note[0];
    int idx = 0;
    switch (char0) {
        case 'C': case 'c': idx = 0; break;
        case 'D': case 'd': idx = 2; break;
        case 'E': case 'e': idx = 4; break;
        case 'F': case 'f': idx = 5; break;
        case 'G': case 'g': idx = 7; break;
        case 'A': case 'a': idx = 9; break;
        case 'B': case 'b': idx = 11; break;
        default: return 0.0;
    }
    char char1 = note[1];
    int octave_char = char1;
    if (char1 == '#' || char1 == 'b') {
        octave_char = note[2];
        if (char1 == '#') idx += 1;
        else idx -= 1;
    }
    int octave = octave_char - '0';
    double freq = 0.0;
    switch (idx) {
        case 0:  freq = 261.63; break; // C4
        case 1:  freq = 277.18; break;
        case 2:  freq = 293.66; break;
        case 3:  freq = 311.13; break;
        case 4:  freq = 329.63; break;
        case 5:  freq = 349.23; break;
        case 6:  freq = 369.99; break;
        case 7:  freq = 392.00; break;
        case 8:  freq = 415.30; break;
        case 9:  freq = 440.00; break;
        case 10: freq = 466.16; break; // Bb4
        case 11: freq = 493.88; break;
        default: return 0.0;
    }
    if (octave > 4) {
        for (int i = 4; i < octave; i++) freq *= 2.0;
    } else if (octave < 4) {
        for (int i = octave; i < 4; i++) freq /= 2.0;
    }
    return freq;
}

static void write_wav_header(FILE *fp, uint32_t data_size) {
    uint32_t chunk_size = 36 + data_size;
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1;
    uint16_t num_channels = 1;
    uint32_t sample_rate = SAMPLE_RATE;
    uint32_t byte_rate = SAMPLE_RATE * 2;
    uint16_t block_align = 2;
    uint16_t bits_per_sample = 16;
    fwrite("RIFF", 1, 4, fp);
    fwrite(&chunk_size, 4, 1, fp);
    fwrite("WAVE", 1, 4, fp);
    fwrite("fmt ", 1, 4, fp);
    fwrite(&subchunk1_size, 4, 1, fp);
    fwrite(&audio_format, 2, 1, fp);
    fwrite(&num_channels, 2, 1, fp);
    fwrite(&sample_rate, 4, 1, fp);
    fwrite(&byte_rate, 4, 1, fp);
    fwrite(&block_align, 2, 1, fp);
    fwrite(&bits_per_sample, 2, 1, fp);
    fwrite("data", 1, 4, fp);
    fwrite(&data_size, 4, 1, fp);
}

int main(int argc, char *argv[]) {
    double pitch_scale = 1.0;
    if (argc > 1) {
        pitch_scale = atof(argv[1]);
        if (pitch_scale <= 0.0) pitch_scale = 1.0;
    }
    printf("[BIONIKA] Synthesizing full arrangement (7 instruments) for Eye of the Tiger (Pitch scale: %.2f)...\n", pitch_scale);
    fflush(stdout);

    // Intro Riff sequences matching eye_of_the_tiger.bio
    const char *bass_seq[] = {
        "C2", "C2", "C2", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "C2", "C2", "C2",
        "C2", "C2", "Bb1", "C2", "C2", "Ab1", "Bb1", "C2"
    };
    const char *lead_seq[] = {
        "C4", "C4", "C4", "C4", "C4", "C4", "C4", "C4",
        "C4", "C4", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "REST", "REST", "REST",
        "REST", "REST", "Bb3", "C4", "REST", "Ab3", "Bb3", "C4"
    };
    const char *arp_seq[] = {
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3",
        "C3", "Eb3", "G3", "C4", "Eb3", "G3", "C4", "Eb3"
    };
    int kick_seq[] = {
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
        1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0
    };
    int snare_seq[] = {
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
        0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0
    };
    int hh_seq[] = {
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0,
        1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0
    };
    int clap_seq[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0,
        1, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 1
    };

    int num_steps = 32;
    int step_duration_ms = 136;
    int samples_per_step = (SAMPLE_RATE * step_duration_ms) / 1000;
    int total_samples = samples_per_step * num_steps;

    int16_t *buffer = malloc(total_samples * sizeof(int16_t));
    memset(buffer, 0, total_samples * sizeof(int16_t));

    // Phase oscillators
    double lead_phase = 0.0;
    double bass_phase = 0.0;
    double arp_phase = 0.0;
    
    // Drum envelopes
    double kick_env = 0.0;
    double kick_pitch = 150.0;
    double snare_env = 0.0;
    double hihat_env = 0.0;
    double clap_env = 0.0;
    
    Resonator snare_res, lead_res;
    resonator_init(&snare_res, 1000.0, 4.0);
    resonator_init(&lead_res, 800.0, 2.0);

    unsigned int seed = 0x12345678;

    for (int step = 0; step < num_steps; step++) {
        // Trigger triggers at step boundary
        if (kick_seq[step]) { kick_env = 1.0; kick_pitch = 150.0; }
        if (snare_seq[step]) snare_env = 1.0;
        if (hh_seq[step]) hihat_env = 1.0;
        if (clap_seq[step]) clap_env = 1.0;

        double lead_freq = parse_note(lead_seq[step]) * pitch_scale;
        double bass_freq = parse_note(bass_seq[step]) * pitch_scale;
        double arp_freq = parse_note(arp_seq[step]) * pitch_scale;

        for (int s = 0; s < samples_per_step; s++) {
            int idx = step * samples_per_step + s;

            // 1. Kick Drum
            double kick_out = 0.0;
            if (kick_env > 0.0) {
                kick_out = sin(2.0 * M_PI * kick_pitch * s / SAMPLE_RATE) * kick_env;
                kick_pitch *= 0.995;
                kick_env *= 0.992;
            }

            // 2. Snare Drum (Resonant white noise)
            double snare_out = 0.0;
            if (snare_env > 0.0) {
                seed = seed * 1103515245 + 12345;
                double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                snare_out = resonator_tick(&snare_res, noise) * snare_env;
                snare_env *= 0.995;
            }

            // 3. Hihat (High-passed white noise)
            double hh_out = 0.0;
            if (hihat_env > 0.0) {
                seed = seed * 1103515245 + 12345;
                double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                hh_out = noise * hihat_env * 0.5;
                hihat_env *= 0.985;
            }

            // 4. Clap (Multi-burst noise)
            double clap_out = 0.0;
            if (clap_env > 0.0) {
                seed = seed * 1103515245 + 12345;
                double noise = ((double)(seed & 0xFFFF) / 65535.0) - 0.5;
                clap_out = noise * clap_env;
                clap_env *= 0.991;
            }

            // 5. Lead Synth (Bandpassed sawtooth wave)
            double lead_out = 0.0;
            if (lead_freq > 0.0) {
                double raw_saw = (lead_phase / M_PI) - 1.0;
                lead_out = resonator_tick(&lead_res, raw_saw) * 0.3;
                lead_phase += (2.0 * M_PI * lead_freq) / SAMPLE_RATE;
                if (lead_phase >= 2.0 * M_PI) lead_phase -= 2.0 * M_PI;
            }

            // 6. Bass Synth (Saturated square wave)
            double bass_out = 0.0;
            if (bass_freq > 0.0) {
                bass_out = (sin(bass_phase) >= 0.0) ? 0.25 : -0.25;
                bass_phase += (2.0 * M_PI * bass_freq) / SAMPLE_RATE;
                if (bass_phase >= 2.0 * M_PI) bass_phase -= 2.0 * M_PI;
            }

            // 7. Arpeggiator (High triangle wave)
            double arp_out = 0.0;
            if (arp_freq > 0.0) {
                double raw_tri = (arp_phase < M_PI) ? (arp_phase / M_PI * 2.0 - 1.0) : (3.0 - arp_phase / M_PI * 2.0);
                arp_out = raw_tri * 0.15;
                arp_phase += (2.0 * M_PI * arp_freq) / SAMPLE_RATE;
                if (arp_phase >= 2.0 * M_PI) arp_phase -= 2.0 * M_PI;
            }

            // Mix all 7 instruments
            double mixed = kick_out + snare_out + hh_out + clap_out + lead_out + bass_out + arp_out;
            
            // Hard clipping limiter
            if (mixed > 1.0) mixed = 1.0;
            if (mixed < -1.0) mixed = -1.0;

            buffer[idx] = (int16_t)(16384.0 * mixed);
        }
    }

    FILE *fp = fopen("eye_of_the_tiger.wav", "wb");
    if (!fp) return 1;
    write_wav_header(fp, total_samples * 2);
    fwrite(buffer, sizeof(int16_t), total_samples, fp);
    fclose(fp);
    free(buffer);

    printf("[BIONIKA] Mixed wav file written. Triggering playback...\n");
    fflush(stdout);

    // Play the full mixed 7-instrument wav natively
    system("aplay eye_of_the_tiger.wav 2>/dev/null || paplay eye_of_the_tiger.wav 2>/dev/null || true");

    return 0;
}

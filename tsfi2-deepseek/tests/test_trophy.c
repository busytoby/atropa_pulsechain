#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 8000
#define BPM 140
#define SAMPLES_PER_BEAT (SAMPLE_RATE * 60 / BPM) // ~3428 samples
#define NUM_BEATS 16
#define TOTAL_SAMPLES (SAMPLES_PER_BEAT * NUM_BEATS) // ~54857 samples (6.85 seconds)

// Resonator
typedef struct {
    double f;
    double q;
    double low, band, high;
} Resonator;

static void resonator_init(Resonator *r, double freq, double Q) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
    r->q = 1.0 / Q;
    r->low = r->band = r->high = 0.0;
}

static void resonator_update(Resonator *r, double freq) {
    double omega = 2.0 * M_PI * freq / SAMPLE_RATE;
    r->f = 2.0 * sin(omega / 2.0);
}

static double resonator_tick(Resonator *r, double input) {
    r->high = input - r->low - r->q * r->band;
    r->band += r->f * r->high;
    r->low += r->f * r->band;
    return r->band;
}

// Antiformant
typedef struct {
    double x1, x2;
    double b0, b1, b2;
} Antiformant;

static void antiformant_init(Antiformant *a, double zero_freq, double radius) {
    double theta = 2.0 * M_PI * zero_freq / SAMPLE_RATE;
    a->b0 = 1.0;
    a->b1 = -2.0 * radius * cos(theta);
    a->b2 = radius * radius;
    a->x1 = a->x2 = 0.0;
}

static double antiformant_tick(Antiformant *a, double input) {
    double out = a->b0 * input + a->b1 * a->x1 + a->b2 * a->x2;
    a->x2 = a->x1;
    a->x1 = input;
    return out;
}

// Low-Pass Filter for Bass
typedef struct {
    double low;
} LowPass;

static double lowpass_tick(LowPass *lp, double input, double cutoff) {
    double rc = 1.0 / (2.0 * M_PI * cutoff);
    double dt = 1.0 / SAMPLE_RATE;
    double alpha = dt / (rc + dt);
    lp->low = lp->low + alpha * (input - lp->low);
    return lp->low;
}

// WAV Header
static void write_wav_header(FILE *fp, uint32_t data_size) {
    uint32_t chunk_size = 36 + data_size;
    uint32_t subchunk1_size = 16;
    uint16_t audio_format = 1; // PCM
    uint16_t num_channels = 1; // Mono
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

int main() {
    printf("[TROPHY] Synthesizing 'arm art tart art' in dubstep format...\n");

    FILE *fp = fopen("trophy.wav", "wb");
    if (!fp) {
        perror("Failed to create trophy.wav");
        return 1;
    }

    write_wav_header(fp, TOTAL_SAMPLES * 2);

    // Initialize vocal resonators & filters
    Resonator v_f1, v_f2, v_f3;
    resonator_init(&v_f1, 730.0, 8.0);
    resonator_init(&v_f2, 1090.0, 10.0);
    resonator_init(&v_f3, 2440.0, 12.0);

    Antiformant v_zero;
    antiformant_init(&v_zero, 800.0, 0.95);

    // Initialize Wub Bass resonators
    Resonator w_f1, w_f2;
    resonator_init(&w_f1, 200.0, 4.0);
    resonator_init(&w_f2, 600.0, 4.0);

    LowPass sub_lp = {0.0};

    double glottal_phase = 0.0;
    double bass_phase = 0.0;
    int16_t *pcm_buffer = malloc(TOTAL_SAMPLES * sizeof(int16_t));

    for (int t = 0; t < TOTAL_SAMPLES; t++) {
        int beat = t / SAMPLES_PER_BEAT;
        int sample_in_beat = t % SAMPLES_PER_BEAT;

        // --- 1. Vocal Synthesis Segment ("arm art tart art") ---
        // We pronounce:
        // Beats 0-1: silent / prep
        // Beats 2-3: "arm"
        // Beats 4-5: "art"
        // Beats 6-7: "tart"
        // Beats 8-9: "art"
        // Beats 10-15: dubstep outro wubs
        double v_f1_tgt = 730.0, v_f2_tgt = 1090.0, v_f3_tgt = 2440.0;
        double v_zero_mix = 0.0;
        double v_gate = 0.0;
        double v_noise_gate = 0.0;

        int active_word = -1;
        int t_word = 0;

        if (beat >= 2 && beat < 4) {
            active_word = 0; // "arm"
            t_word = (beat - 2) * SAMPLES_PER_BEAT + sample_in_beat;
        } else if (beat >= 4 && beat < 6) {
            active_word = 1; // "art"
            t_word = (beat - 4) * SAMPLES_PER_BEAT + sample_in_beat;
        } else if (beat >= 6 && beat < 8) {
            active_word = 2; // "tart"
            t_word = (beat - 6) * SAMPLES_PER_BEAT + sample_in_beat;
        } else if (beat >= 8 && beat < 10) {
            active_word = 3; // "art"
            t_word = (beat - 8) * SAMPLES_PER_BEAT + sample_in_beat;
        }

        if (active_word == 0) { // "arm" (duration ~6857 samples)
            v_gate = 1.0;
            if (t_word < 2500) { // /a/
                v_f1_tgt = 730.0; v_f2_tgt = 1090.0; v_f3_tgt = 2440.0;
            } else if (t_word < 4500) { // transition to /r/
                double r = (t_word - 2500) / 2000.0;
                v_f1_tgt = 730.0 + r*(350.0-730.0);
                v_f2_tgt = 1090.0 + r*(1050.0-1090.0);
                v_f3_tgt = 2440.0 + r*(1500.0-2440.0);
            } else if (t_word < 5500) { // /r/
                v_f1_tgt = 350.0; v_f2_tgt = 1050.0; v_f3_tgt = 1500.0;
            } else { // /m/
                double r = (t_word - 5500) / 1357.0;
                v_f1_tgt = 350.0 + r*(250.0-350.0);
                v_f2_tgt = 1050.0 + r*(1000.0-1050.0);
                v_f3_tgt = 1500.0 + r*(2400.0-1500.0);
                v_zero_mix = r;
            }
        } else if (active_word == 1 || active_word == 3) { // "art"
            v_gate = 1.0;
            if (t_word < 2500) { // /a/
                v_f1_tgt = 730.0; v_f2_tgt = 1090.0; v_f3_tgt = 2440.0;
            } else if (t_word < 4500) { // transition to /r/
                double r = (t_word - 2500) / 2000.0;
                v_f1_tgt = 730.0 + r*(350.0-730.0);
                v_f2_tgt = 1090.0 + r*(1050.0-1090.0);
                v_f3_tgt = 2440.0 + r*(1500.0-2440.0);
            } else if (t_word < 5800) { // /r/
                v_f1_tgt = 350.0; v_f2_tgt = 1050.0; v_f3_tgt = 1500.0;
            } else { // /t/ closure and release noise burst
                v_gate = 0.0;
                v_noise_gate = 1.0;
            }
        } else if (active_word == 2) { // "tart"
            v_gate = 1.0;
            if (t_word < 800) { // initial /t/ burst
                v_gate = 0.0;
                v_noise_gate = 1.0;
            } else if (t_word < 2800) { // /a/
                v_f1_tgt = 730.0; v_f2_tgt = 1090.0; v_f3_tgt = 2440.0;
            } else if (t_word < 4500) { // transition to /r/
                double r = (t_word - 2800) / 1700.0;
                v_f1_tgt = 730.0 + r*(350.0-730.0);
                v_f2_tgt = 1090.0 + r*(1050.0-1090.0);
                v_f3_tgt = 2440.0 + r*(1500.0-2440.0);
            } else if (t_word < 5800) { // /r/
                v_f1_tgt = 350.0; v_f2_tgt = 1050.0; v_f3_tgt = 1500.0;
            } else { // final /t/
                v_gate = 0.0;
                v_noise_gate = 1.0;
            }
        }

        // Update vocal resonators
        resonator_update(&v_f1, v_f1_tgt);
        resonator_update(&v_f2, v_f2_tgt);
        resonator_update(&v_f3, v_f3_tgt);

        // Vocal excitation (Voiced glottal source)
        glottal_phase += 110.0 / SAMPLE_RATE;
        if (glottal_phase >= 1.0) glottal_phase -= 1.0;
        double rosenberg = 0.0;
        if (glottal_phase < 0.6) {
            double tp = glottal_phase / 0.6;
            rosenberg = 3.0 * tp * tp - 2.0 * tp * tp * tp;
        }
        
        double rand_val = ((double)rand() / RAND_MAX - 0.5);
        double vocal_noise = rand_val * 0.4 * v_noise_gate;
        double vocal_exc = rosenberg * v_gate + vocal_noise;

        double v_out = resonator_tick(&v_f1, vocal_exc) + 
                       0.5 * resonator_tick(&v_f2, vocal_exc) + 
                       0.25 * resonator_tick(&v_f3, vocal_exc);

        if (v_zero_mix > 0.01) {
            v_out = (1.0 - v_zero_mix) * v_out + v_zero_mix * antiformant_tick(&v_zero, v_out);
        }

        // --- 2. Dubstep Beats & Synthesizers ---
        // KICK: Triggers on beat 0, 4, 8, 12
        double kick = 0.0;
        if (beat % 4 == 0) {
            double kick_t = (double)sample_in_beat / SAMPLE_RATE;
            if (kick_t < 0.25) {
                // Pitch sweep from 150Hz down to 45Hz
                double f_sweep = 45.0 + 105.0 * exp(-80.0 * kick_t);
                kick = sin(2.0 * M_PI * f_sweep * kick_t) * exp(-10.0 * kick_t);
            }
        }

        // SNARE: Triggers on beat 2, 6, 10, 14 (classic dubstep half-time snare)
        double snare = 0.0;
        if (beat % 4 == 2) {
            double snare_t = (double)sample_in_beat / SAMPLE_RATE;
            if (snare_t < 0.2) {
                double noise_source = ((double)rand() / RAND_MAX - 0.5);
                // Bandpass noise at 1000 Hz, Q=2
                static Resonator snare_res;
                static int init_snare = 0;
                if (!init_snare) {
                    resonator_init(&snare_res, 1000.0, 2.0);
                    init_snare = 1;
                }
                snare = resonator_tick(&snare_res, noise_source) * exp(-15.0 * snare_t) * 1.5;
            }
        }

        // WUB BASS (Formant LFO Bass)
        // Synthesize saw/square wave
        bass_phase += 55.0 / SAMPLE_RATE; // Sub-bass E1 (55 Hz)
        if (bass_phase >= 1.0) bass_phase -= 1.0;
        
        // Square wave with high harmonic content for filtering
        double raw_bass = (bass_phase < 0.5) ? 0.8 : -0.8;
        
        // Add a bit of sawtooth
        raw_bass += (bass_phase * 2.0 - 1.0) * 0.4;

        // LFO Modulation at 8th note speed (4 wubs per beat at 140 BPM)
        double lfo = 0.5 + 0.5 * sin(2.0 * M_PI * 4.0 * (double)t / SAMPLE_RATE);
        
        // Sweep formant filter frequencies based on LFO
        double w_f1_freq = 200.0 + lfo * 500.0; // Sweeps 200Hz to 700Hz (producing /u/ -> /a/ vocal wub)
        double w_f2_freq = 600.0 + lfo * 1200.0; // Sweeps 600Hz to 1800Hz
        
        resonator_update(&w_f1, w_f1_freq);
        resonator_update(&w_f2, w_f2_freq);

        double wub = resonator_tick(&w_f1, raw_bass) + 0.6 * resonator_tick(&w_f2, raw_bass);

        // Sub bass component for low-end pressure
        double sub = sin(2.0 * M_PI * bass_phase);
        sub = lowpass_tick(&sub_lp, sub, 80.0) * 1.2;

        // Mix the wub and sub bass together, ducking when the Kick triggers
        double kick_duck = 1.0;
        if (beat % 4 == 0 && sample_in_beat < 1500) {
            kick_duck = (double)sample_in_beat / 1500.0; // Duck bass during kick transient
        }
        double bass_mix = (wub * 0.4 + sub * 0.8) * kick_duck;

        // --- 3. Master Mix ---
        double master = kick * 1.2 + snare * 1.0 + bass_mix * 0.9 + v_out * 1.1;

        // Limiter / Saturation
        if (master > 1.0) master = 1.0 - 0.2 * (master - 1.0); // Soft knee clipping
        if (master < -1.0) master = -1.0 - 0.2 * (master + 1.0);
        
        // Prevent hard clipping pops
        if (master > 1.2) master = 1.2;
        if (master < -1.2) master = -1.2;

        int16_t sample_out = (int16_t)(master * 20000.0);
        pcm_buffer[t] = sample_out;
    }

    fwrite(pcm_buffer, sizeof(int16_t), TOTAL_SAMPLES, fp);
    fclose(fp);
    free(pcm_buffer);

    printf("[TROPHY] Completed. Wrote 6.85 seconds of dubstep vocoded chant to 'trophy.wav'\n");
    return 0;
}

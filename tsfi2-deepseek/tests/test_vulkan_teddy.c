#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <poll.h>
#include <math.h>
#include <assert.h>
#include <wayland-client.h>
#include "xdg-shell-client-protocol.h"
#include <linux/input.h>
#include <time.h>
#include <omp.h>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include "tsfi_vm_dft_bridge.h"
#include "tsfi_valve.h"
#include "tsfi_obj_loader.h"
#include "tsfi_zener.h"
#include "tsfi_elektuur_issue25.h"



static TSFiVmStateParams params = {
    .identity_pole = 261640507549433ULL,
    .soul = 525772616431102ULL,
    .aura = 341042560473881ULL
};

struct SoundData {
    char type[32];
};

static const char* valve_names[] = {
    "Audion Classic", 
    "Audion Soft-Gas", 
    "Audion Landslide",
    "Audion Tubular T", 
    "Audion Flat-Plate D", 
    "Audion Cold-Cathode",
    "Audion Regenerative", 
    "Audion Grid-Leak",
    "12AX7 Preamp", 
    "EL84 Pentode", 
    "Class C Radio"
};
static int selected_valve = 0;

static inline float apply_valve_simulation(float input_sig, int valve_type) {
    if (valve_type == 0) {
        // DeForest Audion Classic: standard warm saturation
        return tanhf(input_sig * 2.5f);
    } else if (valve_type == 1) {
        // Audion Soft-Gas: gas ionization surge when grid is positive
        float base = tanhf(input_sig * 2.2f);
        if (input_sig > 0.2f) {
            base += 0.25f * (input_sig - 0.2f) * expf(0.5f * (input_sig - 0.2f));
        }
        return base;
    } else if (valve_type == 2) {
        // Audion Landslide: landslide gas breakdown discharge with hysteresis
        static int gas_latched = 0;
        float gain_sig = input_sig * 2.2f;
        if (gain_sig > 0.45f) {
            gas_latched = 1;
        } else if (gain_sig < 0.10f) {
            gas_latched = 0;
        }
        if (gas_latched) {
            return 0.82f + 0.12f * tanhf((gain_sig - 0.45f) * 0.4f);
        } else {
            return tanhf(gain_sig);
        }
    } else if (valve_type == 3) {
        // Audion Tubular T: coaxial quadratic smooth geometry
        if (input_sig > 0.0f) {
            return input_sig / (1.0f + 0.35f * input_sig);
        } else {
            return input_sig * (1.0f + 0.15f * input_sig);
        }
    } else if (valve_type == 4) {
        // Audion Flat-Plate D: sharp flat-plate cutoff
        if (input_sig < -0.6f) return -0.9f;
        return tanhf(input_sig * 2.8f);
    } else if (valve_type == 5) {
        // Audion Cold-Cathode: starvation emission limits
        float val = tanhf(input_sig * 3.0f);
        if (val > 0.55f) val = 0.55f + 0.05f * tanhf((val - 0.55f) * 2.0f);
        if (val < -0.7f) val = -0.7f + 0.05f * tanhf((val + 0.7f) * 2.0f);
        return val;
    } else if (valve_type == 6) {
        // Audion Regenerative: feedback phase-ringing emulation
        static float last_out = 0.0f;
        float val = tanhf((input_sig + last_out * 0.35f) * 2.4f);
        last_out = val;
        return val;
    } else if (valve_type == 7) {
        // Audion Grid-Leak: dynamic negative bias self-rectification (envelope compressor)
        static float grid_leak_bias = 0.0f;
        if (input_sig > 0.0f) {
            grid_leak_bias += 0.05f * (input_sig - grid_leak_bias);
        } else {
            grid_leak_bias *= 0.995f; // decay
        }
        return tanhf((input_sig - grid_leak_bias * 0.8f) * 2.5f);
    } else if (valve_type == 8) {
        // 12AX7 Preamp
        if (input_sig > 0.3f) {
            return 0.3f + 0.2f * tanhf((input_sig - 0.3f) * 1.5f);
        } else {
            return tanhf(input_sig * 1.2f);
        }
    } else if (valve_type == 9) {
        // EL84 Pentode
        return input_sig / sqrtf(1.0f + input_sig * input_sig);
    } else {
        // Class C Radio
        if (input_sig > 0.4f) return 1.0f;
        if (input_sig < -0.4f) return -1.0f;
        return 0.0f;
    }
}

#define MAX_VOICES 16
struct Voice {
    bool active;
    const uint8_t *buf;
    int len;
    int pos;
    struct timespec trigger_time;
    bool latency_measured;
    char type[32];
};
static struct Voice g_voices[MAX_VOICES] = {0};
static pthread_mutex_t g_audio_mutex = PTHREAD_MUTEX_INITIALIZER;
static bool g_audio_running = true;
static pthread_t g_audio_thread;

#define MAX_LATENCY_RECORDS 2000
typedef struct {
    char type[32];
    double sw_delay;
    double hw_delay;
    double total_latency;
} LatencyRecord;

static LatencyRecord g_latency_records[MAX_LATENCY_RECORDS];
static int g_latency_record_count = 0;
static pthread_mutex_t g_latency_mutex = PTHREAD_MUTEX_INITIALIZER;

static void record_latency_measurement(const char *type, double sw_delay, double hw_delay, double total_latency) {
    pthread_mutex_lock(&g_latency_mutex);
    if (g_latency_record_count < MAX_LATENCY_RECORDS) {
        strncpy(g_latency_records[g_latency_record_count].type, type, 31);
        g_latency_records[g_latency_record_count].type[31] = '\0';
        g_latency_records[g_latency_record_count].sw_delay = sw_delay;
        g_latency_records[g_latency_record_count].hw_delay = hw_delay;
        g_latency_records[g_latency_record_count].total_latency = total_latency;
        g_latency_record_count++;
    }
    pthread_mutex_unlock(&g_latency_mutex);
}

struct PrecomputedSound {
    const char *name;
    uint8_t *buf;
    int len;
};
#define NUM_SOUND_TYPES 12
static struct PrecomputedSound g_precomputed_sounds[NUM_SOUND_TYPES] = {
    {"leap", NULL, 0},
    {"dodge", NULL, 0},
    {"sword", NULL, 0},
    {"success", NULL, 0},
    {"fail", NULL, 0},
    {"kick", NULL, 0},
    {"snare", NULL, 0},
    {"tom", NULL, 0},
    {"hats", NULL, 0},
    {"ride", NULL, 0},
    {"clap", NULL, 0},
    {"snap", NULL, 0}
};

typedef struct {
    float mass;
    float damping;
    float stiffness;
    float non_lin_k;
    float pos;
    float vel;
} SpeakerModel;

static float process_speaker(SpeakerModel *sp, float input_force, float dt) {
    float current_stiffness = sp->stiffness + sp->non_lin_k * sp->pos * sp->pos;
    float accel = (input_force - sp->damping * sp->vel - current_stiffness * sp->pos) / sp->mass;
    sp->vel += accel * dt;
    sp->pos += sp->vel * dt;
    return sp->vel;
}

void precompute_all_sounds() {
    pthread_mutex_lock(&g_audio_mutex);
    for (int idx = 0; idx < NUM_SOUND_TYPES; idx++) {
        struct PrecomputedSound *ps = &g_precomputed_sounds[idx];
        if (ps->buf) {
            free(ps->buf);
            ps->buf = NULL;
        }
        int len = 0;
        uint8_t *buf = NULL;
        const char *type = ps->name;

        if (strcmp(type, "leap") == 0) {
            len = 2000; buf = malloc(len);
            if (buf) {
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float freq = 220.0f + 330.0f * (1.0f - expf(-8.0f * t));
                    float phase = freq * t * 2.0f * 3.14159265f;
                    float sat = apply_valve_simulation(1.2f * sinf(phase), selected_valve);
                    buf[i] = 128 + (int)((1.0f - t/0.25f) * 110.0f * sat);
                }
            }
        } else if (strcmp(type, "dodge") == 0) {
            len = 1600; buf = malloc(len);
            if (buf) {
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float freq = 440.0f - 180.0f * sinf(3.14159f * (t / 0.2f));
                    float phase = freq * t * 2.0f * 3.14159265f;
                    float sat = apply_valve_simulation(1.0f * sinf(phase), selected_valve);
                    buf[i] = 128 + (int)((1.0f - t/0.2f) * 110.0f * sat);
                }
            }
        } else if (strcmp(type, "sword") == 0) {
            len = 1200; buf = malloc(len);
            if (buf) {
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float freq = 600.0f * expf(-12.0f * t) + 150.0f;
                    float phase = freq * t * 2.0f * 3.14159265f;
                    float noise = ((float)(rand() % 200) - 100.0f) / 100.0f;
                    float wave = sinf(phase) * 0.6f + noise * 0.4f;
                    float sat = apply_valve_simulation(1.5f * wave, selected_valve);
                    buf[i] = 128 + (int)((1.0f - t/0.15f) * 120.0f * sat);
                }
            }
        } else if (strcmp(type, "success") == 0) {
            len = 2400; buf = malloc(len);
            if (buf) {
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float wave = sinf(523.25f * t * 2.0f * 3.14159f) * 0.5f + sinf(783.99f * t * 2.0f * 3.14159f) * 0.5f;
                    float sat = apply_valve_simulation(2.0f * wave, selected_valve);
                    buf[i] = 128 + (int)((1.0f - t/0.3f) * 110.0f * sat);
                }
            }
        } else if (strcmp(type, "fail") == 0) {
            len = 3200; buf = malloc(len);
            if (buf) {
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float wave = sinf(80.0f * t * 2.0f * 3.14159f) * 0.7f + sinf(120.0f * t * 2.0f * 3.14159f) * 0.3f;
                    float sat = apply_valve_simulation(wave * 2.0f, selected_valve);
                    buf[i] = 128 + (int)((1.0f - t/0.4f) * 120.0f * sat);
                }
            }
        } else if (strcmp(type, "kick") == 0) {
            len = 12000; buf = malloc(len);
            if (buf) {
                float siu_identity_factor = (float)((params.identity_pole % 1000000ULL) / 1000000.0f);
                float siu_soul_factor     = (float)((params.soul % 1000000ULL) / 1000000.0f);
                float *raw_sig = malloc(len * sizeof(float));
                float *valve_out = malloc(len * sizeof(float));
                if (raw_sig && valve_out) {
                    TsfiZener kick_zener;
                    tsfi_zener_init(&kick_zener, 4.7, 0.003, 8000.0); // 4.7V breakdown clamp
                    float y1 = 0.0f, y2 = 0.0f;
                    for (int i = 0; i < len; i++) {
                        float t = (float)i / 8000.0f;
                        
                        // Dynamic frequency sweep of the active Bridged-T resonator circuit
                        float pitch_env_fast = expf(-170.0f * t);
                        float pitch_env_slow = expf(-20.0f * t);
                        float base_f = 46.0f + 8.0f * siu_soul_factor;
                        float sweep_depth = 190.0f + 110.0f * siu_identity_factor;
                        float f = base_f + 70.0f * pitch_env_slow + sweep_depth * pitch_env_fast;
                        
                        // Resonator coefficients
                        float w = 2.0f * 3.14159265f * f / 8000.0f;
                        float cos_w = cosf(w);
                        
                        // Decay envelope adjusting Q dynamically over time (simulating analog circuit dampening)
                        float decay_rate = 0.994f * expf(-2.2f * t);
                        
                        // Trigger excitation impulse (narrow 1ms trigger pulse + cap discharge swing)
                        float trigger = 0.0f;
                        if (i == 0) trigger = 1.0f;
                        else if (i < 8) trigger = -0.5f * expf(-0.5f * (float)i);
                        
                        // 2-pole filter active loop
                        float out = trigger + 2.0f * decay_rate * cos_w * y1 - decay_rate * decay_rate * y2;
                        y2 = y1;
                        y1 = out;
                        
                        // Beater click (transient)
                        float click_freq = 2000.0f + 800.0f * siu_soul_factor;
                        float click_env = expf(-350.0f * t);
                        float click = sinf(click_freq * t * 2.0f * 3.14159265f) * click_env * 0.35f;
                        
                        // Add a tiny bit of noise burst for the initial impact thud
                        float noise = ((float)(rand() % 200) - 100.0f) / 100.0f;
                        float noise_env = expf(-450.0f * t);
                        float click_noise = noise * noise_env * 0.15f;
                        
                        float mixed = out * 0.95f + click + click_noise;
                        
                        // Route through Zener diode clamp in series (Vs = mixed * 8.0V drive, Rs = 470 ohms)
                        double z_noise = 0.0;
                        double clamped = tsfi_zener_tick(&kick_zener, (double)mixed * 8.0, 470.0, &z_noise);
                        
                        // Rescale back to normal range
                        float mixed_clamped = (float)(clamped + z_noise) * 0.25f;
                        
                        float saturated_out = tanhf(1.8f * mixed_clamped);
                        raw_sig[i] = saturated_out * 1.5f;
                    }
                    TsfiValveTriode kick_valve;
                    tsfi_valve_init(&kick_valve, 30.0, 0.00045, 250.0, -1.5);
                    kick_valve.R_plate = 80000.0;
                    kick_valve.is_tubular = (selected_valve == 3);
                    tsfi_valve_process_differential_feedback(&kick_valve, raw_sig, valve_out, len, 0.0, 1.0, 1.0 / 8000.0, 0.00003, 0.0003, 40000.0, 0.2);
                    
                    // Virtual 12" Subwoofer speaker emulation
                    SpeakerModel kick_sp = {1.0f, 28.0f, 750.0f, 9000.0f, 0.0f, 0.0f};
                    for (int i = 0; i < len; i++) {
                        float sample = (valve_out[i] - 125.0f) / 125.0f;
                        float speaker_out = process_speaker(&kick_sp, sample * 450.0f, 1.0f / 8000.0f);
                        if (speaker_out < -1.0f) speaker_out = -1.0f;
                        if (speaker_out > 1.0f) speaker_out = 1.0f;
                        buf[i] = 128 + (int)(speaker_out * 120.0f);
                    }
                }
                if (raw_sig) free(raw_sig);
                if (valve_out) free(valve_out);
            }
        } else if (strcmp(type, "snare") == 0) {
            len = 3200; buf = malloc(len);
            if (buf) {
                float siu_soul_factor = (float)((params.soul % 1000000ULL) / 1000000.0f);
                float siu_aura_factor = (float)((params.aura % 1000000ULL) / 1000000.0f);
                float y1 = 0.0f, y2 = 0.0f;
                float cos_w = cosf(2.0f * 3.14159265f * 2400.0f / 8000.0f); // 2.4 kHz bandpass for snappy rattle
                float r = 0.68f; // Resonant filter damping
                
                // Virtual full-range speaker emulation for snappy midrange crack
                SpeakerModel snare_sp = {0.25f, 65.0f, 8500.0f, 12000.0f, 0.0f, 0.0f};
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    
                    // 1. Dual-mode skin/shell resonance (fundamental + first overtone)
                    float f1 = 180.0f + 40.0f * siu_aura_factor;
                    float f2 = 330.0f + 80.0f * siu_aura_factor;
                    float tone1 = sinf(f1 * t * 2.0f * 3.14159f);
                    float tone2 = sinf(f2 * t * 2.0f * 3.14159f) * 0.45f;
                    float skin_env = expf(-55.0f * t); // rapid skin decay
                    float shell = (tone1 + tone2) * skin_env * 0.5f;
                    
                    // Transient crack click
                    float transient = sinf(3500.0f * t * 2.0f * 3.14159f) * expf(-400.0f * t) * 0.25f;
                    
                    // 2. Band-pass filtered snare wire rattle noise
                    float white_noise = ((float)(rand() % 200) - 100.0f) / 100.0f;
                    float rattle_bp = white_noise + 2.0f * r * cos_w * y1 - r * r * y2;
                    y2 = y1;
                    y1 = rattle_bp;
                    
                    float rattle_env = expf(-16.0f * t); // slower snare wire rattle decay
                    float aura_noise_mix = 0.45f + 0.35f * siu_soul_factor;
                    float rattle = rattle_bp * rattle_env * 0.48f * aura_noise_mix;
                    
                    // Combine and apply selected valve saturation
                    float sat = apply_valve_simulation((shell + transient + rattle) * 1.8f, selected_valve);
                    float speaker_out = process_speaker(&snare_sp, sat * 300.0f, 1.0f / 8000.0f);
                    if (speaker_out < -1.0f) speaker_out = -1.0f;
                    if (speaker_out > 1.0f) speaker_out = 1.0f;
                    buf[i] = 128 + (int)(speaker_out * 115.0f);
                }
            }
        } else if (strcmp(type, "tom") == 0) {
            len = 6000; buf = malloc(len);
            if (buf) {
                float y1 = 0.0f, y2 = 0.0f;
                float siu_identity_factor = (float)((params.identity_pole % 1000000ULL) / 1000000.0f);
                
                // Virtual mid-woofer cabinet emulation
                SpeakerModel tom_sp = {0.55f, 32.0f, 1500.0f, 6000.0f, 0.0f, 0.0f};
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    // Tom sweep: frequency dynamically sweeps from 160Hz + offset down to 80Hz + offset
                    float pitch_env = expf(-30.0f * t);
                    float base_f = 70.0f + 25.0f * siu_identity_factor;
                    float sweep_depth = 60.0f + 40.0f * siu_identity_factor;
                    float f = base_f + sweep_depth * pitch_env;
                    float w = 2.0f * 3.14159265f * f / 8000.0f;
                    float cos_w = cosf(w);
                    
                    // Resonator active decay rate
                    float decay_rate = 0.992f * expf(-2.0f * t);
                    
                    // Impulse trigger (slower rise than kick)
                    float trigger = (i == 0) ? 0.8f : ((i < 60) ? 0.15f * expf(-0.1f * i) : 0.0f);
                    
                    float out = trigger + 2.0f * decay_rate * cos_w * y1 - decay_rate * decay_rate * y2;
                    y2 = y1;
                    y1 = out;
                    
                    // Saturate output using chosen Audion preset
                    float sat = apply_valve_simulation(1.7f * out, selected_valve);
                    float speaker_out = process_speaker(&tom_sp, sat * 380.0f, 1.0f / 8000.0f);
                    if (speaker_out < -1.0f) speaker_out = -1.0f;
                    if (speaker_out > 1.0f) speaker_out = 1.0f;
                    buf[i] = 128 + (int)(speaker_out * 115.0f);
                }
            }
        } else if (strcmp(type, "hats") == 0) {
            len = 3500; buf = malloc(len);
            if (buf) {
                float x1 = 0.0f, x2 = 0.0f;
                float y1 = 0.0f, y2 = 0.0f;
                float freqs[6] = {315.0f, 435.0f, 565.0f, 690.0f, 820.0f, 950.0f};
                
                // High-pass micro-tweeter speaker emulation
                SpeakerModel hats_sp = {0.06f, 135.0f, 42000.0f, 18000.0f, 0.0f, 0.0f};
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    // Mix 6 detuned square-wave oscillators for metallic texture
                    float sum = 0.0f;
                    for (int osc = 0; osc < 6; osc++) {
                        sum += (sinf(freqs[osc] * t * 2.0f * 3.14159265f) > 0.0f ? 1.0f : -1.0f) * 0.12f;
                    }
                    // Add white noise wash
                    sum += ((float)(rand() % 200) - 100.0f) / 100.0f * 0.28f;
                    
                    // 2-pole high-pass filter (cutoff around 6.5kHz at 8kHz sample rate)
                    float hp = sum - 2.0f * x1 + x2 + 1.35f * y1 - 0.55f * y2;
                    x2 = x1;
                    x1 = sum;
                    y2 = y1;
                    y1 = hp;
                    
                    // Rapid hi-hat decay (exponential envelope)
                    float env = expf(-68.0f * t);
                    
                    // Saturation and gain
                    float sat = apply_valve_simulation(hp * env * 2.2f, selected_valve);
                    float speaker_out = process_speaker(&hats_sp, sat * 250.0f, 1.0f / 8000.0f);
                    if (speaker_out < -1.0f) speaker_out = -1.0f;
                    if (speaker_out > 1.0f) speaker_out = 1.0f;
                    buf[i] = 128 + (int)(speaker_out * 115.0f);
                }
            }
        } else if (strcmp(type, "ride") == 0) {
            len = 16000; buf = malloc(len);
            if (buf) {
                float last_sum = 0.0f;
                
                // Bright cymbal tweeter emulation
                SpeakerModel ride_sp = {0.08f, 95.0f, 32000.0f, 15000.0f, 0.0f, 0.0f};
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    // 1. FM-style bell tone (carrier=380Hz, modulator=912Hz)
                    float mod = sinf(912.0f * t * 2.0f * 3.14159265f);
                    float bell = sinf(380.0f * t * 2.0f * 3.14159265f + 1.8f * mod) * 0.45f;
                    
                    // 2. Metallic high-frequency noise wash
                    float white_noise = ((float)(rand() % 200) - 100.0f) / 100.0f;
                    float hp_noise = white_noise - last_sum;
                    last_sum = white_noise;
                    
                    // Combine bell strike with metallic noise wash
                    float strike = bell + hp_noise * 0.22f;
                    
                    // Long cymbal decay rate
                    float env = expf(-3.5f * t);
                    
                    // Saturate through valve simulation
                    float sat = apply_valve_simulation(strike * env * 1.5f, selected_valve);
                    float speaker_out = process_speaker(&ride_sp, sat * 280.0f, 1.0f / 8000.0f);
                    if (speaker_out < -1.0f) speaker_out = -1.0f;
                    if (speaker_out > 1.0f) speaker_out = 1.0f;
                    buf[i] = 128 + (int)(speaker_out * 115.0f);
                }
            }
        } else if (strcmp(type, "clap") == 0) {
            len = 4000; buf = malloc(len);
            if (buf) {
                float y1 = 0.0f, y2 = 0.0f;
                float cos_w = cosf(2.0f * 3.14159265f * 1100.0f / 8000.0f);
                float r = 0.72f;
                
                // Resonant midrange speaker emulation
                SpeakerModel clap_sp = {0.28f, 75.0f, 9500.0f, 10000.0f, 0.0f, 0.0f};
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float env = 0.0f;
                    if (i < 100) {
                        env = expf(-0.03f * i);
                    } else if (i < 200) {
                        env = expf(-0.03f * (i - 100));
                    } else if (i < 300) {
                        env = expf(-0.03f * (i - 200));
                    } else {
                        env = expf(-14.0f * (t - 0.0375f));
                    }
                    float noise = ((float)(rand() % 200) - 100.0f) / 100.0f;
                    float in_val = noise * env * 0.7f;
                    float out = in_val + 2.0f * r * cos_w * y1 - r * r * y2;
                    y2 = y1;
                    y1 = out;
                    
                    float sat = apply_valve_simulation(out * 1.5f, selected_valve);
                    float speaker_out = process_speaker(&clap_sp, sat * 320.0f, 1.0f / 8000.0f);
                    if (speaker_out < -1.0f) speaker_out = -1.0f;
                    if (speaker_out > 1.0f) speaker_out = 1.0f;
                    buf[i] = 128 + (int)(speaker_out * 110.0f);
                }
            }
        } else if (strcmp(type, "snap") == 0) {
            len = 2000; buf = malloc(len);
            if (buf) {
                float y1 = 0.0f, y2 = 0.0f;
                float cos_w = cosf(2.0f * 3.14159265f * 2000.0f / 8000.0f);
                float r = 0.78f;
                
                // Lighter high-midrange dome speaker emulation
                SpeakerModel snap_sp = {0.22f, 90.0f, 14000.0f, 15000.0f, 0.0f, 0.0f};
                for (int i = 0; i < len; i++) {
                    float t = (float)i / 8000.0f;
                    float trigger = (i == 0) ? 1.0f : ((i < 30) ? 0.3f * expf(-0.15f * i) : 0.0f);
                    float noise = ((float)(rand() % 200) - 100.0f) / 100.0f;
                    float click = noise * expf(-180.0f * t) * 0.4f;
                    float out = trigger + 2.0f * r * cos_w * y1 - r * r * y2;
                    y2 = y1;
                    y1 = out;
                    float body = out * expf(-110.0f * t) * 0.7f;
                    float mix = click + body;
                    
                    float sat = apply_valve_simulation(mix * 1.8f, selected_valve);
                    float speaker_out = process_speaker(&snap_sp, sat * 350.0f, 1.0f / 8000.0f);
                    if (speaker_out < -1.0f) speaker_out = -1.0f;
                    if (speaker_out > 1.0f) speaker_out = 1.0f;
                    buf[i] = 128 + (int)(speaker_out * 115.0f);
                }
            }
        }
        
        ps->buf = buf;
        ps->len = len;
    }
    pthread_mutex_unlock(&g_audio_mutex);
}

// TB-303 acid bass sequencer pattern (Classic 303 baseline)
typedef struct {
    uint8_t note;
    bool gate;
    bool slide;
    bool accent;
} Tb303Step;

static Tb303Step tb303_pattern[8] = {
    { 36, true,  false, false }, // C2
    { 36, true,  true,  false }, // C2 slide
    { 48, true,  false, true  }, // C3 accent
    { 38, true,  false, false }, // D2
    { 41, true,  true,  false }, // F2 slide
    { 43, true,  false, false }, // G2
    { 36, true,  false, true  }, // C2 accent
    { 48, false, false, false }  // rest
};

static float tb303_phase = 0.0f;
static float tb303_freq_curr = 0.0f;
static float tb303_freq_target = 0.0f;
static float tb303_env_val = 0.0f;
static float tb303_vca_env = 0.0f;
static float tb303_s1 = 0.0f, tb303_s2 = 0.0f, tb303_s3 = 0.0f, tb303_s4 = 0.0f;
static float tb303_accent_intensity = 0.0f;
static bool tb303_square_mode = false;
static TsfiAtoomVersterker tb303_amp;



void tb303_trigger_note(uint8_t note, bool accent, bool slide) {
    pthread_mutex_lock(&g_audio_mutex);
    float midi_freq = 440.0f * powf(2.0f, ((float)note - 69.0f) / 12.0f);
    tb303_freq_target = midi_freq;
    if (!slide || tb303_freq_curr == 0.0f) {
        tb303_freq_curr = midi_freq;
        tb303_env_val = 1.0f;
        tb303_vca_env = 1.0f;
    }
    tb303_accent_intensity = accent ? 1.0f : 0.0f;
    pthread_mutex_unlock(&g_audio_mutex);
}

static float fur_length;
static float breathing_freq;
static bool opt_ssaa;
static bool opt_viewport_boost;

static void* audio_mixer_thread(void *arg) {
    (void)arg;
    snd_pcm_t *pcm_handle = NULL;
    bool is_mock = false;
    if (snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        printf("[WARN] ALSA device cannot be opened. Using mock ALSA fallback.\n");
        is_mock = true;
    } else if (snd_pcm_set_params(pcm_handle, SND_PCM_FORMAT_U8, SND_PCM_ACCESS_RW_INTERLEAVED, 1, 8000, 1, 20000) < 0) {
        printf("[WARN] ALSA set_params failed. Using mock ALSA fallback.\n");
        snd_pcm_close(pcm_handle);
        pcm_handle = NULL;
        is_mock = true;
    }

    #define AUDIO_BLOCK_SIZE 256
    uint8_t mix_buf[AUDIO_BLOCK_SIZE];

    while (g_audio_running) {
        pthread_mutex_lock(&g_audio_mutex);
        bool any_active = false;
        for (int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
            int sum = 0;
            for (int v = 0; v < MAX_VOICES; v++) {
                if (g_voices[v].active) {
                    any_active = true;
                    int sample = (int)g_voices[v].buf[g_voices[v].pos] - 128;
                    sum += sample;
                    g_voices[v].pos++;
                    if (g_voices[v].pos >= g_voices[v].len) {
                        g_voices[v].active = false;
                    }
                }
            }
            
            // 303 Synth Engine sample generation
            tb303_freq_curr += (tb303_freq_target - tb303_freq_curr) * 0.0028f; // glide
            tb303_phase += tb303_freq_curr / 8000.0f;
            if (tb303_phase >= 1.0f) tb303_phase -= 1.0f;
            
            float source_wave;
            if (tb303_square_mode) {
                source_wave = (tb303_phase < 0.5f) ? 0.45f : -0.45f;
            } else {
                source_wave = 2.0f * tb303_phase - 1.0f;
            }
            
            // 303 Decay Envelopes
            float decay_rate = 0.9992f - 0.0003f * tb303_accent_intensity;
            tb303_env_val *= decay_rate;
            tb303_vca_env *= 0.9994f;
            
            // Diode Ladder Filter parameters modulated by Teddy Bear's physical traits
            float base_cutoff_hz = 200.0f + 150.0f * (1.0f - tb303_accent_intensity) + 120.0f * breathing_freq;
            float env_mod_hz = 900.0f + 600.0f * tb303_accent_intensity;
            float cutoff_hz = base_cutoff_hz + tb303_env_val * env_mod_hz;
            if (cutoff_hz > 3800.0f) cutoff_hz = 3800.0f;
            
            float g = tanhf(3.14159265f * cutoff_hz / 8000.0f);
            float res_k = 0.70f + 0.10f * tb303_accent_intensity + 0.15f * (fur_length / 0.15f);
            if (res_k > 0.94f) res_k = 0.94f; // cap to prevent excessive feedback distortion
            float fb = res_k * 4.0f * tanhf(tb303_s4);
            
            float input_stage = source_wave - fb;
            float ds1 = g * (tanhf(input_stage) - tanhf(tb303_s1));
            tb303_s1 += ds1;
            float ds2 = g * (tanhf(tb303_s1) - tanhf(tb303_s2));
            tb303_s2 += ds2;
            float ds3 = g * (tanhf(tb303_s2) - tanhf(tb303_s3));
            tb303_s3 += ds3;
            float ds4 = g * (tanhf(tb303_s3) - tanhf(tb303_s4));
            tb303_s4 += ds4;
            
            float synth_out = tb303_s4 * tb303_vca_env * 0.38f;
            
            // Map viewport boost & SSAA to drive valve saturation
            float drive = 1.0f;
            if (opt_viewport_boost) drive += 1.3f;
            if (opt_ssaa) drive += 0.8f;
            float driven_out = synth_out * drive;
            
            float sat_out = apply_valve_simulation(driven_out, selected_valve);
            
            // Involve Elektuur's transistor amplifier (AtoomVersterker)
            // Scale input to match base-emitter clipping range (input gain is 10x), process, and scale back down
            float amp_out = tsfi_atoom_versterker_process(&tb303_amp, sat_out * 0.1f) * 1.5f;
            
            int tb303_sample = (int)(amp_out * 128.0f);
            
            sum += tb303_sample;
            if (tb303_vca_env > 0.001f) {
                any_active = true;
            }
            
            int mixed = sum + 128;
            if (mixed < 0) mixed = 0;
            if (mixed > 255) mixed = 255;
            mix_buf[i] = (uint8_t)mixed;
        }

        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        snd_pcm_sframes_t delay_frames = 0;
        if (!is_mock && pcm_handle) {
            if (snd_pcm_delay(pcm_handle, &delay_frames) < 0) {
                delay_frames = 0;
            }
        } else {
            delay_frames = 1000 + (rand() % 100);
        }
        double hardware_buffer_delay = (double)delay_frames / 8000.0;

        for (int v = 0; v < MAX_VOICES; v++) {
            if (g_voices[v].active && !g_voices[v].latency_measured) {
                double sw_delay = (current_time.tv_sec - g_voices[v].trigger_time.tv_sec) +
                                   (current_time.tv_nsec - g_voices[v].trigger_time.tv_nsec) / 1e9;
                double total_latency = sw_delay + hardware_buffer_delay;
                record_latency_measurement(g_voices[v].type, sw_delay, hardware_buffer_delay, total_latency);
                g_voices[v].latency_measured = true;
            }
        }

        pthread_mutex_unlock(&g_audio_mutex);

        if (!is_mock && pcm_handle) {
            if (any_active) {
                snd_pcm_sframes_t frames = snd_pcm_writei(pcm_handle, mix_buf, AUDIO_BLOCK_SIZE);
                if (frames < 0) {
                    snd_pcm_prepare(pcm_handle);
                    snd_pcm_writei(pcm_handle, mix_buf, AUDIO_BLOCK_SIZE);
                }
            } else {
                memset(mix_buf, 128, AUDIO_BLOCK_SIZE);
                snd_pcm_writei(pcm_handle, mix_buf, AUDIO_BLOCK_SIZE);
            }
        } else {
            usleep(32000);
        }
    }
    if (!is_mock && pcm_handle) {
        snd_pcm_close(pcm_handle);
    }
    return NULL;
}

static void start_audio_mixer() {
    tsfi_atoom_versterker_init(&tb303_amp, 25.0f); // 25C baseline temp
    g_audio_running = true;
    pthread_create(&g_audio_thread, NULL, audio_mixer_thread, NULL);
}

static void stop_audio_mixer() {
    g_audio_running = false;
    pthread_join(g_audio_thread, NULL);
    for (int idx = 0; idx < NUM_SOUND_TYPES; idx++) {
        if (g_precomputed_sounds[idx].buf) {
            free(g_precomputed_sounds[idx].buf);
            g_precomputed_sounds[idx].buf = NULL;
        }
    }
}

static void play_synth_sound(const char *type) {
    const uint8_t *buf = NULL;
    int len = 0;
    
    pthread_mutex_lock(&g_audio_mutex);
    for (int idx = 0; idx < NUM_SOUND_TYPES; idx++) {
        if (strcmp(g_precomputed_sounds[idx].name, type) == 0) {
            buf = g_precomputed_sounds[idx].buf;
            len = g_precomputed_sounds[idx].len;
            break;
        }
    }
    
    if (buf && len > 0) {
        for (int i = 0; i < MAX_VOICES; i++) {
            if (!g_voices[i].active) {
                g_voices[i].buf = buf;
                g_voices[i].len = len;
                g_voices[i].pos = 0;
                g_voices[i].active = true;
                clock_gettime(CLOCK_MONOTONIC, &g_voices[i].trigger_time);
                g_voices[i].latency_measured = false;
                strncpy(g_voices[i].type, type, sizeof(g_voices[i].type) - 1);
                g_voices[i].type[sizeof(g_voices[i].type) - 1] = '\0';
                break;
            }
        }
    }
    pthread_mutex_unlock(&g_audio_mutex);
}

#include "tsfi_svdag.h"
#include "tsfi_opt_zmm.h"
#include "lau_memory.h"
#include "lau_thunk.h"
#include "tsfi_hilbert.h"
#include "tsfi_vision.h"
#include "tsfi_wave512.h"
#include "vulkan/vulkan_render.h" 
#include "vulkan/vulkan_logic.h"
#include "vulkan/vulkan_main.h"
#include "tsfi_logic.h"
#include "tsfi_k0rn_ops.h"
#include "tsfi_genetic.h"
#include "tsfi_vm_dft_bridge.h"
#include "tsfi_staging.h"

// Define custom CV_16FC4 layout pixel mapping to AB4H
typedef struct {
    uint16_t r, g, b, a;
} Ab4hPixel;

// Zero-copy AB4H Mat drawing structure
typedef struct {
    int rows;
    int cols;
    int stride;
    Ab4hPixel *data;
} TsfiAb4hMat;

// Structure to hold spatial points representing our teddy bear geometry (Ellipsoids)
typedef struct {
    float x, y, z;
    float r;
    float sx, sy, sz; // Scale factors to squash/stretch spheres into ellipsoids
} SphereGeometry;

// Global settings changed interactively via Wayland GUI
static float fur_length = 0.08f;
static float scale_val = 1.00f;
static float light_angle_deg = 135.0f;
static float breathing_freq = 1.0f;
static float twitch_intensity = 0.5f;
static float sickness_intensity = 0.5f;
static float ammeter_T = 293.15f;
static float voltmeter_V = 120.0f;
static float dna_fur_r = 0.62f;
static float dna_fur_g = 0.44f;
static float dna_fur_b = 0.22f;
static float dna_eye_r = 0.0f;
static float dna_eye_g = 1.0f;
static float dna_eye_b = 0.0f;

// Dynamic Fur Physics parameters
static float wind_velocity = 0.8f;
static float fur_stiffness = 1.0f;
static float dynamic_wind_dx = 0.0f;
static float dynamic_wind_dy = 0.0f;

// 1960s Popular Mechanics DIY Stepping-Switch Frame Sequencer with Closed-Loop State Verification
typedef struct {
    float fur_length;
    float scale_val;
    float light_angle;
    float expected_voltmeter_min;
    float expected_voltmeter_max;
} SequencerStep;

static SequencerStep sequence_program[6] = {
    { 0.08f, 1.0f, 135.0f, 100.0f, 125.0f },
    { 0.12f, 1.1f, 150.0f,  90.0f, 115.0f },
    { 0.05f, 0.9f, 120.0f, 105.0f, 130.0f },
    { 0.15f, 1.2f, 180.0f,  80.0f, 110.0f },
    { 0.06f, 0.8f,  90.0f, 110.0f, 135.0f },
    { 0.10f, 1.0f, 135.0f, 100.0f, 125.0f }
};

static int seq_current_step = 0;
static int seq_frame_counter = 0;
static bool seq_halted = false;
static bool seq_validation_passed = true;

// Synthesizer drum sequencer grid state (Track 0 = Kick, Track 1 = Snare, Track 2 = Toms, Track 3 = Hats, Track 4 = Ride, Track 5 = Clap, Track 6 = Snap)
static uint8_t seq_grid[7][8] = {
    {1, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 0, 0, 0, 1, 0},
    {0, 1, 0, 1, 0, 1, 0, 0},
    {0, 0, 1, 0, 0, 1, 0, 1},
    {1, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 0, 1, 0, 0, 0, 1}
};
static int seq_play_step = 0;
static int seq_play_counter = 0;
static float track_trigger_env[7] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

struct Particle {
    float x;
    float y;
    float vx;
    float vy;
    float r, g, b;
    float life;
    bool active;
};
#define MAX_PARTICLES 128
static struct Particle particles[MAX_PARTICLES];

static void spawn_particles(float x, float y, float r, float g, float b) {
    int spawned = 0;
    for (int i = 0; i < MAX_PARTICLES && spawned < 12; i++) {
        if (!particles[i].active) {
            particles[i].active = true;
            particles[i].x = x;
            particles[i].y = y;
            float angle = (float)(rand() % 360) * (3.14159265f / 180.0f);
            float speed = 2.0f + (float)(rand() % 100) / 100.0f * 4.0f;
            particles[i].vx = cosf(angle) * speed;
            particles[i].vy = sinf(angle) * speed;
            particles[i].r = r;
            particles[i].g = g;
            particles[i].b = b;
            particles[i].life = 1.0f;
            spawned++;
        }
    }
}

// Dragon's Lair Interactive Storybook Sequencer (1983 Retro QTE Game Loop)
static bool dl_game_active = false;
static int dl_score = 0;
static int dl_lives = 3;
static int dl_stage = 1;
static int dl_expected_action = 0;
static int dl_status = 0;
static const char* dl_stage_texts[6] = {
    "1. Entrance: Floor collapses! (RIGHT)",
    "2. Cavern: Beast lunges! (SWORD)",
    "3. Bridge: Falling bridge! (LEFT)",
    "4. Keep: Knight attacks! (UP)",
    "5. Lair: Singe wakes! (DOWN)",
    "6. Victory: Princess Daphne rescued!"
};
static int dl_stage_actions[5] = { 2, 3, 1, 4, 5 };
static bool hover_dl_btn = false;

static void init_dl_game(void) {
    dl_score = 0;
    dl_lives = 3;
    dl_stage = 1;
    dl_status = 0;
    dl_expected_action = dl_stage_actions[0];
}

static int mouse_x = 0;
static int mouse_y = 0;
static bool mouse_pressed = false;
static int active_slider = -1; // -1: none, 0: Fur Length, 1: Scale, 2: Light Angle, 3: Wind Velocity, 4: Fur Stiffness

static TsfiValveTriode mouse_valve_x;
static TsfiValveTriode mouse_valve_y;
static double mouse_sim_x = 0.0;
static double mouse_sim_y = 0.0;
static bool mouse_valves_initialized = false;
static float haptic_rumble_amplitude = 0.0f;
static float haptic_rumble_frequency = 0.0f;

// Hover states for dynamic aesthetics
static bool hover_slider0 = false;
static bool hover_slider1 = false;
static bool hover_slider2 = false;
static bool hover_slider3 = false;
static bool hover_slider4 = false;
static bool hover_btn1 = false;
static bool hover_btn2 = false;
static bool hover_btn3 = false;
static bool hover_valve_btn = false;

static bool demo_mode = false;
static bool opt_ssaa = true;
static bool opt_vignette = true;
static bool opt_tonemap = true;
static bool opt_viewport_boost = true;
static bool hover_ssaa = false;
static bool hover_vignette = false;
static bool hover_tonemap = false;
static bool hover_viewport_boost = false;

// Multi-Model Selection and Genetic Optimizer Trigger variables
static const char* vlm_names[] = {"moondream", "qwen2-vl", "llama3.2-vision", "claude", "gemini", "mock"};
static const char* generator_names[] = {"sd15", "turbo", "dream", "flux", "flux-dev", "sdxl"};
static int selected_vlm = 0;
static int selected_generator = 1;
static char opt_status[128] = "Optimizer Status: IDLE";
static bool hover_vlm_btn = false;
static bool hover_gen_btn = false;
static bool hover_run_btn = false;

// Synthesizer Texture Loading and Frame Buffer Display
static bool display_synthesized_image = false;
static bool force_procedural_rendering = false;
static uint8_t synthesized_pixels[512 * 512 * 3];
static bool load_synthesized_pixels() {
    FILE *f = fopen("tmp/dna_render_bear.raw", "rb");
    if (!f) return false;
    size_t read_bytes = fread(synthesized_pixels, 1, 512 * 512 * 3, f);
    fclose(f);
    return (read_bytes == 512 * 512 * 3);
}

// Custom 16-bit float converter helper
static inline uint16_t double_to_half(double f) {
    union { float f; uint32_t u; } u = { (float)f };
    uint32_t sign = (u.u >> 16) & 0x8000;
    int32_t exponent = ((u.u >> 23) & 0xff) - 127;
    uint32_t mantissa = u.u & 0x7fffff;
    if (exponent <= -15) return sign;
    if (exponent >= 16) return sign | 0x7c00;
    return sign | ((exponent + 15) << 10) | (mantissa >> 13);
}

// Half-to-float helper (Zero swizzling, maps 16-bit half precision float back to float)
static inline float half_to_float(uint16_t h) {
    uint32_t sign = (h & 0x8000) << 16;
    uint32_t exponent = (h & 0x7c00) >> 10;
    uint32_t mantissa = (h & 0x03ff) << 13;
    if (exponent == 0) {
        if (mantissa == 0) return 0.0f;
        while (!(mantissa & 0x00800000)) { mantissa <<= 1; exponent--; }
        exponent++; exponent += 127 - 15;
    } else if (exponent == 31) exponent = 255;
    else exponent += 127 - 15;
    union { uint32_t u; float f; } u = { sign | (exponent << 23) | mantissa };
    return u.f;
}

static inline Ab4hPixel make_ab4h_pixel(float r, float g, float b, float a) {
    Ab4hPixel p;
    p.r = double_to_half(r);
    p.g = double_to_half(g);
    p.b = double_to_half(b);
    p.a = double_to_half(a);
    return p;
}

// Shell texturing noise generator for photorealistic fur strands
static float hash_noise_2d(int x, int y) {
    uint32_t hash = (uint32_t)x * 73856093 ^ (uint32_t)y * 19349663;
    hash = (hash ^ 61) ^ (hash >> 16);
    hash += hash << 3;
    hash ^= hash >> 4;
    hash *= 0x27d4eb2d;
    hash ^= hash >> 15;
    return (float)(hash & 0xFFFF) / 65535.0f;
}

static float smooth_noise_2d(float x, float y) {
    int ix = (int)floorf(x);
    int iy = (int)floorf(y);
    float fx = x - (float)ix;
    float fy = y - (float)iy;
    
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);
    
    float a = hash_noise_2d(ix,     iy);
    float b = hash_noise_2d(ix + 1, iy);
    float c = hash_noise_2d(ix,     iy + 1);
    float d = hash_noise_2d(ix + 1, iy + 1);
    
    return a * (1.0f - ux) * (1.0f - uy) +
           b * ux * (1.0f - uy) +
           c * (1.0f - ux) * uy +
           d * ux * uy;
}

static float smooth_noise_2d_deriv(float x, float y, float *dx_out, float *dy_out) {
    int ix = (int)floorf(x);
    int iy = (int)floorf(y);
    float fx = x - (float)ix;
    float fy = y - (float)iy;
    
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uy = fy * fy * (3.0f - 2.0f * fy);
    
    float dux = 6.0f * fx * (1.0f - fx);
    float duy = 6.0f * fy * (1.0f - fy);
    
    float a = hash_noise_2d(ix,     iy);
    float b = hash_noise_2d(ix + 1, iy);
    float c = hash_noise_2d(ix,     iy + 1);
    float d = hash_noise_2d(ix + 1, iy + 1);
    
    float val = a * (1.0f - ux) * (1.0f - uy) +
                b * ux * (1.0f - uy) +
                c * (1.0f - ux) * uy +
                d * ux * uy;
                
    *dx_out = dux * ((b - a) * (1.0f - uy) + (d - c) * uy);
    *dy_out = duy * ((c - a) * (1.0f - ux) + (d - b) * ux);
    
    return val;
}

static float fur_noise(float x, float y, int shell, float *w_dx, float *w_dy) {
    float density = 36.0f; // Fine hair strands
    float gravity_y = 0.10f; // combing downwards offset per shell
    
    // Apply gravity-based grooming slope and waviness as a function of shell height
    float normalized_height = (float)shell / 48.0f;
    float comb_y = normalized_height * gravity_y;
    // Mirror comb_x depending on sign of x to maintain bilateral symmetry
    float comb_x = (x >= 0.0f ? 1.0f : -1.0f) * sinf(normalized_height * 4.0f) * 0.03f; // curl waviness
    
    float input_x = fabsf(x) * density;
    float input_y = y * density;
    
    // Domain warping: distort sampling coordinates using smooth 2D value noise with analytical derivatives
    float scale = 0.08f;
    float wx = smooth_noise_2d_deriv(input_x * scale, input_y * scale, w_dx, w_dy);
    float wy = smooth_noise_2d(input_x * scale + 13.7f, input_y * scale + 27.3f);
    
    float warp_strength = 0.6f + sickness_intensity * 2.0f;
    float warped_x = input_x + (wx - 0.5f) * warp_strength;
    float warped_y = input_y + (wy - 0.5f) * warp_strength;
    
    // Calculate wind offset in noise space (coordinate shift)
    float wind_offset_x = (normalized_height * dynamic_wind_dx) / (fur_stiffness + 0.01f);
    float wind_offset_y = (normalized_height * dynamic_wind_dy) / (fur_stiffness + 0.01f);
    
    // Cellular hair strand simulation
    float max_val = 0.0f;
    int ix = (int)floorf(warped_x);
    int iy = (int)floorf(warped_y);
    
    // Check 3x3 neighborhood to support overlapping hair fibers
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            int cx = ix + dx;
            int cy = iy + dy;
            
            // Deterministic hash for cell
            uint32_t hash = (uint32_t)cx * 73856093 ^ (uint32_t)cy * 19349663;
            hash = (hash ^ 61) ^ (hash >> 16);
            hash += hash << 3;
            hash ^= hash >> 4;
            hash *= 0x27d4eb2d;
            hash ^= hash >> 15;
            
            // Jittered fiber center in the cell
            float jx = (float)(hash & 0xFF) / 255.0f;
            float jy = (float)((hash >> 8) & 0xFF) / 255.0f;
            
            // Grooming direction offset (comb_x, comb_y) and wind offset
            float fx = (float)cx + 0.5f + (jx - 0.5f) * 0.7f - (fabsf(comb_x) + wind_offset_x) * density;
            float fy = (float)cy + 0.5f + (jy - 0.5f) * 0.7f - (comb_y + wind_offset_y) * density;
            
            float dist_x = warped_x - fx;
            float dist_y = warped_y - fy;
            float dist = sqrtf(dist_x * dist_x + dist_y * dist_y);
            
            // Tapered radius: hair gets thinner near the tip (larger shell index)
            float base_radius = 0.48f;
            float radius = base_radius * (1.0f - normalized_height * 0.85f);
            
            if (dist < radius) {
                float val = 1.0f - (dist / radius);
                if (val > max_val) {
                    max_val = val;
                }
            }
        }
    }
    
    // Correct derivative sign for mirrored x
    if (x < 0.0f) {
        *w_dx = -(*w_dx);
    }
    
    return max_val;
}

// GUI Drawing primitives directly using zero-copy AB4H matrix canvas
void draw_rect_ab4h(TsfiAb4hMat *canvas, int x, int y, int rw, int rh, Ab4hPixel color) {
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int cy = y; cy < y + rh; cy++) {
        if (cy < 0 || cy >= canvas->rows) continue;
        for (int cx = x; cx < x + rw; cx++) {
            if (cx < 0 || cx >= canvas->cols) continue;
            Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + cy * canvas->stride) + cx;
            if (ta >= 1.0f) {
                *dest = color;
            } else {
                float cr = half_to_float(dest->r), cg = half_to_float(dest->g), cb = half_to_float(dest->b), ca = half_to_float(dest->a);
                float nr = cr * (1.0f - ta) + tr * ta;
                float ng = cg * (1.0f - ta) + tg * ta;
                float nb = cb * (1.0f - ta) + tb * ta;
                float na = ca * (1.0f - ta) + ta;
                dest->r = double_to_half(nr);
                dest->g = double_to_half(ng);
                dest->b = double_to_half(nb);
                dest->a = double_to_half(na);
            }
        }
    }
}

void draw_char_ab4h(TsfiAb4hMat *canvas, char c, int x, int y, Ab4hPixel color) {
    uint32_t temp_buf[32 * 32] = {0};
    StagingBuffer temp_sb = {
        .magic = TSFI_STAGING_MAGIC,
        .data = temp_buf,
        .width = 32,
        .height = 32,
        .stride = 32 * 4,
        .size = 32 * 32 * 4
    };
    draw_debug_codepoint(&temp_sb, 0, 0, (uint32_t)c, 0xFFFFFFFF);
    float tr = half_to_float(color.r), tg = half_to_float(color.g), tb = half_to_float(color.b), ta = half_to_float(color.a);
    for (int j = 0; j < 32; j++) {
        int py = y - 4 + j;
        if (py < 0 || py >= canvas->rows) continue;
        for (int i = 0; i < 32; i++) {
            int px = x + i;
            if (px < 0 || px >= canvas->cols) continue;
            uint32_t p = temp_buf[j * 32 + i];
            float alpha = ((p >> 24) & 0xFF) / 255.0f * ta;
            if (alpha > 0.01f) {
                Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + py * canvas->stride) + px;
                float cr = half_to_float(dest->r), cg = half_to_float(dest->g), cb = half_to_float(dest->b), ca = half_to_float(dest->a);
                float nr = cr * (1.0f - alpha) + tr * alpha;
                float ng = cg * (1.0f - alpha) + tg * alpha;
                float nb = cb * (1.0f - alpha) + tb * alpha;
                float na = ca * (1.0f - alpha) + alpha;
                dest->r = double_to_half(nr);
                dest->g = double_to_half(ng);
                dest->b = double_to_half(nb);
                dest->a = double_to_half(na);
            }
        }
    }
}

void draw_string_ab4h(TsfiAb4hMat *canvas, const char *str, int x, int y, Ab4hPixel color) {
    int cx = x;
    while (*str) {
        if (*str == '\n') {
            y += 20;
            cx = x;
            str++;
            continue;
        }
        draw_char_ab4h(canvas, *str, cx, y, color);
        cx += 10;
        str++;
    }
}



static float smin_cubic(float a, float b, float k) {
    float h = k - fabs(a - b);
    if (h < 0.0f) h = 0.0f;
    h /= k;
    float min_val = (a < b) ? a : b;
    return min_val - h * h * h * k * (1.0f / 6.0f);
}

static float evaluate_d_blend(float cx, float cy, SphereGeometry *body) {
    float d = 1e10f;
    for (int i = 0; i < 8; i++) {
        float dx = (cx - body[i].x) / body[i].sx;
        float dy = (cy - body[i].y) / body[i].sy;
        float dist = (sqrtf(dx*dx + dy*dy) - body[i].r) * fminf(body[i].sx, body[i].sy);
        
        // Apply cupped/hollowed ear subtraction in 2D
        if (i == 2) { // Left ear
            float active_scale = body[i].r / 0.09f;
            float inner_dx = (cx - (body[i].x + 0.015f * active_scale)) / body[i].sx;
            float inner_dy = (cy - (body[i].y - 0.01f * active_scale)) / body[i].sy;
            float inner_dist = (sqrtf(inner_dx*inner_dx + inner_dy*inner_dy) - body[i].r * 0.70f) * fminf(body[i].sx, body[i].sy);
            dist = fmaxf(dist, -inner_dist);
        } else if (i == 3) { // Right ear
            float active_scale = body[i].r / 0.09f;
            float inner_dx = (cx - (body[i].x - 0.015f * active_scale)) / body[i].sx;
            float inner_dy = (cy - (body[i].y - 0.01f * active_scale)) / body[i].sy;
            float inner_dist = (sqrtf(inner_dx*inner_dx + inner_dy*inner_dy) - body[i].r * 0.70f) * fminf(body[i].sx, body[i].sy);
            dist = fmaxf(dist, -inner_dist);
        }
        
        d = smin_cubic(d, dist, 0.08f);
    }
    return d;
}

static float evaluate_d_blend_all(float cx, float cy, SphereGeometry *body, float *out_nx, float *out_ny, float *out_r, float *out_g, float *out_b) {
    float dists[8];
    float nxs[8];
    float nys[8];
    
    float sum_w = 0.0f;
    float r_acc = 0.0f;
    float g_acc = 0.0f;
    float b_acc = 0.0f;
    
    for (int i = 0; i < 8; i++) {
        float dx = (cx - body[i].x) / body[i].sx;
        float dy = (cy - body[i].y) / body[i].sy;
        float len = sqrtf(dx*dx + dy*dy);
        float dist = (len - body[i].r) * fminf(body[i].sx, body[i].sy);
        float nx = len > 0.0001f ? (dx / len) / body[i].sx : 0.0f;
        float ny = len > 0.0001f ? (dy / len) / body[i].sy : 0.0f;
        float nlen = sqrtf(nx*nx + ny*ny);
        if (nlen > 0.0001f) { nx /= nlen; ny /= nlen; }
        
        // Apply cupped/hollowed ear subtraction in 2D
        if (i == 2) { // Left ear
            float active_scale = body[i].r / 0.09f;
            float inner_dx = (cx - (body[i].x + 0.015f * active_scale)) / body[i].sx;
            float inner_dy = (cy - (body[i].y - 0.01f * active_scale)) / body[i].sy;
            float inner_len = sqrtf(inner_dx*inner_dx + inner_dy*inner_dy);
            float inner_dist = (inner_len - body[i].r * 0.70f) * fminf(body[i].sx, body[i].sy);
            if (dist > -inner_dist) {
                // Outer ear is closer
            } else {
                dist = -inner_dist;
                nx = inner_len > 0.0001f ? (-inner_dx / inner_len) / body[i].sx : 0.0f;
                ny = inner_len > 0.0001f ? (-inner_dy / inner_len) / body[i].sy : 0.0f;
                float nlen2 = sqrtf(nx*nx + ny*ny);
                if (nlen2 > 0.0001f) { nx /= nlen2; ny /= nlen2; }
            }
        } else if (i == 3) { // Right ear
            float active_scale = body[i].r / 0.09f;
            float inner_dx = (cx - (body[i].x - 0.015f * active_scale)) / body[i].sx;
            float inner_dy = (cy - (body[i].y - 0.01f * active_scale)) / body[i].sy;
            float inner_len = sqrtf(inner_dx*inner_dx + inner_dy*inner_dy);
            float inner_dist = (inner_len - body[i].r * 0.70f) * fminf(body[i].sx, body[i].sy);
            if (dist > -inner_dist) {
                // Outer ear is closer
            } else {
                dist = -inner_dist;
                nx = inner_len > 0.0001f ? (-inner_dx / inner_len) / body[i].sx : 0.0f;
                ny = inner_len > 0.0001f ? (-inner_dy / inner_len) / body[i].sy : 0.0f;
                float nlen2 = sqrtf(nx*nx + ny*ny);
                if (nlen2 > 0.0001f) { nx /= nlen2; ny /= nlen2; }
            }
        }
        
        dists[i] = dist;
        nxs[i] = nx;
        nys[i] = ny;
        
        float w = expf(-dist / 0.04f);
        sum_w += w;
        
        float sphere_r, sphere_g, sphere_b;
        if (dl_game_active) {
            if (i == 0 || i == 1 || i == 6 || i == 7) {
                sphere_r = 0.85f; sphere_g = 0.12f; sphere_b = 0.08f;
            } else if (i == 2 || i == 3) {
                sphere_r = 0.18f; sphere_g = 0.10f; sphere_b = 0.18f;
            } else if (i == 4 || i == 5) {
                sphere_r = 0.95f; sphere_g = 0.78f; sphere_b = 0.20f;
            } else {
                sphere_r = 0.85f; sphere_g = 0.12f; sphere_b = 0.08f;
            }
        } else {
            sphere_r = dna_fur_r;
            sphere_g = dna_fur_g;
            sphere_b = dna_fur_b;

            float height_factor = (body[i].y - 0.20f) / 0.40f;
            sphere_r += 0.12f * cosf(3.14159f * height_factor);
            sphere_g += 0.08f * cosf(3.14159f * (height_factor + 0.25f));
            sphere_b += 0.05f * cosf(3.14159f * (height_factor + 0.50f));
        }

        if (!dl_game_active && i == 0) {
            float active_scale = body[0].r / 0.32f;
            float chest_dx = cx - body[0].x;
            float chest_dy = cy - (body[0].y - 0.05f * active_scale);
            float chest_dist = sqrtf(chest_dx*chest_dx * 2.2f + chest_dy*chest_dy * 1.6f);
            float patch_blend = 1.0f - (chest_dist / (body[0].r * 0.72f));
            if (patch_blend < 0.0f) patch_blend = 0.0f;
            if (patch_blend > 1.0f) patch_blend = 1.0f;
            patch_blend = patch_blend * patch_blend * (3.0f - 2.0f * patch_blend);
            
            sphere_r = sphere_r * (1.0f - patch_blend) + 0.88f * patch_blend;
            sphere_g = sphere_g * (1.0f - patch_blend) + 0.80f * patch_blend;
            sphere_b = sphere_b * (1.0f - patch_blend) + 0.68f * patch_blend;
        }

        if (!dl_game_active && (i == 2 || i == 3)) {
            float active_scale = body[i].r / 0.09f;
            float inner_dx = cx - (body[i].x + (i == 2 ? 0.015f : -0.015f) * active_scale);
            float inner_dy = cy - (body[i].y - 0.01f * active_scale);
            float inner_dist = sqrtf(inner_dx*inner_dx + inner_dy*inner_dy);
            float ear_blend = 1.0f - (inner_dist / (body[i].r * 0.75f));
            if (ear_blend < 0.0f) ear_blend = 0.0f;
            if (ear_blend > 1.0f) ear_blend = 1.0f;
            ear_blend = ear_blend * ear_blend * (3.0f - 2.0f * ear_blend);
            
            sphere_r = dna_fur_r * (1.0f - ear_blend) + 0.82f * ear_blend;
            sphere_g = dna_fur_g * (1.0f - ear_blend) + 0.62f * ear_blend;
            sphere_b = dna_fur_b * (1.0f - ear_blend) + 0.52f * ear_blend;
        }

        if (!dl_game_active && (i == 6 || i == 7)) {
            float active_scale = body[i].r / 0.09f;
            float pad_dx = cx - body[i].x;
            float pad_dy = cy - (body[i].y - 0.02f * active_scale);
            float pad_dist = sqrtf(pad_dx*pad_dx + pad_dy*pad_dy);
            float pad_blend = 1.0f - (pad_dist / (body[i].r * 0.65f));
            if (pad_blend < 0.0f) pad_blend = 0.0f;
            if (pad_blend > 1.0f) pad_blend = 1.0f;
            pad_blend = pad_blend * pad_blend * (3.0f - 2.0f * pad_blend);

            sphere_r = sphere_r * (1.0f - pad_blend) + 0.82f * pad_blend;
            sphere_g = sphere_g * (1.0f - pad_blend) + 0.62f * pad_blend;
            sphere_b = sphere_b * (1.0f - pad_blend) + 0.52f * pad_blend;
        }
        
        r_acc += sphere_r * w;
        g_acc += sphere_g * w;
        b_acc += sphere_b * w;
    }
    
    float d = dists[0];
    float nx = nxs[0];
    float ny = nys[0];
    
    for (int i = 1; i < 8; i++) {
        float k = 0.08f;
        float w = 0.5f + 0.5f * (d - dists[i]) / k;
        if (w < 0.0f) w = 0.0f;
        if (w > 1.0f) w = 1.0f;
        float w_smooth = w * w * (3.0f - 2.0f * w); // C1 continuous smoothstep blend
        
        d = smin_cubic(d, dists[i], k);
        nx = nx * (1.0f - w_smooth) + nxs[i] * w_smooth;
        ny = ny * (1.0f - w_smooth) + nys[i] * w_smooth;
    }
    
    *out_nx = nx;
    *out_ny = ny;
    
    if (sum_w > 0.0001f) {
        *out_r = r_acc / sum_w;
        *out_g = g_acc / sum_w;
        *out_b = b_acc / sum_w;
    } else {
        *out_r = dna_fur_r;
        *out_g = dna_fur_g;
        *out_b = dna_fur_b;
    }
    
    return d;
}


__attribute__((unused)) static float sphere_ao(float px, float py, float pz, float nx, float ny, float nz, float cx, float cy, float cz, float r, float sx, float sy, float sz, float lx, float ly, float lz) {
    float wdx = cx - px;
    float wdy = cy - py;
    float wdz = cz - pz;
    float wd2 = wdx*wdx + wdy*wdy + wdz*wdz;
    float wd = sqrtf(wd2);
    
    // Average scaled radius of the ellipsoid
    float avg_r = r * (sx + sy + sz) / 3.0f;
    if (wd < avg_r + 0.001f) return 1.0f;
    
    float vx = wdx / wd;
    float vy = wdy / wd;
    float vz = wdz / wd;
    
    // Normal-oriented cosine weighting in world space
    float ndot = nx * vx + ny * vy + nz * vz;
    if (ndot < 0.0f) ndot = 0.0f;
    
    // Solid angle approximation of the sphere
    float sin_theta2 = (avg_r * avg_r) / wd2;
    if (sin_theta2 > 1.0f) sin_theta2 = 1.0f;
    
    // Directional weighting based on light alignment (directional AO)
    float ldot = vx * lx + vy * ly + vz * lz;
    float directional_factor = 0.4f + 0.6f * ldot;
    if (directional_factor < 0.0f) directional_factor = 0.0f;
    
    return 1.0f - ndot * sin_theta2 * directional_factor;
}


static float calculate_shadow(float px, float py, float pz, float lx, float ly, float lz, SphereGeometry *body, int ignore_idx) {
    float shadow = 1.0f;
    for (int i = 0; i < 15; i++) {
        if (i == ignore_idx) continue;
        
        float inv_sx = 1.0f / body[i].sx;
        float inv_sy = 1.0f / body[i].sy;
        float inv_sz = 1.0f / body[i].sz;
        
        float oc_x = (body[i].x - px) * inv_sx;
        float oc_y = (body[i].y - py) * inv_sy;
        float oc_z = (body[i].z - pz) * inv_sz;
        
        float r_lx = lx * inv_sx;
        float r_ly = ly * inv_sy;
        float r_lz = lz * inv_sz;
        float r_len = sqrtf(r_lx*r_lx + r_ly*r_ly + r_lz*r_lz);
        if (r_len > 0.0001f) { r_lx /= r_len; r_ly /= r_len; r_lz /= r_len; }
        
        float b = oc_x * r_lx + oc_y * r_ly + oc_z * r_lz;
        if (b < 0.001f) continue;
        
        float c = oc_x * oc_x + oc_y * oc_y + oc_z * oc_z - body[i].r * body[i].r;
        float h = b * b - c;
        
        float avg_scale = (body[i].sx + body[i].sy + body[i].sz) / 3.0f;
        if (h > 0.0f) {
            // Ray intersects the ellipsoid interior: full shadow
            return 0.15f;
        } else {
            // Cone-traced contact-hardening soft shadow approximation
            float dist_sq = oc_x*oc_x + oc_y*oc_y + oc_z*oc_z - b*b;
            float dist_to_surface = sqrtf(fmaxf(0.0f, dist_sq)) - body[i].r;
            if (dist_to_surface < 0.0f) dist_to_surface = 0.0f;
            
            // Light source size/spread coefficient for contact-hardening penumbra width
            float light_spread = 0.12f;
            float penumbra_radius = b * light_spread + 0.01f;
            
            float penumbra = 0.15f + 0.85f * (dist_to_surface * avg_scale) / penumbra_radius;
            if (penumbra < shadow) {
                shadow = penumbra;
                if (shadow <= 0.15f) return 0.15f;
            }
        }
    }
    if (shadow < 0.15f) shadow = 0.15f;
    return shadow;
}

static void calculate_combined_shadow_ao(float px, float py, float pz, float nx, float ny, float nz, float lx, float ly, float lz, SphereGeometry *body, int ignore_idx, float *out_shadow, float *out_ao) {
    float shadow = 1.0f;
    float ao = 1.0f;
    
    for (int i = 0; i < 15; i++) {
        if (i == ignore_idx) continue;
        
        float wdx = body[i].x - px;
        float wdy = body[i].y - py;
        float wdz = body[i].z - pz;
        float wd2 = wdx*wdx + wdy*wdy + wdz*wdz;
        float wd = sqrtf(wd2);
        
        // --- 1. Ambient Occlusion ---
        float avg_scale = (body[i].sx + body[i].sy + body[i].sz) / 3.0f;
        float avg_r = body[i].r * avg_scale;
        if (wd >= avg_r + 0.001f) {
            float vx = wdx / wd;
            float vy = wdy / wd;
            float vz = wdz / wd;
            
            // Normal-oriented cosine weighting in world space
            float ndot = nx * vx + ny * vy + nz * vz;
            if (ndot < 0.0f) ndot = 0.0f;
            
            // Solid angle approximation of the sphere
            float sin_theta2 = (avg_r * avg_r) / wd2;
            if (sin_theta2 > 1.0f) sin_theta2 = 1.0f;
            
            // Directional weighting based on light alignment (directional AO)
            float ldot = vx * lx + vy * ly + vz * lz;
            float directional_factor = 0.4f + 0.6f * ldot;
            if (directional_factor < 0.0f) directional_factor = 0.0f;
            
            ao *= (1.0f - ndot * sin_theta2 * directional_factor);
        }
        
        // --- 2. Soft Shadow ---
        float inv_sx = 1.0f / body[i].sx;
        float inv_sy = 1.0f / body[i].sy;
        float inv_sz = 1.0f / body[i].sz;
        
        float oc_x = wdx * inv_sx;
        float oc_y = wdy * inv_sy;
        float oc_z = wdz * inv_sz;
        
        float r_lx = lx * inv_sx;
        float r_ly = ly * inv_sy;
        float r_lz = lz * inv_sz;
        float r_len = sqrtf(r_lx*r_lx + r_ly*r_ly + r_lz*r_lz);
        if (r_len > 0.0001f) { r_lx /= r_len; r_ly /= r_len; r_lz /= r_len; }
        
        float b = oc_x * r_lx + oc_y * r_ly + oc_z * r_lz;
        if (b >= 0.001f) {
            float c = oc_x * oc_x + oc_y * oc_y + oc_z * oc_z - body[i].r * body[i].r;
            float h = b * b - c;
            
            if (h > 0.0f) {
                shadow = 0.15f;
            } else {
                float dist_sq = oc_x*oc_x + oc_y*oc_y + oc_z*oc_z - b*b;
                float dist_to_surface = sqrtf(fmaxf(0.0f, dist_sq)) - body[i].r;
                if (dist_to_surface < 0.0f) dist_to_surface = 0.0f;
                
                float light_spread = 0.12f;
                float penumbra_radius = b * light_spread + 0.01f;
                
                float penumbra = 0.15f + 0.85f * (dist_to_surface * avg_scale) / penumbra_radius;
                if (penumbra < shadow) {
                    shadow = penumbra;
                }
            }
        }
    }
    
    if (shadow < 0.15f) shadow = 0.15f;
    *out_shadow = shadow;
    *out_ao = ao;
}

static void draw_vaesen_valve_ab4h(TsfiAb4hMat *canvas, int x, int y, int w, int h, float intensity, int type) {
    if (x < 0 || y < 0 || x + w >= canvas->cols || y + h >= canvas->rows) return;
    Ab4hPixel gold = make_ab4h_pixel(0.85f, 0.65f, 0.2f, 1.0f);
    Ab4hPixel base_col = make_ab4h_pixel(0.4f, 0.25f, 0.1f, 1.0f);
    for (int cy = y + h - 12; cy < y + h; cy++) {
        for (int cx = x + 4; cx < x + w - 4; cx++) {
            Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + cy * canvas->stride) + cx;
            *dest = base_col;
        }
    }
    for (int cy = y + 10; cy < y + h - 12; cy++) {
        Ab4hPixel *d1 = (Ab4hPixel *)((char *)canvas->data + cy * canvas->stride) + (x + 2);
        Ab4hPixel *d2 = (Ab4hPixel *)((char *)canvas->data + cy * canvas->stride) + (x + w - 3);
        *d1 = gold;
        *d2 = gold;
    }
    for (int cx = x + 5; cx < x + w - 5; cx++) {
        Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + (y + 5) * canvas->stride) + cx;
        *dest = gold;
    }
    for (int cx = x + 3; cx <= x + 5; cx++) {
        Ab4hPixel *d1 = (Ab4hPixel *)((char *)canvas->data + (y + 7) * canvas->stride) + cx;
        Ab4hPixel *d2 = (Ab4hPixel *)((char *)canvas->data + (y + 7) * canvas->stride) + (w - 1 - cx + x);
        *d1 = gold;
        *d2 = gold;
    }
    Ab4hPixel glow_col;
    if (type == 1) {
        glow_col = make_ab4h_pixel(0.1f * intensity, 0.4f * intensity, 0.9f * intensity, 1.0f);
    } else if (type == 2) {
        glow_col = make_ab4h_pixel(0.9f * intensity, 0.1f * intensity, 0.4f * intensity, 1.0f);
    } else if (type == 3) {
        glow_col = make_ab4h_pixel(0.9f * intensity, 0.6f * intensity, 0.1f * intensity, 1.0f);
    } else {
        glow_col = make_ab4h_pixel(0.9f * intensity, 0.35f * intensity, 0.05f * intensity, 1.0f);
    }
    for (int gy = y + 8; gy < y + h - 12; gy++) {
        for (int gx = x + 3; gx < x + w - 3; gx++) {
            Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + gy * canvas->stride) + gx;
            float r_val = half_to_float(dest->r) + half_to_float(glow_col.r) * 0.4f;
            float g_val = half_to_float(dest->g) + half_to_float(glow_col.g) * 0.4f;
            float b_val = half_to_float(dest->b) + half_to_float(glow_col.b) * 0.4f;
            dest->r = double_to_half(r_val);
            dest->g = double_to_half(g_val);
            dest->b = double_to_half(b_val);
        }
    }
    Ab4hPixel filament_col = make_ab4h_pixel(1.0f, 0.7f + 0.3f * intensity, 0.3f + 0.7f * (1.0f - intensity), 1.0f);
    for (int gy = y + 12; gy < y + h - 18; gy++) {
        Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + gy * canvas->stride) + (x + w / 2);
        *dest = filament_col;
    }
    Ab4hPixel grid_col = make_ab4h_pixel(0.5f, 0.5f, 0.5f, 1.0f);
    for (int gy = y + 16; gy < y + h - 22; gy += 8) {
        for (int gx = x + 6; gx < x + w - 6; gx++) {
            Ab4hPixel *dest = (Ab4hPixel *)((char *)canvas->data + gy * canvas->stride) + gx;
            *dest = grid_col;
        }
    }
}

static Ab4hPixel evaluate_palette(float t) {
    float r = 0.5f + 0.5f * cosf(6.28318f * (1.0f * t + 0.00f));
    float g = 0.5f + 0.5f * cosf(6.28318f * (1.0f * t + 0.33f));
    float b = 0.5f + 0.5f * cosf(6.28318f * (1.0f * t + 0.67f));
    return make_ab4h_pixel(r, g, b, 1.0f);
}

// Complete Viewport rendering (left: rotating teddy bear, right: tool palette)
void render_frame(TsfiAb4hMat *canvas, int frame) {
    // 1. Clear background to dark charcoal/purple with semi-transparency (0.75f alpha)
    Ab4hPixel bg = make_ab4h_pixel(0.04f, 0.03f, 0.06f, 0.75f);
    draw_rect_ab4h(canvas, 0, 0, canvas->cols, canvas->rows, bg);

    // Watch and dynamically reload the raw synthesized image output file if not forced to procedural
    static time_t last_mtime = 0;
    struct stat st;
    if (!force_procedural_rendering && stat("tmp/dna_render_bear.raw", &st) == 0) {
        if (st.st_mtime != last_mtime) {
            last_mtime = st.st_mtime;
            if (load_synthesized_pixels()) {
                display_synthesized_image = true;
                snprintf(opt_status, sizeof(opt_status), "Optimizer Status: SYNTHESIZED RELOADED");
            }
        }
    }

    // 2. Render Viewport on the Left (800x720)
    // Stepping-Switch Sequencer update (Popular Mechanics)
    if (!seq_halted) {
        seq_frame_counter++;
        if (seq_frame_counter >= 60) {
            float current_V = voltmeter_V;
            if (current_V >= sequence_program[seq_current_step].expected_voltmeter_min &&
                current_V <= sequence_program[seq_current_step].expected_voltmeter_max) {
                seq_current_step = (seq_current_step + 1) % 6;
                seq_frame_counter = 0;
                seq_validation_passed = true;
                fur_length = sequence_program[seq_current_step].fur_length;
                scale_val = sequence_program[seq_current_step].scale_val;
                light_angle_deg = sequence_program[seq_current_step].light_angle;
            } else {
                seq_halted = true;
                seq_validation_passed = false;
                snprintf(opt_status, sizeof(opt_status), "SEQUENCER ERROR: STEP %d FAULT", seq_current_step + 1);
            }
        }
    }

    // Audio drum sequencer step advance
    for (int t = 0; t < 7; t++) {
        track_trigger_env[t] *= 0.85f;
    }
    seq_play_counter++;
    if (seq_play_counter >= 15) {
        seq_play_step = (seq_play_step + 1) % 8;
        seq_play_counter = 0;
        
        // Trigger TB-303 acid bassline step
        extern void tb303_trigger_note(uint8_t note, bool accent, bool slide);
        if (tb303_pattern[seq_play_step].gate) {
            tb303_trigger_note(
                tb303_pattern[seq_play_step].note,
                tb303_pattern[seq_play_step].accent,
                tb303_pattern[seq_play_step].slide
            );
        }
        
        if (seq_grid[0][seq_play_step]) {
            ammeter_T += 12.0f; // Kick drum adds a thermal surge to the ammeter
            play_synth_sound("kick");
            track_trigger_env[0] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 622.0f + 5.0f, 0.2f, 0.9f, 0.2f);
        }
        if (seq_grid[1][seq_play_step]) {
            play_synth_sound("snare");
            track_trigger_env[1] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 635.0f + 5.0f, 0.3f, 0.8f, 1.0f);
        }
        if (seq_grid[2][seq_play_step]) {
            play_synth_sound("tom");
            track_trigger_env[2] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 648.0f + 5.0f, 0.9f, 0.6f, 0.1f);
        }
        if (seq_grid[3][seq_play_step]) {
            play_synth_sound("hats");
            track_trigger_env[3] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 661.0f + 5.0f, 0.9f, 0.9f, 0.2f);
        }
        if (seq_grid[4][seq_play_step]) {
            play_synth_sound("ride");
            track_trigger_env[4] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 674.0f + 5.0f, 0.4f, 0.9f, 0.9f);
        }
        if (seq_grid[5][seq_play_step]) {
            play_synth_sound("clap");
            track_trigger_env[5] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 687.0f + 5.0f, 1.0f, 0.5f, 0.5f);
        }
        if (seq_grid[6][seq_play_step]) {
            play_synth_sound("snap");
            track_trigger_env[6] = 1.0f;
            spawn_particles(850.0f + (float)seq_play_step * 30.0f + 11.0f, 700.0f + 5.0f, 0.8f, 0.4f, 1.0f);
        }
    }

    double light_rad = (double)light_angle_deg * (3.14159265 / 180.0);
    float lx = (float)cos(light_rad);
    float ly = (float)sin(light_rad);
    float lz = 1.0f;
    float len = sqrtf(lx*lx + ly*ly + lz*lz);
    lx /= len; ly /= len; lz /= len;

    // Dynamic wind components calculated dynamically
    dynamic_wind_dx = (float)cos(light_rad) * wind_velocity;
    dynamic_wind_dy = (float)sin(light_rad) * wind_velocity;

    float pos_x = ((float)(params.identity_pole % 40) - 20.0f) * 0.02f;
    float theta = (float)frame * 0.04f;
    float cos_t = cosf(theta);
    float sin_t = sinf(theta);

    float current_sq = (breathing_freq * scale_val) * (breathing_freq * scale_val);
    ammeter_T += (current_sq * 10.0f - 0.05f * (ammeter_T - 293.15f)) * 0.033f;
    float needle_deflection = 0.005f * (ammeter_T - 293.15f);
    extern uint64_t lau_yul_get_diyat_tax_total(void);
    uint64_t yul_diyat = lau_yul_get_diyat_tax_total();
    if (yul_diyat > 0) {
        voltmeter_V = (float)yul_diyat;
    } else {
        voltmeter_V = 120.0f - (needle_deflection * 15.0f) + 3.0f * sinf((float)frame * 0.08f);
    }
    if (seq_grid[1][seq_play_step] && seq_play_counter < 3) {
        voltmeter_V -= 8.0f;
    }
    float ammeter_damping = 1.0f / (1.0f + needle_deflection * 1.5f);

    float parasitic_leak = 0.015f * cosf((float)frame * 0.24f * breathing_freq);
    float breathe = (0.05f * sinf((float)frame * 0.12f * breathing_freq) + parasitic_leak) * ammeter_damping;
    float twitch = 0.0f;
    if ((frame % 60) < 6) {
        twitch = ((float)(rand() % 100) / 100.0f) * 0.03f * twitch_intensity;
    }
    float active_scale = scale_val + breathe + twitch;

    SphereGeometry body[15];
    if (dl_game_active) {
        body[0] = (SphereGeometry){ 0.0f + pos_x,  -0.15f, 0.0f, 0.26f * active_scale, 1.25f, 0.90f, 1.0f };
        body[1] = (SphereGeometry){ 0.0f + pos_x,   0.24f, 0.0f, 0.22f * active_scale, 0.95f, 1.05f, 1.0f };
        body[2] = (SphereGeometry){ -0.32f * active_scale + pos_x, 0.10f * active_scale, -0.05f, 0.25f * active_scale, 0.50f, 1.50f, 1.0f };
        body[3] = (SphereGeometry){  0.32f * active_scale + pos_x, 0.10f * active_scale, -0.05f, 0.25f * active_scale, 0.50f, 1.50f, 1.0f };
        body[4] = (SphereGeometry){ -0.15f * active_scale + pos_x, 0.42f * active_scale, 0.0f, 0.06f * active_scale, 0.50f, 2.00f, 1.0f };
        body[5] = (SphereGeometry){  0.15f * active_scale + pos_x, 0.42f * active_scale, 0.0f, 0.06f * active_scale, 0.50f, 2.00f, 1.0f };
        body[6] = (SphereGeometry){ -0.25f * active_scale + pos_x, -0.08f * active_scale, 0.05f * active_scale, 0.06f * active_scale, 0.80f, 1.00f, 1.0f };
        body[7] = (SphereGeometry){  0.25f * active_scale + pos_x, -0.08f * active_scale, 0.05f * active_scale, 0.06f * active_scale, 0.80f, 1.00f, 1.0f };
        body[8] = (SphereGeometry){ 0.0f + pos_x,   0.14f, 0.15f * active_scale, 0.10f * active_scale, 1.35f, 0.85f, 1.0f };
        body[9] = (SphereGeometry){ 0.0f + pos_x,   0.17f, 0.23f * active_scale, 0.03f * active_scale, 1.20f, 0.85f, 1.0f };
        body[10] = (SphereGeometry){ -0.08f * active_scale + pos_x, 0.28f, 0.18f * active_scale, 0.035f * active_scale, 0.85f, 1.15f, 1.0f };
        body[11] = (SphereGeometry){  0.08f * active_scale + pos_x, 0.28f, 0.18f * active_scale, 0.035f * active_scale, 0.85f, 1.15f, 1.0f };
        body[12] = (SphereGeometry){ -0.10f * active_scale + pos_x, -0.32f * active_scale, -0.10f * active_scale, 0.15f * active_scale, 1.50f, 0.60f, 1.0f };
        body[13] = (SphereGeometry){ 0.0f + pos_x, -0.05f, 0.15f * active_scale, 0.05f * active_scale, 1.00f, 1.00f, 1.0f };
        body[14] = (SphereGeometry){  0.35f * active_scale + pos_x, -0.25f * active_scale, 0.10f * active_scale, 0.06f * active_scale, 1.00f, 1.00f, 1.0f };
    } else {
        body[0] = (SphereGeometry){ 0.0f + pos_x,  -0.12f, 0.0f, 0.32f * active_scale, 1.00f, 1.05f, 1.0f };
        body[1] = (SphereGeometry){ 0.0f + pos_x,   0.30f, 0.0f, 0.28f * active_scale, 1.15f, 1.05f, 1.0f };
        body[2] = (SphereGeometry){ -0.23f * active_scale + pos_x, 0.50f * active_scale, 0.0f, 0.12f * active_scale, 1.15f, 1.00f, 1.0f };
        body[3] = (SphereGeometry){  0.23f * active_scale + pos_x, 0.50f * active_scale, 0.0f, 0.12f * active_scale, 1.15f, 1.00f, 1.0f };
        body[4] = (SphereGeometry){ -0.25f * active_scale + pos_x, -0.05f * active_scale, 0.08f * active_scale, 0.07f * active_scale, 0.90f, 1.00f, 1.0f };
        body[5] = (SphereGeometry){  0.25f * active_scale + pos_x, -0.05f * active_scale, 0.08f * active_scale, 0.07f * active_scale, 0.90f, 1.00f, 1.0f };
        body[6] = (SphereGeometry){ -0.16f * active_scale + pos_x, -0.28f * active_scale, 0.10f * active_scale, 0.09f * active_scale, 1.00f, 1.00f, 1.0f };
        body[7] = (SphereGeometry){  0.16f * active_scale + pos_x, -0.28f * active_scale, 0.10f * active_scale, 0.09f * active_scale, 1.00f, 1.00f, 1.0f };
        body[8] = (SphereGeometry){ 0.0f + pos_x,   0.20f, 0.20f * active_scale, 0.11f * active_scale, 1.25f, 0.95f, 1.0f };
        body[9] = (SphereGeometry){ 0.0f + pos_x,   0.24f, 0.29f * active_scale, 0.035f * active_scale, 1.20f, 0.85f, 1.0f };
        body[10] = (SphereGeometry){ -0.11f * active_scale + pos_x, 0.34f, 0.24f * active_scale, 0.03f * active_scale, 0.90f, 1.10f, 1.0f };
        body[11] = (SphereGeometry){  0.11f * active_scale + pos_x, 0.34f, 0.24f * active_scale, 0.03f * active_scale, 0.90f, 1.10f, 1.0f };
        body[12] = (SphereGeometry){ 0.0f + pos_x, 0.10f, 0.18f * active_scale, 0.035f * active_scale, 1.30f, 0.80f, 1.0f };
        body[13] = (SphereGeometry){ -0.05f * active_scale + pos_x, 0.10f, 0.17f * active_scale, 0.045f * active_scale, 1.30f, 0.80f, 1.0f };
        body[14] = (SphereGeometry){  0.05f * active_scale + pos_x, 0.10f, 0.17f * active_scale, 0.045f * active_scale, 1.30f, 0.80f, 1.0f };

        // Apply head tilt around the neck joint (pos_x, 0.20f) for head indices
        float tilt_angle = (twitch_intensity - 0.5f) * 0.4f;
        float cos_h = cosf(tilt_angle);
        float sin_h = sinf(tilt_angle);
        int head_indices[] = {1, 2, 3, 8, 9, 10, 11};
        for (int idx = 0; idx < 7; idx++) {
            int i = head_indices[idx];
            float hx = body[i].x - pos_x;
            float hy = body[i].y - 0.20f;
            body[i].x = pos_x + hx * cos_h - hy * sin_h;
            body[i].y = 0.20f + hx * sin_h + hy * cos_h;
        }
    }

    for (int i = 0; i < 15; i++) {
        float dx = body[i].x - pos_x;
        float dz = body[i].z;
        body[i].x = pos_x + dx * cos_t - dz * sin_t;
        body[i].z = dx * sin_t + dz * cos_t;
    }

    int num_shells = 48;
    bool boost_active = opt_viewport_boost && mouse_pressed && (active_slider >= 0 && active_slider <= 2);

    float frame_jitter_x = 0.0f;
    float frame_jitter_y = 0.0f;
    if (haptic_rumble_amplitude > 0.0f) {
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        frame_jitter_x = haptic_rumble_amplitude * cosf(angle) * 0.1f;
        frame_jitter_y = haptic_rumble_amplitude * sinf(angle) * 0.1f;
    }

    #pragma omp parallel for collapse(2) shared(frame_jitter_x, frame_jitter_y)
    for (int y = 0; y < 720; y++) {
        for (int x = 0; x < 800; x++) {
            if (boost_active && ((x % 2 != 0) || (y % 2 != 0))) {
                continue;
            }
            // Quick bounding check for optimization (spatial pruning)
            float cx_center = ((float)x / 800.0f) * 2.4f - 1.2f + frame_jitter_x;
            float cy_center = ((float)y / 720.0f) * 2.16f - 1.08f + frame_jitter_y;
            
            bool skip_shading = true;
            if (display_synthesized_image) {
                skip_shading = false;
            } else {
                float d_center = evaluate_d_blend(cx_center, cy_center, body);
                if (d_center < fur_length + 0.15f) {
                    skip_shading = false;
                } else {
                    for (int i = 8; i < 15; i++) {
                        float dx = cx_center - body[i].x;
                        float dy = cy_center - body[i].y;
                        float dist = sqrtf(dx*dx + dy*dy) - body[i].r;
                        if (dist < 0.05f) {
                            skip_shading = false;
                            break;
                        }
                    }
                }
            }

            float acc_r = 0.02f;
            float acc_g = 0.01f;
            float acc_b = 0.03f;
            float acc_a = 0.75f;

            if (skip_shading) {
                float uvx = ((float)x / 800.0f) * 2.0f - 1.0f;
                float uvy = ((float)y / 720.0f) * 2.0f - 1.0f;
                float rad = sqrtf(uvx*uvx + uvy*uvy);
                float bg_factor = 1.0f - rad * 0.6f;
                if (bg_factor < 0.0f) bg_factor = 0.0f;
                
                float bg_shadow = calculate_shadow(cx_center, cy_center, -0.4f, lx, ly, lz, body, -1);
                
                acc_r = (0.10f * bg_factor + 0.02f) * (0.35f + 0.65f * bg_shadow);
                acc_g = (0.14f * bg_factor + 0.01f) * (0.35f + 0.65f * bg_shadow);
                acc_b = (0.22f * bg_factor + 0.03f) * (0.35f + 0.65f * bg_shadow);
                acc_a = 1.0f;
            } else {
                // Perform dynamic Super-Sampling Anti-Aliasing (SSAA)
                float sum_r = 0.0f;
                float sum_g = 0.0f;
                float sum_b = 0.0f;
                float sum_a = 0.0f;

                int num_samples = opt_ssaa ? 4 : 1;
                float offsets_x[4] = { -0.25f, 0.25f, -0.25f, 0.25f };
                float offsets_y[4] = { -0.25f, -0.25f, 0.25f, 0.25f };
                if (!opt_ssaa) {
                    offsets_x[0] = 0.0f;
                    offsets_y[0] = 0.0f;
                }

                for (int sub = 0; sub < num_samples; sub++) {
                    float sx = (float)x + offsets_x[sub];
                    float sy = (float)y + offsets_y[sub];

                    float cx = (sx / 800.0f) * 2.4f - 1.2f + frame_jitter_x;
                    float cy = (sy / 720.0f) * 2.16f - 1.08f + frame_jitter_y;

                    float sub_uvx = (sx / 800.0f) * 2.0f - 1.0f;
                    float sub_uvy = (sy / 720.0f) * 2.0f - 1.0f;
                    float sub_rad = sqrtf(sub_uvx*sub_uvx + sub_uvy*sub_uvy);
                    float sub_bg_factor = 1.0f - sub_rad * 0.6f;
                    if (sub_bg_factor < 0.0f) sub_bg_factor = 0.0f;
                    
                    float sub_bg_shadow = calculate_shadow(cx, cy, -0.4f, lx, ly, lz, body, -1);
                    
                    float sub_r = (0.10f * sub_bg_factor + 0.02f) * (0.35f + 0.65f * sub_bg_shadow);
                    float sub_g = (0.14f * sub_bg_factor + 0.01f) * (0.35f + 0.65f * sub_bg_shadow);
                    float sub_b = (0.22f * sub_bg_factor + 0.03f) * (0.35f + 0.65f * sub_bg_shadow);
                    float sub_a = 1.0f;

                    bool hit_bear = false;

                    if (display_synthesized_image) {
                        int src_x = ((int)sx * 512) / 800;
                        int src_y = ((int)sy * 512) / 720;
                        if (src_x >= 0 && src_x < 512 && src_y >= 0 && src_y < 512) {
                            int idx = (src_y * 512 + src_x) * 3;
                            sub_r = (float)synthesized_pixels[idx] / 255.0f;
                            sub_g = (float)synthesized_pixels[idx+1] / 255.0f;
                            sub_b = (float)synthesized_pixels[idx+2] / 255.0f;
                            sub_a = 1.0f;
                            hit_bear = true;
                        }
                    }

                    if (!hit_bear) {
                        for (int i = 8; i < 15; i++) {
                            float dx = cx - body[i].x;
                            float dy = cy - body[i].y;
                            float dist2 = dx*dx + dy*dy;
                            float r2 = body[i].r * body[i].r;
                            if (dist2 < r2) {
                                hit_bear = true;
                                float nz = sqrtf(r2 - dist2) / body[i].r;
                                float nx = dx / body[i].r;
                                float ny = dy / body[i].r;

                                float pz = body[i].z + sqrtf(r2 - dist2);
                                float shadow_val = 1.0f;
                                float analytical_ao = 1.0f;
                                calculate_combined_shadow_ao(cx, cy, pz, nx, ny, nz, lx, ly, lz, body, i, &shadow_val, &analytical_ao);

                                // 3-point hemisphere lighting
                                float diffuse_key = nx*lx + ny*ly + nz*lz;
                                if (diffuse_key < 0.0f) diffuse_key = 0.0f;
                                diffuse_key *= shadow_val;

                                float sky_light = 0.5f + 0.5f * ny;
                                float bounce_light = nx*(-lx) + nz*(-lz);
                                if (bounce_light < 0.0f) bounce_light = 0.0f;

                                // Independent lighting terms: shadow_val only affects key light, analytical_ao only affects ambient lights
                                float diffuse = (diffuse_key * 1.0f) + (sky_light * 0.15f + bounce_light * 0.10f) * analytical_ao;
                                float fresnel = powf(1.0f - fmaxf(nz, 0.0f), 4.0f) * 0.12f;

                                if (dl_game_active) {
                                    if (dist2 > r2 * 0.88f) {
                                        sub_r = 0.0f;
                                        sub_g = 0.0f;
                                        sub_b = 0.0f;
                                    } else {
                                        float cel_diff = 0.35f;
                                        if (diffuse > 0.65f) cel_diff = 1.0f;
                                        else if (diffuse > 0.25f) cel_diff = 0.65f;

                                        if (i == 8) {
                                            sub_r = 0.90f * cel_diff;
                                            sub_g = 0.85f * cel_diff;
                                            sub_b = 0.55f * cel_diff;
                                        } else if (i == 9) {
                                            sub_r = 0.15f * cel_diff;
                                            sub_g = 0.05f * cel_diff;
                                            sub_b = 0.05f * cel_diff;
                                        } else if (i == 10 || i == 11) {
                                            if (dist2 < r2 * 0.20f) {
                                                sub_r = 0.0f; sub_g = 0.0f; sub_b = 0.0f;
                                            } else {
                                                sub_r = 0.10f * cel_diff;
                                                sub_g = 0.90f * cel_diff;
                                                sub_b = 0.20f * cel_diff;
                                            }
                                        } else if (i == 12) {
                                            sub_r = 0.85f * cel_diff;
                                            sub_g = 0.12f * cel_diff;
                                            sub_b = 0.08f * cel_diff;
                                        } else {
                                            sub_r = 0.95f * cel_diff;
                                            sub_g = 0.78f * cel_diff;
                                            sub_b = 0.20f * cel_diff;
                                        }
                                    }
                                } else {
                                    if (i == 8) { // Snout with SSS
                                        float sss = powf(fmaxf(0.0f, -(nx*lx + ny*ly + nz*lz)), 3.0f) * 0.25f;
                                        sub_r = 0.85f * (diffuse * 0.7f + 0.3f) + fresnel * 0.3f + sss * 0.2f;
                                        sub_g = 0.80f * (diffuse * 0.7f + 0.3f) + fresnel * 0.3f + sss * 0.15f;
                                        sub_b = 0.70f * (diffuse * 0.7f + 0.3f) + fresnel * 0.3f + sss * 0.1f;
                                    } else if (i == 9) { // Nose with glossy specular highlights
                                         float hx = lx;
                                         float hy = ly;
                                         float hz = lz + 1.0f;
                                         float hlen = sqrtf(hx*hx + hy*hy + hz*hz);
                                         if (hlen > 0.0001f) { hx /= hlen; hy /= hlen; hz /= hlen; }
                                         float spec = nx*hx + ny*hy + nz*hz;
                                         if (spec < 0.0f) spec = 0.0f;
                                         spec = powf(spec, 24.0f) * 0.6f;
                                         sub_r = 0.06f * (diffuse * 0.8f + 0.2f) + spec + fresnel * 0.15f;
                                         sub_g = 0.06f * (diffuse * 0.8f + 0.2f) + spec + fresnel * 0.15f;
                                         sub_b = 0.06f * (diffuse * 0.8f + 0.2f) + spec + fresnel * 0.15f;
                                    } else if (i == 10 || i == 11) { // Eyes with specular highlights
                                         float hx = lx;
                                         float hy = ly;
                                         float hz = lz + 1.0f;
                                         float hlen = sqrtf(hx*hx + hy*hy + hz*hz);
                                         hx /= hlen; hy /= hlen; hz /= hlen;
                                         float spec = nx*hx + ny*hy + nz*hz;
                                         if (spec < 0.0f) spec = 0.0f;
                                         float spec_base = powf(spec, 32.0f) * 0.6f;
                                         float spec_coat = powf(spec, 128.0f) * 1.5f; // Sharp clear-coat glass highlight
                                         float eye_reflection = (ny * 0.4f + 0.6f) * fresnel * 0.8f; // Glossy reflections

                                         // Dynamic beat-responsive glow on the eyes synchronized with the kick drum and TB-303 env
                                         float kick_glow = track_trigger_env[0] * 0.4f;
                                         float acid_glow = tb303_vca_env * (0.2f + 0.6f * tb303_accent_intensity);
                                         sub_r = dna_eye_r * (diffuse * 0.6f + 0.2f) + spec_base + spec_coat + eye_reflection + kick_glow + acid_glow * 0.1f;
                                         sub_g = dna_eye_g * (diffuse * 0.6f + 0.2f) + spec_base + spec_coat + eye_reflection + (kick_glow * 0.2f) + acid_glow * 0.7f;
                                         sub_b = dna_eye_b * (diffuse * 0.6f + 0.2f) + spec_base + spec_coat + eye_reflection + (kick_glow * 0.2f) + acid_glow * 0.4f;
                                    } else { // Red Bow Tie (Indices 12, 13, 14)
                                        float hx = lx;
                                        float hy = ly;
                                        float hz = lz + 1.0f;
                                        float hlen = sqrtf(hx*hx + hy*hy + hz*hz);
                                        hx /= hlen; hy /= hlen; hz /= hlen;
                                        float spec = nx*hx + ny*hy + nz*hz;
                                        if (spec < 0.0f) spec = 0.0f;
                                        spec = powf(spec, 16.0f) * 0.5f;

                                        sub_r = 0.85f * (diffuse * 0.7f + 0.3f) + spec + fresnel * 0.2f;
                                        sub_g = 0.05f * (diffuse * 0.7f + 0.3f) + spec;
                                        sub_b = 0.10f * (diffuse * 0.7f + 0.3f) + spec;
                                    }
                                }
                                sub_a = 1.0f;
                                hit_bear = true;
                                break;
                            }
                        }
                    }

                    if (!hit_bear) {
                        float base_r = dna_fur_r + 0.08f * track_trigger_env[0];
                        float base_g = dna_fur_g + 0.05f * track_trigger_env[0];
                        float base_b = dna_fur_b + 0.03f * track_trigger_env[0];
                        float nx_blend = 0.0f;
                        float ny_blend = 0.0f;
                        float d_blend = evaluate_d_blend_all(cx, cy, body, &nx_blend, &ny_blend, &base_r, &base_g, &base_b);
                        
                        if (dl_game_active) {
                            if (d_blend < 0.0f) {
                                hit_bear = true;
                                if (d_blend > -0.012f) {
                                    sub_r = 0.0f; sub_g = 0.0f; sub_b = 0.0f;
                                } else {
                                    float nx = nx_blend;
                                    float ny = ny_blend;
                                    float dist_from_edge = -d_blend;
                                    float nz = dist_from_edge / (0.22f * active_scale);
                                    if (nz > 1.0f) nz = 1.0f;
                                    float nxy_scale = sqrtf(1.0f - nz*nz);
                                    nx *= nxy_scale;
                                    ny *= nxy_scale;

                                    int closest_idx = 0;
                                    float min_d = 1e10f;
                                    for (int i = 0; i < 8; i++) {
                                        float dx = cx - body[i].x;
                                        float dy = cy - body[i].y;
                                        float dist = sqrtf(dx*dx + dy*dy) - body[i].r;
                                        if (dist < min_d) {
                                            min_d = dist;
                                            closest_idx = i;
                                        }
                                    }

                                    float radius_with_shell = body[closest_idx].r;
                                    float dx_closest = cx - body[closest_idx].x;
                                    float dy_closest = cy - body[closest_idx].y;
                                    float under_sqrt = radius_with_shell*radius_with_shell - dx_closest*dx_closest - dy_closest*dy_closest;
                                    float pz = body[closest_idx].z + sqrtf(under_sqrt > 0.0f ? under_sqrt : 0.0f);
                                    float shadow_val = 1.0f;
                                    float analytical_ao = 1.0f;
                                    calculate_combined_shadow_ao(cx, cy, pz, nx, ny, nz, lx, ly, lz, body, closest_idx, &shadow_val, &analytical_ao);

                                    float diffuse_key = nx*lx + ny*ly + nz*lz;
                                    if (diffuse_key < 0.0f) diffuse_key = 0.0f;
                                    diffuse_key *= shadow_val;

                                    float sky_light = 0.5f + 0.5f * ny;
                                    float bounce_light = nx*(-lx) + nz*(-lz);
                                    if (bounce_light < 0.0f) bounce_light = 0.0f;

                                    float diffuse = (diffuse_key * 1.0f) + (sky_light * 0.15f + bounce_light * 0.10f) * analytical_ao;

                                    float cel_diff = 0.35f;
                                    if (diffuse > 0.65f) cel_diff = 1.0f;
                                    else if (diffuse > 0.25f) cel_diff = 0.65f;

                                    sub_r = base_r * cel_diff;
                                    sub_g = base_g * cel_diff;
                                    sub_b = base_b * cel_diff;
                                }
                                sub_a = 1.0f;
                            }
                        } else {
                            float accum_r = 0.0f;
                            float accum_g = 0.0f;
                            float accum_b = 0.0f;
                            float accum_a = 0.0f;

                            for (int shell = num_shells - 1; shell >= 0; shell--) {
                                float shell_offset = ((float)shell / num_shells) * fur_length;
                                if (d_blend < shell_offset) {
                                    float w_dx = 0.0f, w_dy = 0.0f;
                                    float noise_val = fur_noise(sx, sy, shell, &w_dx, &w_dy);
                                    float threshold = (float)shell / num_shells;

                                    if (noise_val > threshold || shell == 0) {
                                        hit_bear = true;

                                        float nx = nx_blend;
                                        float ny = ny_blend;

                                        float dist_from_edge = -d_blend;
                                        if (dist_from_edge < 0.0f) dist_from_edge = 0.0f;
                                        float nz = dist_from_edge / (0.22f * active_scale);
                                        if (nz > 1.0f) nz = 1.0f;
                                        float nxy_scale = sqrtf(1.0f - nz*nz);
                                        nx *= nxy_scale;
                                        ny *= nxy_scale;

                                        float bump_strength = 0.15f;
                                        nx += w_dx * bump_strength;
                                        ny += w_dy * bump_strength;
                                        float len3d = sqrtf(nx*nx + ny*ny + nz*nz);
                                        if (len3d > 0.001f) { nx /= len3d; ny /= len3d; nz /= len3d; }

                                        int closest_idx = 0;
                                        float min_d = 1e10f;
                                        for (int i = 0; i < 8; i++) {
                                            float dx = cx - body[i].x;
                                            float dy = cy - body[i].y;
                                            float dist = sqrtf(dx*dx + dy*dy) - body[i].r;
                                            if (dist < min_d) {
                                                min_d = dist;
                                                closest_idx = i;
                                            }
                                        }

                                        float radius_with_shell = body[closest_idx].r + shell_offset;
                                        float dx_closest = cx - body[closest_idx].x;
                                        float dy_closest = cy - body[closest_idx].y;
                                        float under_sqrt = radius_with_shell*radius_with_shell - dx_closest*dx_closest - dy_closest*dy_closest;
                                        float pz = body[closest_idx].z + sqrtf(under_sqrt > 0.0f ? under_sqrt : 0.0f);
                                        float shadow_val = 1.0f;
                                        float analytical_ao = 1.0f;
                                        calculate_combined_shadow_ao(cx, cy, pz, nx, ny, nz, lx, ly, lz, body, closest_idx, &shadow_val, &analytical_ao);

                                        float diffuse_key = nx*lx + ny*ly + nz*lz;
                                        if (diffuse_key < 0.0f) diffuse_key = 0.0f;
                                        diffuse_key *= shadow_val;

                                        float sky_light = 0.5f + 0.5f * ny;
                                        float bounce_light = nx*(-lx) + nz*(-lz);
                                        if (bounce_light < 0.0f) bounce_light = 0.0f;

                                        float diffuse = (diffuse_key * 1.0f) + (sky_light * 0.15f + bounce_light * 0.10f) * analytical_ao;
                                        float fresnel = powf(1.0f - fmaxf(nz, 0.0f), 4.0f) * 0.12f;

                                        float tip_factor = (float)shell / num_shells;
                                        float current_r = base_r;
                                        float current_g = base_g;
                                        float current_b = base_b;
                                        if (tip_factor > 0.5f) {
                                            float blend = (tip_factor - 0.5f) / 0.5f;
                                            current_r = current_r * (1.0f - blend) + 0.85f * blend;
                                            current_g = current_g * (1.0f - blend) + 0.68f * blend;
                                            current_b = current_b * (1.0f - blend) + 0.42f * blend;
                                        }

                                        float hx = lx;
                                        float hy = ly;
                                        float hz = lz + 1.0f;
                                        float hlen = sqrtf(hx*hx + hy*hy + hz*hz);
                                        if (hlen > 0.0001f) { hx /= hlen; hy /= hlen; hz /= hlen; }
                                         float dotNH = nx*hx + ny*hy + nz*hz;
                                         float fur_spec = powf(fmaxf(0.0f, dotNH), 32.0f) * 0.5f * tip_factor;

                                        float shell_r = current_r * (diffuse * 0.8f + 0.2f) + fresnel * 0.85f + fur_spec;
                                        float shell_g = current_g * (diffuse * 0.8f + 0.2f) + fresnel * 0.75f + fur_spec * 0.8f;
                                        float shell_b = current_b * (diffuse * 0.8f + 0.2f) + fresnel * 0.65f + fur_spec * 0.5f;

                                        float sheen = powf(1.0f - fmaxf(nx*lx + ny*ly + nz*lz, 0.0f), 3.0f) * powf(nz, 2.0f) * 0.15f;
                                        shell_r += sheen * 0.75f;
                                        shell_g += sheen * 0.65f;
                                        shell_b += sheen * 0.50f;

                                         // Generalized Subsurface Scattering for translucent organic fur look
                                         float sss_thickness = 1.0f - tip_factor;
                                         float sss = powf(fmaxf(0.0f, -(nx*lx + ny*ly + nz*lz)), 3.0f) * 0.28f * sss_thickness;
                                         shell_r += sss * 0.85f;
                                         shell_g += sss * 0.45f;
                                         shell_b += sss * 0.30f;

                                         float ao = 0.4f + 0.6f * ((float)shell / num_shells);
                                         shell_r *= ao; shell_g *= ao; shell_b *= ao;

                                        float shell_a = (shell == 0) ? 1.0f : (noise_val - threshold) * 2.5f;
                                        if (shell_a < 0.0f) shell_a = 0.0f;
                                        if (shell_a > 1.0f) shell_a = 1.0f;

                                        accum_r += (1.0f - accum_a) * shell_r * shell_a;
                                        accum_g += (1.0f - accum_a) * shell_g * shell_a;
                                        accum_b += (1.0f - accum_a) * shell_b * shell_a;
                                        accum_a += (1.0f - accum_a) * shell_a;

                                        if (accum_a >= 0.95f) {
                                            accum_a = 1.0f;
                                            break;
                                        }
                                    }
                                }
                            }
                            if (hit_bear) {
                                sub_r = accum_r + (1.0f - accum_a) * sub_r;
                                sub_g = accum_g + (1.0f - accum_a) * sub_g;
                                sub_b = accum_b + (1.0f - accum_a) * sub_b;
                                sub_a = 1.0f;
                            }
                        }
                    }

                    sum_r += sub_r;
                    sum_g += sub_g;
                    sum_b += sub_b;
                    sum_a += sub_a;
                }

                acc_r = sum_r / (float)num_samples;
                acc_g = sum_g / (float)num_samples;
                acc_b = sum_b / (float)num_samples;
                acc_a = sum_a / (float)num_samples;
            }

            Ab4hPixel *dest_pixel = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride) + x;
            dest_pixel->r = double_to_half(acc_r);
            dest_pixel->g = double_to_half(acc_g);
            dest_pixel->b = double_to_half(acc_b);
            dest_pixel->a = double_to_half(acc_a);
        }
    }

    // 2.5 Viewport Boost reconstruction pass
    if (boost_active) {
        #pragma omp parallel for collapse(2)
        for (int y = 0; y < 720; y += 2) {
            for (int x = 0; x < 800; x += 2) {
                Ab4hPixel *p00 = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride) + x;
                if (x + 1 < 800) {
                    Ab4hPixel *p10 = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride) + (x + 1);
                    *p10 = *p00;
                }
                if (y + 1 < 720) {
                    Ab4hPixel *p01 = (Ab4hPixel *)((char *)canvas->data + (y + 1) * canvas->stride) + x;
                    *p01 = *p00;
                }
                if (x + 1 < 800 && y + 1 < 720) {
                    Ab4hPixel *p11 = (Ab4hPixel *)((char *)canvas->data + (y + 1) * canvas->stride) + (x + 1);
                    *p11 = *p00;
                }
            }
        }
    }

    // 3. Render Tool Palette on the Right (480x720)
    // Draw Divider Line
    Ab4hPixel divider_color = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    draw_rect_ab4h(canvas, 800, 0, 4, 720, divider_color);

    // Header Text
    Ab4hPixel text_title = make_ab4h_pixel(0.8f, 0.3f, 1.0f, 1.0f); // Neon Violet
    draw_string_ab4h(canvas, "TSFI TEDDY GENERATOR & LLM INTERFACE", 820, 30, text_title);

    // Render 3 Steampunk Vaesen Valves
    float v0_intensity = track_trigger_env[0] > track_trigger_env[3] ? track_trigger_env[0] : track_trigger_env[3];
    float v1_intensity = track_trigger_env[1] > track_trigger_env[4] ? track_trigger_env[1] : track_trigger_env[4];
    float v2_intensity = track_trigger_env[2];
    if (track_trigger_env[5] > v2_intensity) v2_intensity = track_trigger_env[5];
    if (track_trigger_env[6] > v2_intensity) v2_intensity = track_trigger_env[6];

    v0_intensity = 0.15f + 0.85f * v0_intensity;
    v1_intensity = 0.15f + 0.85f * v1_intensity;
    v2_intensity = 0.15f + 0.85f * v2_intensity;

    draw_vaesen_valve_ab4h(canvas, 1090, 15, 26, 50, v0_intensity, 1);
    draw_vaesen_valve_ab4h(canvas, 1125, 15, 26, 50, v1_intensity, 2);
    draw_vaesen_valve_ab4h(canvas, 1160, 15, 26, 50, v2_intensity, 3);

    // Draw selection highlights
    int sel_x = (selected_valve == 1) ? 1090 : ((selected_valve == 2) ? 1125 : ((selected_valve == 3) ? 1160 : -1));
    if (sel_x != -1) {
        Ab4hPixel ring_col = make_ab4h_pixel(1.0f, 0.9f, 0.2f, 1.0f);
        draw_rect_ab4h(canvas, sel_x - 3, 12, 32, 2, ring_col);
        draw_rect_ab4h(canvas, sel_x - 3, 66, 32, 2, ring_col);
        draw_rect_ab4h(canvas, sel_x - 3, 12, 2, 56, ring_col);
        draw_rect_ab4h(canvas, sel_x + 27, 12, 2, 56, ring_col);
    }

    // Live Valve Telemetry (Voltmeter & Ammeter)
    float active_ammeter = 0.05f * (ammeter_T - 293.15f);
    char v0_v_str[16], v0_i_str[16];
    char v1_v_str[16], v1_i_str[16];
    char v2_v_str[16], v2_i_str[16];
    
    snprintf(v0_v_str, sizeof(v0_v_str), "%.0fV", voltmeter_V * (0.85f + 0.15f * v0_intensity));
    snprintf(v0_i_str, sizeof(v0_i_str), "%.1fmA", active_ammeter * v0_intensity * 100.0f);
    
    snprintf(v1_v_str, sizeof(v1_v_str), "%.0fV", voltmeter_V * (0.85f + 0.15f * v1_intensity));
    snprintf(v1_i_str, sizeof(v1_i_str), "%.1fmA", active_ammeter * v1_intensity * 100.0f);
    
    snprintf(v2_v_str, sizeof(v2_v_str), "%.0fV", voltmeter_V * (0.85f + 0.15f * v2_intensity));
    snprintf(v2_i_str, sizeof(v2_i_str), "%.1fmA", active_ammeter * v2_intensity * 100.0f);

    Ab4hPixel col_v0 = make_ab4h_pixel(0.4f, 0.6f, 1.0f, 1.0f); // Blue glow for SG
    Ab4hPixel col_v1 = make_ab4h_pixel(1.0f, 0.4f, 0.6f, 1.0f); // Pink glow for LS
    Ab4hPixel col_v2 = make_ab4h_pixel(1.0f, 0.8f, 0.2f, 1.0f); // Yellow/gold glow for TB

    draw_string_ab4h(canvas, "SG",   1094, 70, col_v0);
    draw_string_ab4h(canvas, v0_v_str, 1088, 81, col_v0);
    draw_string_ab4h(canvas, v0_i_str, 1084, 92, col_v0);

    draw_string_ab4h(canvas, "LS",   1129, 70, col_v1);
    draw_string_ab4h(canvas, v1_v_str, 1123, 81, col_v1);
    draw_string_ab4h(canvas, v1_i_str, 1119, 92, col_v1);

    draw_string_ab4h(canvas, "TB",   1164, 70, col_v2);
    draw_string_ab4h(canvas, v2_v_str, 1158, 81, col_v2);
    draw_string_ab4h(canvas, v2_i_str, 1154, 92, col_v2);

    // Section Header: Controls (Shifted down to 108 to clear valve telemetry)
    Ab4hPixel text_hdr = make_ab4h_pixel(0.9f, 0.9f, 0.95f, 1.0f);
    draw_string_ab4h(canvas, "=== PARAMETER TOOL PALETTE ===", 820, 108, text_hdr);

    Ab4hPixel track_col = make_ab4h_pixel(0.2f, 0.2f, 0.22f, 1.0f);
    Ab4hPixel thumb_col = make_ab4h_pixel(1.0f, 1.0f, 1.0f, 1.0f);

    if (dl_game_active) {
        // Draw Dragon's Lair Storybook Panel
        Ab4hPixel panel_bg = make_ab4h_pixel(0.12f, 0.04f, 0.04f, 1.0f);
        draw_rect_ab4h(canvas, 820, 110, 370, 255, panel_bg);
        
        Ab4hPixel dl_title_col = make_ab4h_pixel(1.0f, 0.82f, 0.2f, 1.0f);
        draw_string_ab4h(canvas, "DRAGON'S LAIR STORYBOOK", 840, 130, dl_title_col);
        
        char stats_buf[128];
        snprintf(stats_buf, sizeof(stats_buf), "LIVES: %d/3 | SCORE: %d | STAGE: %d/5", dl_lives, dl_score, dl_stage);
        draw_string_ab4h(canvas, stats_buf, 840, 155, make_ab4h_pixel(1.0f, 0.35f, 0.35f, 1.0f));
        
        draw_string_ab4h(canvas, dl_stage_texts[dl_stage - 1], 840, 185, make_ab4h_pixel(0.9f, 0.9f, 0.95f, 1.0f));
        
        if (dl_status == 0) {
            char prompt_buf[128];
            snprintf(prompt_buf, sizeof(prompt_buf), "QTE Prompt: %s", 
                     (dl_expected_action == 1) ? "LEAP LEFT" :
                     (dl_expected_action == 2) ? "DODGE RIGHT" :
                     (dl_expected_action == 3) ? "USE SWORD" :
                     (dl_expected_action == 4) ? "JUMP UP" : "DUCK DOWN");
            draw_string_ab4h(canvas, prompt_buf, 840, 220, make_ab4h_pixel(1.0f, 1.0f, 0.2f, 1.0f));
            
            const char* act_names[6] = { "", "LEFT", "RIGHT", "SWORD", "UP", "DOWN" };
            for (int act = 1; act <= 5; act++) {
                int btn_x = 835 + (act - 1) * 70;
                Ab4hPixel act_btn_bg = make_ab4h_pixel(0.25f, 0.08f, 0.08f, 1.0f);
                draw_rect_ab4h(canvas, btn_x, 260, 65, 30, act_btn_bg);
                draw_string_ab4h(canvas, act_names[act], btn_x + 5, 270, make_ab4h_pixel(0.95f, 0.95f, 0.95f, 1.0f));
            }
        } else if (dl_status == 1) {
            draw_string_ab4h(canvas, "VICTORY! Princess Daphne rescued!", 840, 220, make_ab4h_pixel(0.2f, 0.9f, 0.2f, 1.0f));
            Ab4hPixel play_btn_bg = make_ab4h_pixel(0.08f, 0.35f, 0.08f, 1.0f);
            draw_rect_ab4h(canvas, 940, 260, 140, 30, play_btn_bg);
            draw_string_ab4h(canvas, "PLAY AGAIN", 965, 270, thumb_col);
        } else if (dl_status == 2) {
            draw_string_ab4h(canvas, "GAME OVER! Dirk has perished!", 840, 220, make_ab4h_pixel(0.9f, 0.2f, 0.2f, 1.0f));
            Ab4hPixel retry_btn_bg = make_ab4h_pixel(0.35f, 0.08f, 0.08f, 1.0f);
            draw_rect_ab4h(canvas, 940, 260, 140, 30, retry_btn_bg);
            draw_string_ab4h(canvas, "RETRY QUEST", 960, 270, thumb_col);
        }
    } else {
        // Slider 0: Fur Length
        char str_buf[128];
        snprintf(str_buf, sizeof(str_buf), "Fur Length: %.3f", fur_length);
        draw_string_ab4h(canvas, str_buf, 820, 100, text_hdr);
        draw_rect_ab4h(canvas, 850, 120, 350, 8, track_col);
        int fill_w0 = (int)(350 * (fur_length - 0.01f) / (0.20f - 0.01f));
        for (int tx = 0; tx < fill_w0; tx++) {
            float t = (float)tx / 350.0f;
            Ab4hPixel col = evaluate_palette(t);
            if (hover_slider0 || active_slider == 0) {
                float mult = 1.2f + haptic_rumble_amplitude * 2.5f;
                col.r = double_to_half(half_to_float(col.r) * mult);
                col.g = double_to_half(half_to_float(col.g) * mult);
                col.b = double_to_half(half_to_float(col.b) * mult);
            }
            draw_rect_ab4h(canvas, 850 + tx, 120, 1, 8, col);
        }
        draw_rect_ab4h(canvas, 850 + fill_w0 - 6, 114, 12, 20, thumb_col);

        // Slider 1: Scale
        snprintf(str_buf, sizeof(str_buf), "Scale: %.2f", scale_val);
        draw_string_ab4h(canvas, str_buf, 820, 150, text_hdr);
        draw_rect_ab4h(canvas, 850, 170, 350, 8, track_col);
        int fill_w1 = (int)(350 * (scale_val - 0.2f) / (2.0f - 0.2f));
        for (int tx = 0; tx < fill_w1; tx++) {
            float t = (float)tx / 350.0f;
            Ab4hPixel col = evaluate_palette(t);
            if (hover_slider1 || active_slider == 1) {
                float mult = 1.2f + haptic_rumble_amplitude * 2.5f;
                col.r = double_to_half(half_to_float(col.r) * mult);
                col.g = double_to_half(half_to_float(col.g) * mult);
                col.b = double_to_half(half_to_float(col.b) * mult);
            }
            draw_rect_ab4h(canvas, 850 + tx, 170, 1, 8, col);
        }
        draw_rect_ab4h(canvas, 850 + fill_w1 - 6, 164, 12, 20, thumb_col);

        // Slider 2: Light Angle
        snprintf(str_buf, sizeof(str_buf), "Light Angle: %.1f Deg", light_angle_deg);
        draw_string_ab4h(canvas, str_buf, 820, 200, text_hdr);
        draw_rect_ab4h(canvas, 850, 220, 350, 8, track_col);
        int fill_w2 = (int)(350 * (light_angle_deg / 360.0f));
        for (int tx = 0; tx < fill_w2; tx++) {
            float t = (float)tx / 350.0f;
            Ab4hPixel col = evaluate_palette(t);
            if (hover_slider2 || active_slider == 2) {
                float mult = 1.2f + haptic_rumble_amplitude * 2.5f;
                col.r = double_to_half(half_to_float(col.r) * mult);
                col.g = double_to_half(half_to_float(col.g) * mult);
                col.b = double_to_half(half_to_float(col.b) * mult);
            }
            draw_rect_ab4h(canvas, 850 + tx, 220, 1, 8, col);
        }
        draw_rect_ab4h(canvas, 850 + fill_w2 - 6, 214, 12, 20, thumb_col);

        // Slider 3: Wind Velocity
        snprintf(str_buf, sizeof(str_buf), "Wind Velocity: %.2f", wind_velocity);
        draw_string_ab4h(canvas, str_buf, 820, 250, text_hdr);
        draw_rect_ab4h(canvas, 850, 270, 350, 8, track_col);
        int fill_w3 = (int)(350 * (wind_velocity / 2.0f));
        for (int tx = 0; tx < fill_w3; tx++) {
            float t = (float)tx / 350.0f;
            Ab4hPixel col = evaluate_palette(t);
            if (hover_slider3 || active_slider == 3) {
                float mult = 1.2f + haptic_rumble_amplitude * 2.5f;
                col.r = double_to_half(half_to_float(col.r) * mult);
                col.g = double_to_half(half_to_float(col.g) * mult);
                col.b = double_to_half(half_to_float(col.b) * mult);
            }
            draw_rect_ab4h(canvas, 850 + tx, 270, 1, 8, col);
        }
        draw_rect_ab4h(canvas, 850 + fill_w3 - 6, 264, 12, 20, thumb_col);

        // Slider 4: Fur Stiffness
        snprintf(str_buf, sizeof(str_buf), "Fur Stiffness: %.2f", fur_stiffness);
        draw_string_ab4h(canvas, str_buf, 820, 300, text_hdr);
        draw_rect_ab4h(canvas, 850, 320, 350, 8, track_col);
        int fill_w4 = (int)(350 * (fur_stiffness - 0.1f) / (5.0f - 0.1f));
        for (int tx = 0; tx < fill_w4; tx++) {
            float t = (float)tx / 350.0f;
            Ab4hPixel col = evaluate_palette(t);
            if (hover_slider4 || active_slider == 4) {
                float mult = 1.2f + haptic_rumble_amplitude * 2.5f;
                col.r = double_to_half(half_to_float(col.r) * mult);
                col.g = double_to_half(half_to_float(col.g) * mult);
                col.b = double_to_half(half_to_float(col.b) * mult);
            }
            draw_rect_ab4h(canvas, 850 + tx, 320, 1, 8, col);
        }
        draw_rect_ab4h(canvas, 850 + fill_w4 - 6, 314, 12, 20, thumb_col);
    }

    // Action Buttons
    Ab4hPixel btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    Ab4hPixel btn_text = make_ab4h_pixel(0.85f, 0.85f, 0.9f, 1.0f);

    // Rendering Toggles
    Ab4hPixel ssaa_bg = opt_ssaa ? make_ab4h_pixel(0.25f, 0.15f, 0.35f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    if (hover_ssaa) {
        ssaa_bg.r = double_to_half(half_to_float(ssaa_bg.r) + 0.10f);
        ssaa_bg.g = double_to_half(half_to_float(ssaa_bg.g) + 0.10f);
        ssaa_bg.b = double_to_half(half_to_float(ssaa_bg.b) + 0.15f);
    }
    draw_rect_ab4h(canvas, 850, 350, 80, 30, ssaa_bg);
    draw_string_ab4h(canvas, opt_ssaa ? "SSAA: ON" : "SSAA: OFF", 855, 358, btn_text);

    Ab4hPixel vig_bg = opt_vignette ? make_ab4h_pixel(0.25f, 0.15f, 0.35f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    if (hover_vignette) {
        vig_bg.r = double_to_half(half_to_float(vig_bg.r) + 0.10f);
        vig_bg.g = double_to_half(half_to_float(vig_bg.g) + 0.10f);
        vig_bg.b = double_to_half(half_to_float(vig_bg.b) + 0.15f);
    }
    draw_rect_ab4h(canvas, 940, 350, 80, 30, vig_bg);
    draw_string_ab4h(canvas, opt_vignette ? "VIG: ON" : "VIG: OFF", 948, 358, btn_text);

    Ab4hPixel tone_bg = opt_tonemap ? make_ab4h_pixel(0.25f, 0.15f, 0.35f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    if (hover_tonemap) {
        tone_bg.r = double_to_half(half_to_float(tone_bg.r) + 0.10f);
        tone_bg.g = double_to_half(half_to_float(tone_bg.g) + 0.10f);
        tone_bg.b = double_to_half(half_to_float(tone_bg.b) + 0.15f);
    }
    draw_rect_ab4h(canvas, 1030, 350, 80, 30, tone_bg);
    draw_string_ab4h(canvas, opt_tonemap ? "TONE: ON" : "TONE: OFF", 1035, 358, btn_text);

    Ab4hPixel boost_bg = opt_viewport_boost ? make_ab4h_pixel(0.25f, 0.15f, 0.35f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    if (hover_viewport_boost) {
        boost_bg.r = double_to_half(half_to_float(boost_bg.r) + 0.10f);
        boost_bg.g = double_to_half(half_to_float(boost_bg.g) + 0.10f);
        boost_bg.b = double_to_half(half_to_float(boost_bg.b) + 0.15f);
    }
    draw_rect_ab4h(canvas, 1120, 350, 80, 30, boost_bg);
    draw_string_ab4h(canvas, opt_viewport_boost ? "BOOST: ON" : "BOOST: OFF", 1125, 358, btn_text);

    // Button 1: Randomize
    if (hover_btn1) btn_bg = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    else btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 850, 400, 160, 40, btn_bg);
    draw_string_ab4h(canvas, "RANDOMIZE", 880, 412, btn_text);

    // Button 2: Export PPM
    if (hover_btn2) btn_bg = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    else btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 1030, 400, 170, 40, btn_bg);
    draw_string_ab4h(canvas, "EXPORT PPM", 1060, 412, btn_text);

    // Button 3: Reset
    if (hover_btn3) btn_bg = make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f);
    else btn_bg = make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 850, 450, 160, 40, btn_bg);
    draw_string_ab4h(canvas, "RESET STATE", 880, 462, btn_text);

    // Button 4: Valve Preset Selector
    Ab4hPixel valve_btn_bg = hover_valve_btn ? make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 1030, 450, 170, 40, valve_btn_bg);
    char valve_lbl[64];
    snprintf(valve_lbl, sizeof(valve_lbl), "VALVE: %s", valve_names[selected_valve]);
    draw_string_ab4h(canvas, valve_lbl, 1040, 462, btn_text);

    // VLM Selector
    draw_string_ab4h(canvas, "VLM Critic:", 820, 510, text_hdr);
    Ab4hPixel vlm_btn_bg = hover_vlm_btn ? make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 960, 500, 240, 30, vlm_btn_bg);
    draw_string_ab4h(canvas, vlm_names[selected_vlm], 980, 508, btn_text);

    // Generator Selector
    draw_string_ab4h(canvas, "Generator:", 820, 550, text_hdr);
    Ab4hPixel gen_btn_bg = hover_gen_btn ? make_ab4h_pixel(0.2f, 0.2f, 0.28f, 1.0f) : make_ab4h_pixel(0.12f, 0.12f, 0.16f, 1.0f);
    draw_rect_ab4h(canvas, 960, 540, 240, 30, gen_btn_bg);
    draw_string_ab4h(canvas, generator_names[selected_generator], 980, 548, btn_text);

    // Run Optimizer & Dragon's Lair Buttons
    Ab4hPixel run_btn_bg = hover_run_btn ? make_ab4h_pixel(0.25f, 0.15f, 0.35f, 1.0f) : make_ab4h_pixel(0.15f, 0.08f, 0.25f, 1.0f);
    draw_rect_ab4h(canvas, 850, 580, 170, 25, run_btn_bg);
    draw_string_ab4h(canvas, "RUN OPTIMIZER", 880, 588, btn_text);

    Ab4hPixel dl_btn_bg = hover_dl_btn ? make_ab4h_pixel(0.35f, 0.15f, 0.15f, 1.0f) : (dl_game_active ? make_ab4h_pixel(0.25f, 0.08f, 0.08f, 1.0f) : make_ab4h_pixel(0.15f, 0.08f, 0.08f, 1.0f));
    draw_rect_ab4h(canvas, 1030, 580, 170, 25, dl_btn_bg);
    draw_string_ab4h(canvas, dl_game_active ? "EXIT LAIR" : "DRAGON'S LAIR", 1060, 588, btn_text);

    // Telemetry display (Ammeter and Voltmeter) & Sequencer Grid (7-Track, 8-Step)
    const char* seq_labels[7] = {"KICK", "SNAR", "TOMS", "HATS", "RIDE", "CLAP", "SNAP"};
    for (int t = 0; t < 7; t++) {
        float env = track_trigger_env[t];
        Ab4hPixel label_col = make_ab4h_pixel(0.7f + 0.3f * env, 0.7f + 0.1f * env, 0.8f - 0.5f * env, 1.0f);
        draw_string_ab4h(canvas, seq_labels[t], 812, 638 + t * 12, label_col);
    }
 
    for (int t = 0; t < 7; t++) {
        int y_pos = 635 + t * 12;
        float env = track_trigger_env[t];
        for (int s = 0; s < 8; s++) {
            int x_pos = 850 + s * 30;
            Ab4hPixel cell_bg;
            if (seq_grid[t][s]) {
                float pr = 0.1f + 0.5f * env;
                float pg = 0.6f + 0.3f * env;
                float pb = 0.1f;
                cell_bg = (seq_play_step == s) ? make_ab4h_pixel(0.2f, 0.9f, 0.2f, 1.0f) : make_ab4h_pixel(pr, pg, pb, 1.0f);
            } else {
                cell_bg = (seq_play_step == s) ? make_ab4h_pixel(0.3f, 0.3f, 0.4f, 1.0f) : make_ab4h_pixel(0.15f, 0.15f, 0.2f, 1.0f);
            }
            int h_offset = (env > 0.5f && seq_grid[t][s]) ? 2 : 0;
            draw_rect_ab4h(canvas, x_pos, y_pos - h_offset/2, 22, 10 + h_offset, cell_bg);
            
            if (seq_play_step == s) {
                Ab4hPixel border_col = make_ab4h_pixel(0.3f + 0.7f * env, 0.8f + 0.2f * env, 1.0f - 0.8f * env, 1.0f);
                draw_rect_ab4h(canvas, x_pos, y_pos, 22, 2, border_col);
                draw_rect_ab4h(canvas, x_pos, y_pos + 8, 22, 2, border_col);
            }
        }
    }

    // Sleek HUD overlay for Telemetry and Diagnostics in the viewport (bottom-left)
    draw_rect_ab4h(canvas, 10, 640, 480, 75, make_ab4h_pixel(0.0f, 0.0f, 0.0f, 0.6f));

    char telemetry_buf[256];
    float ammeter_val = 0.05f * (ammeter_T - 293.15f);
    snprintf(telemetry_buf, sizeof(telemetry_buf), "Telem: A=%.2fA V=%.1fV | TEDDY BEAR 303 ACTIVE (%s) | %s", ammeter_val, voltmeter_V, tb303_square_mode ? "SQ" : "SAW", opt_status);
    Ab4hPixel telemetry_col = make_ab4h_pixel(0.3f, 0.8f, 1.0f, 1.0f);
    draw_string_ab4h(canvas, telemetry_buf, 15, 650, telemetry_col);
 
    char status_buf[128];
    bool geniac_active = (opt_ssaa && opt_vignette) || (opt_tonemap || opt_viewport_boost);
    snprintf(status_buf, sizeof(status_buf), "GENIAC: %s | DIAG SEQ: %d/6 (%s)",
             geniac_active ? "CLOSED" : "OPEN",
             seq_current_step + 1,
             seq_validation_passed ? "OK" : "FAULT");
    draw_string_ab4h(canvas, status_buf, 15, 670, make_ab4h_pixel(0.9f, 0.7f, 0.2f, 1.0f));
 
    Ab4hPixel log_col = make_ab4h_pixel(0.5f, 0.85f, 0.5f, 1.0f);
    const char *prompt_fur = (fur_length > 0.14f) ? "dense" : ((fur_length < 0.05f) ? "short" : "fluffy");
    const char *prompt_size = (scale_val > 1.4f) ? "giant" : ((scale_val < 0.6f) ? "tiny" : "standard");
    char dynamic_prompt[128];
    snprintf(dynamic_prompt, sizeof(dynamic_prompt), "Prompt: 'a %s %s golden-brown' [1042, 9811]", prompt_fur, prompt_size);
    draw_string_ab4h(canvas, dynamic_prompt, 15, 690, log_col);
    // Update and draw particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (particles[i].active) {
            particles[i].x += particles[i].vx;
            particles[i].y += particles[i].vy;
            particles[i].vy += 0.2f; // gravity effect
            particles[i].life -= 0.04f; // fade out speed
            if (particles[i].life <= 0.0f) {
                particles[i].active = false;
            } else {
                int px = (int)particles[i].x;
                int py = (int)particles[i].y;
                if (px >= 820 && px < 1200 && py >= 0 && py < 720) {
                    Ab4hPixel p_col = make_ab4h_pixel(particles[i].r * particles[i].life, particles[i].g * particles[i].life, particles[i].b * particles[i].life, 1.0f);
                    draw_rect_ab4h(canvas, px - 1, py - 1, 3, 3, p_col);
                }
            }
        }
    }
}

// Downsamples AB4H 64-bit float canvas to standard 32-bit ARGB Wayland framebuffer
void present_ab4h_to_argb(TsfiAb4hMat *canvas, uint32_t *dest_argb) {
    for (int y = 0; y < canvas->rows; y++) {
        Ab4hPixel *row = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
        for (int x = 0; x < canvas->cols; x++) {
            float r = half_to_float(row[x].r);
            float g = half_to_float(row[x].g);
            float b = half_to_float(row[x].b);
            float a = half_to_float(row[x].a);

            if (x < 800) {
                // Filmic tone mapping / contrast adjustment
                r = r * (r * 0.17f + 0.83f);
                g = g * (g * 0.17f + 0.83f);
                b = b * (b * 0.17f + 0.83f);

                // Vignette effect (darkening towards corners)
                float uvx = (float)x / 800.0f;
                float uvy = (float)y / 720.0f;
                float vignette = uvx * uvy * (1.0f - uvx) * (1.0f - uvy);
                vignette = 16.0f * vignette;
                if (vignette < 0.0f) vignette = 0.0f;
                if (vignette > 1.0f) vignette = 1.0f;
                vignette = 0.6f + 0.4f * powf(vignette, 0.25f);
                
                r *= vignette;
                g *= vignette;
                b *= vignette;

                // CRT Scanlines and Scrolling Refresh Line
                static int scan_frame = 0;
                if (x == 0 && y == 0) {
                    scan_frame++;
                }
                float scanline = 0.93f + 0.07f * sinf((float)y * 1.6f);
                float scroll = sinf((float)y * 0.02f - (float)scan_frame * 0.03f);
                float bar = (scroll > 0.98f) ? 1.08f : (scroll > 0.95f ? 1.0f + 4.0f * (scroll - 0.95f) : 1.0f);
                r *= scanline * bar;
                g *= scanline * bar;
                b *= scanline * bar;
            }

            // Gamma correction approximation
            r = sqrtf(r);
            g = sqrtf(g);
            b = sqrtf(b);

            int ir = (int)(r * 255.0f);
            int ig = (int)(g * 255.0f);
            int ib = (int)(b * 255.0f);
            int ia = (int)(a * 255.0f);

            if (ir < 0) {
                ir = 0;
            }
            if (ir > 255) {
                ir = 255;
            }
            if (ig < 0) {
                ig = 0;
            }
            if (ig > 255) {
                ig = 255;
            }
            if (ib < 0) {
                ib = 0;
            }
            if (ib > 255) {
                ib = 255;
            }
            if (ia < 0) {
                ia = 0;
            }
            if (ia > 255) {
                ia = 255;
            }

            dest_argb[y * canvas->cols + x] = (ia << 24) | (ir << 16) | (ig << 8) | ib;
        }
    }
}

static void export_ppm_real(TsfiAb4hMat *canvas) {
    const char *out_path = "/home/mariarahel/.gemini/antigravity-cli/brain/dc445656-3da0-44e3-be2f-cae81a8b8170/scratch/photorealistic_teddy_bear.ppm";
    FILE *fp = fopen(out_path, "wb");
    if (fp) {
        fprintf(fp, "P6\n%d %d\n255\n", 1280, 720);
        for (int y = 0; y < 720; y++) {
            Ab4hPixel *row = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
            for (int x = 0; x < 1280; x++) {
                float r = half_to_float(row[x].r);
                float g = half_to_float(row[x].g);
                float b = half_to_float(row[x].b);
                
                int ir = (int)(r * 255.0f);
                int ig = (int)(g * 255.0f);
                int ib = (int)(b * 255.0f);
                
                if (ir < 0) ir = 0;
                if (ir > 255) ir = 255;
                if (ig < 0) ig = 0;
                if (ig > 255) ig = 255;
                if (ib < 0) ib = 0;
                if (ib > 255) ib = 255;

                fputc(ir, fp);
                fputc(ig, fp);
                fputc(ib, fp);
            }
        }
        fclose(fp);
        printf("[SUCCESS] Exported frame to %s\n", out_path);
    }
}

// Executes Object Recognition validation on the viewport of the canvas to identify the teddy bear structures
void validate_rendering_via_object_recognition(TsfiAb4hMat *canvas) {
    printf("[VISION] Executing Object Recognition on viewport...\n");
    int w = 800;
    int h = 720;
    Ab4hPixel *viewport = malloc(w * h * sizeof(Ab4hPixel));
    assert(viewport != NULL);
    
    // Copy viewport region (left 800x720)
    for (int y = 0; y < h; y++) {
        Ab4hPixel *src = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
        memcpy(&viewport[y * w], src, w * sizeof(Ab4hPixel));
    }
    
    TSFiFlowerPhenotype dummy = {0};
    TSFiResonanceAnalysis analysis = {0};
    tsfi_vision_analyze_staging_ab4h((const uint16_t *)viewport, w, h, &dummy, &analysis);
    
    // Validate visual signature characteristics of the teddy bear
    bool valid = true;
    if (analysis.coverage < 0.10f) {
        printf("  [FAIL] Visual Coverage too low: %.4f (min: 0.10f)\n", analysis.coverage);
        valid = false;
    }
    if (analysis.glyph_symmetry < 0.60f) {
        printf("  [FAIL] Vertical symmetry regression: %.4f (min: 0.60f)\n", analysis.glyph_symmetry);
        valid = false;
    }
    if (analysis.complexity < 0.20f) {
        printf("  [FAIL] Fur shell texture complexity too low: %.4f (min: 0.20f)\n", analysis.complexity);
        valid = false;
    }
    if (analysis.specular_contrast < 3.0f) {
        printf("  [FAIL] Specular Contrast too low: %.4f (min: 3.0f)\n", analysis.specular_contrast);
        valid = false;
    }
    if (analysis.surface_grain < 0.01f) {
        printf("  [FAIL] High-frequency surface grain too low: %.4f (min: 0.01f)\n", analysis.surface_grain);
        valid = false;
    }
    if (analysis.center_mass_x < 0.30f || analysis.center_mass_x > 0.60f) {
        printf("  [FAIL] Horizontal framing centering error: %.4f (target: 0.30f - 0.60f)\n", analysis.center_mass_x);
        valid = false;
    }
    if (analysis.chromatic_balance < 1.20f) {
        printf("  [FAIL] Chromatic balance too low: %.4f (min: 1.20f)\n", analysis.chromatic_balance);
        valid = false;
    }
    if (analysis.center_mass_y < 0.30f || analysis.center_mass_y > 0.70f) {
        printf("  [FAIL] Vertical framing centering error: %.4f (target: 0.30f - 0.70f)\n", analysis.center_mass_y);
        valid = false;
    }
    if (analysis.rim_intensity < 0.80f) {
        printf("  [FAIL] Peak specular/rim highlight intensity too low: %.4f (min: 0.80f)\n", analysis.rim_intensity);
        valid = false;
    }
    if (analysis.avg_intensity < 0.05f || analysis.avg_intensity > 0.40f) {
        printf("  [FAIL] Average luminance intensity out of bounds: %.4f (target: 0.05f - 0.40f)\n", analysis.avg_intensity);
        valid = false;
    }
    
    if (valid) {
        float confidence = 0.5f + (analysis.glyph_symmetry - 0.65f) * 1.5f;
        if (confidence > 0.99f) confidence = 0.99f;
        printf("  [PASS] Object recognized: TSFI_CLASS_TEDDY (Confidence: %.2f)\n", confidence);
        printf("         Symmetry: %.4f, Coverage: %.4f, Complexity: %.4f, Avg Intensity: %.4f\n",
               analysis.glyph_symmetry, analysis.coverage, analysis.complexity, analysis.avg_intensity);
        printf("         Contrast: %.4f, Grain: %.4f, Center X: %.4f, Center Y: %.4f\n",
               analysis.specular_contrast, analysis.surface_grain, analysis.center_mass_x, analysis.center_mass_y);
        printf("         Chroma Balance: %.4f, Rim/Peak Highlight: %.4f\n",
               analysis.chromatic_balance, analysis.rim_intensity);
    } else {
        printf("  [FAIL] Object Recognition failed to identify a valid Teddy Bear structure.\n");
    }
    
    free(viewport);
}

// Wayland pointer interface callbacks
static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *wl_surface, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)data; (void)wl_pointer; (void)serial; (void)wl_surface; (void)surface_x; (void)surface_y;
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer, uint32_t serial, struct wl_surface *wl_surface) {
    (void)data; (void)wl_pointer; (void)serial; (void)wl_surface;
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer, uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)data; (void)wl_pointer; (void)time;
    double target_x = (double)wl_fixed_to_int(surface_x);
    double target_y = (double)wl_fixed_to_int(surface_y);

    if (!mouse_valves_initialized) {
        tsfi_valve_init(&mouse_valve_x, 15.0, 0.0003, 1280.0, -0.5);
        tsfi_valve_init(&mouse_valve_y, 15.0, 0.0003, 720.0, -0.5);
        mouse_valve_x.R_plate = 100000.0;
        mouse_valve_y.R_plate = 100000.0;
        mouse_valve_x.state_vp = target_x;
        mouse_valve_y.state_vp = target_y;
        mouse_sim_x = target_x;
        mouse_sim_y = target_y;
        mouse_valves_initialized = true;
    }

    float vg_in_x[16];
    float vg_in_y[16];
    for (int k = 0; k < 16; k++) {
        vg_in_x[k] = -((float)target_x / 1280.0f * 2.0f - 1.0f);
        vg_in_y[k] = -((float)target_y / 720.0f * 2.0f - 1.0f);
    }

    float vp_out_x[16];
    float vp_out_y[16];

    tsfi_valve_process_differential_feedback(&mouse_valve_x, vg_in_x, vp_out_x, 16, 0.0, 1.0, 0.001, 0.00002, 0.0002, 50000.0, 0.35);
    tsfi_valve_process_differential_feedback(&mouse_valve_y, vg_in_y, vp_out_y, 16, 0.0, 1.0, 0.001, 0.00002, 0.0002, 50000.0, 0.35);

    if (force_procedural_rendering) {
        mouse_x = (int)target_x;
        mouse_y = (int)target_y;
        mouse_sim_x = (float)target_x;
        mouse_sim_y = (float)target_y;
        haptic_rumble_amplitude = 0.0f;
        haptic_rumble_frequency = 0.0f;
    } else {
        mouse_sim_x = vp_out_x[15];
        mouse_sim_y = vp_out_y[15];

        mouse_x = (int)mouse_sim_x;
        mouse_y = (int)mouse_sim_y;

        float mean_vp = 0.0f;
        for (int k = 0; k < 16; k++) {
            mean_vp += vp_out_x[k];
        }
        mean_vp /= 16.0f;

        float variance = 0.0f;
        int zero_crossings = 0;
        float prev_dev = vp_out_x[0] - mean_vp;
        for (int k = 0; k < 16; k++) {
            float dev = vp_out_x[k] - mean_vp;
            variance += dev * dev;
            if ((prev_dev < 0.0f && dev >= 0.0f) || (prev_dev > 0.0f && dev <= 0.0f)) {
                zero_crossings++;
            }
            prev_dev = dev;
        }
        variance /= 16.0f;

        haptic_rumble_amplitude = sqrtf(variance) / 1280.0f;
        haptic_rumble_frequency = (float)zero_crossings * 60.0f;
    }

    // Recalculate hover states
    hover_slider0 = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 135 && mouse_y <= 165);
    hover_slider1 = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 235 && mouse_y <= 265);
    hover_slider2 = (mouse_x >= 850 && mouse_x <= 1200 && mouse_y >= 335 && mouse_y <= 365);
    hover_ssaa = (mouse_y >= 380 && mouse_y <= 410 && mouse_x >= 850 && mouse_x <= 930);
    hover_vignette = (mouse_y >= 380 && mouse_y <= 410 && mouse_x >= 940 && mouse_x <= 1020);
    hover_tonemap = (mouse_y >= 380 && mouse_y <= 410 && mouse_x >= 1030 && mouse_x <= 1110);
    hover_viewport_boost = (mouse_y >= 380 && mouse_y <= 410 && mouse_x >= 1120 && mouse_x <= 1200);
    hover_btn1 = (mouse_y >= 430 && mouse_y <= 470 && mouse_x >= 850 && mouse_x <= 1010);
    hover_btn2 = (mouse_y >= 430 && mouse_y <= 470 && mouse_x >= 1030 && mouse_x <= 1200);
    hover_btn3 = (mouse_y >= 480 && mouse_y <= 520 && mouse_x >= 850 && mouse_x <= 1010);
    hover_valve_btn = (mouse_y >= 480 && mouse_y <= 520 && mouse_x >= 1030 && mouse_x <= 1200);
    hover_vlm_btn = (mouse_x >= 960 && mouse_x <= 1200 && mouse_y >= 530 && mouse_y <= 560);
    hover_gen_btn = (mouse_x >= 960 && mouse_x <= 1200 && mouse_y >= 570 && mouse_y <= 600);
    hover_run_btn = (mouse_x >= 850 && mouse_x <= 1020 && mouse_y >= 605 && mouse_y <= 630);
    hover_dl_btn = (mouse_x >= 1030 && mouse_x <= 1200 && mouse_y >= 605 && mouse_y <= 630);

    if (mouse_pressed && active_slider != -1) {
        float pct = (float)(mouse_x - 850) / 350.0f;
        if (pct < 0.0f) pct = 0.0f;
        if (pct > 1.0f) pct = 1.0f;

        if (active_slider == 0) {
            fur_length = 0.01f + pct * (0.20f - 0.01f);
        } else if (active_slider == 1) {
            scale_val = 0.2f + pct * (2.0f - 0.2f);
        } else if (active_slider == 2) {
            light_angle_deg = pct * 360.0f;
        }
        display_synthesized_image = false;
    }
}

static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state) {
    (void)data; (void)wl_pointer; (void)serial; (void)time;
    if (button == BTN_LEFT) {
        if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
            mouse_pressed = true;

            // Check click on step sequencer grid (Kick/Snare/Toms/Hats/Ride/Clap/Snap tracks)
            for (int t = 0; t < 7; t++) {
                int y_start = 635 + t * 12;
                if (mouse_y >= y_start && mouse_y <= y_start + 10) {
                    for (int s = 0; s < 8; s++) {
                        int x_start = 850 + s * 30;
                        if (mouse_x >= x_start && mouse_x <= x_start + 22) {
                            seq_grid[t][s] ^= 1;
                        }
                    }
                }
            }
            if (!dl_game_active && mouse_x >= 850 && mouse_x <= 1200) {
                if (mouse_y >= 135 && mouse_y <= 165) {
                    active_slider = 0;
                } else if (mouse_y >= 235 && mouse_y <= 265) {
                    active_slider = 1;
                } else if (mouse_y >= 335 && mouse_y <= 365) {
                    active_slider = 2;
                }
            }
            
            if (active_slider != -1) {
                float pct = (float)(mouse_x - 850) / 350.0f;
                if (pct < 0.0f) pct = 0.0f;
                if (pct > 1.0f) pct = 1.0f;
                if (active_slider == 0) {
                    fur_length = 0.01f + pct * (0.20f - 0.01f);
                } else if (active_slider == 1) {
                    scale_val = 0.2f + pct * (2.0f - 0.2f);
                } else if (active_slider == 2) {
                    light_angle_deg = pct * 360.0f;
                }
                display_synthesized_image = false;
            }

            if (mouse_y >= 380 && mouse_y <= 410) {
                if (mouse_x >= 850 && mouse_x <= 930) {
                    opt_ssaa = !opt_ssaa;
                } else if (mouse_x >= 940 && mouse_x <= 1020) {
                    opt_vignette = !opt_vignette;
                } else if (mouse_x >= 1030 && mouse_x <= 1110) {
                    opt_tonemap = !opt_tonemap;
                } else if (mouse_x >= 1120 && mouse_x <= 1200) {
                    opt_viewport_boost = !opt_viewport_boost;
                }
            }
            
            if (mouse_y >= 430 && mouse_y <= 470) {
                if (mouse_x >= 850 && mouse_x <= 1010) {
                    fur_length = 0.02f + (float)(rand() % 100) / 100.0f * 0.16f;
                    scale_val = 0.4f + (float)(rand() % 100) / 100.0f * 1.4f;
                    light_angle_deg = (float)(rand() % 360);
                    display_synthesized_image = false;
                } else if (mouse_x >= 1030 && mouse_x <= 1200) {
                    active_slider = -2;
                }
            } else if (mouse_y >= 480 && mouse_y <= 520) {
                if (mouse_x >= 850 && mouse_x <= 1010) {
                    fur_length = 0.08f;
                    scale_val = 1.0f;
                    light_angle_deg = 135.0f;
                    display_synthesized_image = false;
                } else if (mouse_x >= 1030 && mouse_x <= 1200) {
                    selected_valve = (selected_valve + 1) % 11;
                    printf("  -> Cycled Valve Preset: %d (%s)\n", selected_valve, valve_names[selected_valve]);
                    precompute_all_sounds();
                }
            } else if (mouse_y >= 530 && mouse_y <= 560 && mouse_x >= 960 && mouse_x <= 1200) {
                selected_vlm = (selected_vlm + 1) % 6;
                char run_cmd[512];
                snprintf(run_cmd, sizeof(run_cmd), "python3 ../scripts/genetic_teddy_optimizer.py \"golden\" --vlm %s --generator %s > /tmp/vulkan_optimizer.log 2>&1 &", 
                         vlm_names[selected_vlm], generator_names[selected_generator]);
                int ret = system(run_cmd);
                if (ret == 0) {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: RUNNING (Background)");
                } else {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: LAUNCH FAILED");
                }
            } else if (mouse_y >= 570 && mouse_y <= 600 && mouse_x >= 960 && mouse_x <= 1200) {
                selected_generator = (selected_generator + 1) % 6;
                char run_cmd[512];
                snprintf(run_cmd, sizeof(run_cmd), "python3 ../scripts/genetic_teddy_optimizer.py \"golden\" --vlm %s --generator %s > /tmp/vulkan_optimizer.log 2>&1 &", 
                         vlm_names[selected_vlm], generator_names[selected_generator]);
                int ret = system(run_cmd);
                if (ret == 0) {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: RUNNING (Background)");
                } else {
                    snprintf(opt_status, sizeof(opt_status), "Optimizer Status: LAUNCH FAILED");
                }
            }
            if (dl_game_active && mouse_y >= 260 && mouse_y <= 290) {
                if (dl_status == 0) {
                    int clicked_action = 0;
                    if (mouse_x >= 835 && mouse_x <= 900) clicked_action = 1;
                    else if (mouse_x >= 905 && mouse_x <= 970) clicked_action = 2;
                    else if (mouse_x >= 975 && mouse_x <= 1040) clicked_action = 3;
                    else if (mouse_x >= 1045 && mouse_x <= 1110) clicked_action = 4;
                    else if (mouse_x >= 1115 && mouse_x <= 1180) clicked_action = 5;

                    if (clicked_action > 0) {
                        if (clicked_action == 1 || clicked_action == 2) {
                            play_synth_sound("dodge");
                        } else if (clicked_action == 3) {
                            play_synth_sound("sword");
                        } else if (clicked_action == 4 || clicked_action == 5) {
                            play_synth_sound("leap");
                        }

                        if (clicked_action == dl_expected_action) {
                            dl_score += 1500;
                            dl_stage++;
                            play_synth_sound("success");
                            if (dl_stage == 6) {
                                dl_status = 1;
                            } else {
                                dl_expected_action = dl_stage_actions[dl_stage - 1];
                            }
                        } else {
                            dl_lives--;
                            play_synth_sound("fail");
                            if (dl_lives == 0) {
                                dl_status = 2;
                            }
                        }
                    }
                } else {
                    if (mouse_x >= 940 && mouse_x <= 1080) {
                        init_dl_game();
                    }
                }
            } else if (mouse_y >= 605 && mouse_y <= 630) {
                if (mouse_x >= 850 && mouse_x <= 1020) {
                    char run_cmd[512];
                    snprintf(run_cmd, sizeof(run_cmd), "python3 ../scripts/genetic_teddy_optimizer.py \"golden\" --vlm %s --generator %s > /tmp/vulkan_optimizer.log 2>&1 &", 
                             vlm_names[selected_vlm], generator_names[selected_generator]);
                    int ret = system(run_cmd);
                    if (ret == 0) {
                        snprintf(opt_status, sizeof(opt_status), "Optimizer Status: RUNNING (Background)");
                    } else {
                        snprintf(opt_status, sizeof(opt_status), "Optimizer Status: LAUNCH FAILED");
                    }
                } else if (mouse_x >= 1030 && mouse_x <= 1200) {
                    dl_game_active = !dl_game_active;
                    if (dl_game_active) {
                        init_dl_game();
                    }
                }
            }
        } else {
            mouse_pressed = false;
            active_slider = -1;
        }
    }
}

static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value) {
    (void)data; (void)wl_pointer; (void)time; (void)axis; (void)value;
}
static void pointer_handle_frame(void *data, struct wl_pointer *wl_pointer) {
    (void)data; (void)wl_pointer;
}
static void pointer_handle_axis_source(void *data, struct wl_pointer *wl_pointer, uint32_t axis_source) {
    (void)data; (void)wl_pointer; (void)axis_source;
}
static void pointer_handle_axis_stop(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis) {
    (void)data; (void)wl_pointer; (void)time; (void)axis;
}
static void pointer_handle_axis_discrete(void *data, struct wl_pointer *wl_pointer, uint32_t axis, int32_t discrete) {
    (void)data; (void)wl_pointer; (void)axis; (void)discrete;
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
    .frame = pointer_handle_frame,
    .axis_source = pointer_handle_axis_source,
    .axis_stop = pointer_handle_axis_stop,
    .axis_discrete = pointer_handle_axis_discrete
};

static bool window_running = true;

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct wl_seat *seat = NULL;
static struct wl_pointer *pointer = NULL;
static struct wl_keyboard *keyboard = NULL;
static struct xdg_wm_base *xdg_wm_base = NULL;
static struct xdg_surface *xdg_surface = NULL;
static struct xdg_toplevel *xdg_toplevel = NULL;
static struct wl_surface *surface = NULL;
static struct wl_buffer *wl_buf = NULL;

static void keyboard_handle_keymap(void *data, struct wl_keyboard *kb, uint32_t format, int32_t fd, uint32_t size) {
    (void)data; (void)kb; (void)format; (void)fd; (void)size;
    close(fd);
}
static void keyboard_handle_enter(void *data, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surf, struct wl_array *keys) {
    (void)data; (void)kb; (void)serial; (void)surf; (void)keys;
}
static void keyboard_handle_leave(void *data, struct wl_keyboard *kb, uint32_t serial, struct wl_surface *surf) {
    (void)data; (void)kb; (void)serial; (void)surf;
}
static void keyboard_handle_key(void *data, struct wl_keyboard *kb, uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)data; (void)kb; (void)serial; (void)time;
    if (key == KEY_ESC && state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        printf("\n[WAYLAND] ESC key detected via seat keyboard. Closing window.\n");
        window_running = false;
    }
    if (key == KEY_W && state == WL_KEYBOARD_KEY_STATE_PRESSED) {
        tb303_square_mode = !tb303_square_mode;
        printf("  -> TB-303 Waveform Toggled: %s\n", tb303_square_mode ? "SQUARE" : "SAWTOOTH");
        display_synthesized_image = false;
    }
}
static void keyboard_handle_modifiers(void *data, struct wl_keyboard *kb, uint32_t serial, uint32_t mods_depended, uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)data; (void)kb; (void)serial; (void)mods_depended; (void)mods_latched; (void)mods_locked; (void)group;
}
static void keyboard_handle_repeat_info(void *data, struct wl_keyboard *kb, int32_t rate, int32_t delay) {
    (void)data; (void)kb; (void)rate; (void)delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_handle_keymap,
    .enter = keyboard_handle_enter,
    .leave = keyboard_handle_leave,
    .key = keyboard_handle_key,
    .modifiers = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info
};

static void seat_handle_capabilities(void *data, struct wl_seat *s, uint32_t caps) {
    (void)data;
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
        pointer = wl_seat_get_pointer(s);
        wl_pointer_add_listener(pointer, &pointer_listener, NULL);
    }
    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard) {
        keyboard = wl_seat_get_keyboard(s);
        wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
    }
}
static void seat_handle_name(void *data, struct wl_seat *s, const char *name) {
    (void)data; (void)s; (void)name;
}
static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name = seat_handle_name
};

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xwm, uint32_t serial) {
    (void)data;
    xdg_wm_base_pong(xwm, serial);
}
static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping
};

static void xdg_surface_handle_configure(void *data, struct xdg_surface *xs, uint32_t serial) {
    (void)data;
    xdg_surface_ack_configure(xs, serial);
}
static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure
};

static void xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xt, int32_t width, int32_t height, struct wl_array *states) {
    (void)data; (void)xt; (void)width; (void)height; (void)states;
}
static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xt) {
    (void)data; (void)xt;
    window_running = false;
}
static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close
};

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version) {
    (void)data; (void)version;
    if (strcmp(interface, "wl_compositor") == 0) {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    } else if (strcmp(interface, "wl_shm") == 0) {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        seat = wl_registry_bind(registry, name, &wl_seat_interface, 1);
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        xdg_wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    }
}
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name) {
    (void)data; (void)registry; (void)name;
}
static const struct wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove
};

typedef struct {
    uint8_t fur_r;
    uint8_t fur_g;
    uint8_t fur_b;
    uint8_t eye_r;
    uint8_t eye_g;
    uint8_t eye_b;
    uint8_t base_sickness;
    uint8_t base_scale;
    uint8_t base_fur_length;
    uint8_t light_angle_deg;
    uint8_t breathing_freq;
    uint8_t twitch_intensity;
} TsfiTeddyDna;

static void reload_genome() {
    FILE *df = fopen("assets/bear_genome.dna", "rb");
    if (df) {
        TsfiTeddyDna dna;
        if (fread(&dna, sizeof(TsfiTeddyDna), 1, df) == 1) {
            fur_length = (float)dna.base_fur_length / 1000.0f;
            scale_val = (float)dna.base_scale / 100.0f;
            light_angle_deg = (float)dna.light_angle_deg / 255.0f * 360.0f;
            breathing_freq = (float)dna.breathing_freq / 128.0f;
            twitch_intensity = (float)dna.twitch_intensity / 255.0f;
            sickness_intensity = (float)dna.base_sickness / 255.0f;
            
            dna_fur_r = (float)dna.fur_r / 255.0f;
            dna_fur_g = (float)dna.fur_g / 255.0f;
            dna_fur_b = (float)dna.fur_b / 255.0f;
            dna_eye_r = (float)dna.eye_r / 255.0f;
            dna_eye_g = (float)dna.eye_g / 255.0f;
            dna_eye_b = (float)dna.eye_b / 255.0f;
        }
        fclose(df);
    }
}

static uint64_t compute_viewport_pixel_sum(TsfiAb4hMat *canvas) {
    uint64_t sum = 0;
    for (int y = 0; y < 720; y++) {
        for (int x = 0; x < 800; x++) {
            Ab4hPixel *p = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride) + x;
            sum += (uint64_t)p->r + (uint64_t)p->g + (uint64_t)p->b;
        }
    }
    return sum;
}

static TSFiResonanceAnalysis get_visual_metrics(TsfiAb4hMat *canvas) {
    int w = 800;
    int h = 720;
    Ab4hPixel *viewport = malloc(w * h * sizeof(Ab4hPixel));
    assert(viewport != NULL);
    for (int y = 0; y < h; y++) {
        Ab4hPixel *src = (Ab4hPixel *)((char *)canvas->data + y * canvas->stride);
        memcpy(&viewport[y * w], src, w * sizeof(Ab4hPixel));
    }
    TSFiFlowerPhenotype dummy = {0};
    TSFiResonanceAnalysis analysis = {0};
    tsfi_vision_analyze_staging_ab4h((const uint16_t *)viewport, w, h, &dummy, &analysis);
    free(viewport);
    return analysis;
}

static void run_ui_self_tests(TsfiAb4hMat *canvas) {
    printf("=== Starting Programmatic UI Self-Test Suite ===\n");

    // Reset values first
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;
    selected_vlm = 0;
    selected_generator = 0;
    force_procedural_rendering = true;
    display_synthesized_image = false;
    
    // Render initial frame and compute default pixel sum and visual metrics
    render_frame(canvas, 0);
    uint64_t initial_sum = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis initial_metrics = get_visual_metrics(canvas);
    printf("  -> Initial viewport pixel sum: %lu\n", initial_sum);
    printf("  -> Initial visual metrics: coverage=%.4f, symmetry=%.4f, complexity=%.4f\n",
           initial_metrics.coverage, initial_metrics.glyph_symmetry, initial_metrics.complexity);
    
    // 1. Test Slider 0: Fur length
    printf("[Test] Testing Slider 0 (Fur Length)...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(150));
    assert(hover_slider0 && "Hover Slider 0 failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == 0 && "Active slider 0 activation failed");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1025), wl_fixed_from_int(150));
    printf("  -> Fur length after dragging to 50%%: %f\n", fur_length);
    assert(fur_length > 0.09f && fur_length < 0.12f && "Fur length slider drag calculation incorrect");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
    assert(!mouse_pressed && "Mouse release failed");
    
    render_frame(canvas, 0);
    uint64_t sum_after_slider0 = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis metrics_after_slider0 = get_visual_metrics(canvas);
    printf("  -> Pixel sum after Slider 0 (Fur Length) move: %lu\n", sum_after_slider0);
    printf("  -> Visual metrics after Slider 0: coverage=%.4f, symmetry=%.4f, complexity=%.4f\n",
           metrics_after_slider0.coverage, metrics_after_slider0.glyph_symmetry, metrics_after_slider0.complexity);
    assert(sum_after_slider0 != initial_sum && "Slider 0 failed to update the rendered image");
    assert((metrics_after_slider0.complexity != initial_metrics.complexity || metrics_after_slider0.coverage != initial_metrics.coverage) &&
           "Visual metrics failed to react to fur length modifications");

    // Reset to defaults
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;

    // 2. Test Slider 1: Scale
    printf("[Test] Testing Slider 1 (Scale)...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(250));
    assert(hover_slider1 && "Hover Slider 1 failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == 1 && "Active slider 1 activation failed");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1025), wl_fixed_from_int(250));
    printf("  -> Scale value after dragging to 50%%: %f\n", scale_val);
    assert(scale_val > 0.9f && scale_val < 1.3f && "Scale slider drag calculation incorrect");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    render_frame(canvas, 0);
    uint64_t sum_after_slider1 = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis metrics_after_slider1 = get_visual_metrics(canvas);
    printf("  -> Pixel sum after Slider 1 (Scale) move: %lu\n", sum_after_slider1);
    printf("  -> Visual metrics after Slider 1: coverage=%.4f, symmetry=%.4f, complexity=%.4f\n",
           metrics_after_slider1.coverage, metrics_after_slider1.glyph_symmetry, metrics_after_slider1.complexity);
    assert(sum_after_slider1 != initial_sum && "Slider 1 failed to update the rendered image");
    assert(metrics_after_slider1.coverage > 0.0f && "Visual coverage must be non-zero");

    // Reset to defaults
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;

    // 3. Test Slider 2: Light angle
    printf("[Test] Testing Slider 2 (Light Angle)...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(350));
    assert(hover_slider2 && "Hover Slider 2 failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == 2 && "Active slider 2 activation failed");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1025), wl_fixed_from_int(350));
    printf("  -> Light angle after dragging to 50%%: %f\n", light_angle_deg);
    assert(light_angle_deg > 170.0f && light_angle_deg < 190.0f && "Light angle slider drag calculation incorrect");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    render_frame(canvas, 0);
    uint64_t sum_after_slider2 = compute_viewport_pixel_sum(canvas);
    TSFiResonanceAnalysis metrics_after_slider2 = get_visual_metrics(canvas);
    printf("  -> Pixel sum after Slider 2 (Light Angle) move: %lu\n", sum_after_slider2);
    printf("  -> Visual metrics after Slider 2: coverage=%.4f, symmetry=%.4f, complexity=%.4f, avg_intensity=%.4f\n",
           metrics_after_slider2.coverage, metrics_after_slider2.glyph_symmetry, metrics_after_slider2.complexity, metrics_after_slider2.avg_intensity);
    assert(sum_after_slider2 != initial_sum && "Slider 2 failed to update the rendered image");
    assert(metrics_after_slider2.avg_intensity != initial_metrics.avg_intensity && "Average pixel intensity did not react to light angle changes");

    // Reset to defaults
    fur_length = 0.08f;
    scale_val = 1.0f;
    light_angle_deg = 135.0f;

    // 4. Test "Randomize" Button
    printf("[Test] Testing Randomize Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(450));
    assert(hover_btn1 && "Hover Randomize button failed");
    float old_fur = fur_length;
    float old_scale = scale_val;
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Randomized Fur: %f, Scale: %f\n", fur_length, scale_val);
    assert((fur_length != old_fur || scale_val != old_scale) && "Randomize failed to mutate values");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 5. Test "Reset/Analyze" Button (which sets active_slider = -2)
    printf("[Test] Testing Reset/Analyze Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1100), wl_fixed_from_int(450));
    assert(hover_btn2 && "Hover Reset/Analyze button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(active_slider == -2 && "Reset/Analyze button press failed to trigger action");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 6. Test "Default" Button
    printf("[Test] Testing Default Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(500));
    assert(hover_btn3 && "Hover Default button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    assert(fur_length == 0.08f && scale_val == 1.0f && light_angle_deg == 135.0f && "Default button failed to reset values");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 7. Test VLM Critic Selection Cycle
    printf("[Test] Testing VLM Critic Cycle...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(545));
    assert(hover_vlm_btn && "Hover VLM button failed");
    int old_vlm = selected_vlm;
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Cycled VLM: %d (was %d)\n", selected_vlm, old_vlm);
    assert(selected_vlm == (old_vlm + 1) % 6 && "VLM Cycle button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 8. Test Image Generator Cycle
    printf("[Test] Testing Image Generator Cycle...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(585));
    assert(hover_gen_btn && "Hover Generator button failed");
    int old_gen = selected_generator;
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Cycled Generator: %d (was %d)\n", selected_generator, old_gen);
    assert(selected_generator == (old_gen + 1) % 6 && "Generator Cycle button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 9. Test Trigger Genetic Optimizer Button (should start optimizer command line process)
    printf("[Test] Testing Trigger Genetic Optimizer Button...\n");
    pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(630));
    assert(hover_run_btn && "Hover Trigger Genetic Optimizer button failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
    printf("  -> Optimizer status line: '%s'\n", opt_status);
    assert(strcmp(opt_status, "Optimizer Status: LAUNCH FAILED") != 0 && "Optimizer launch failed");
    pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);

    // 10. Run render frame to verify everything displays correctly under tested state
    printf("[Test] Exercising render frame layout...\n");
    render_frame(canvas, 0);

    force_procedural_rendering = false;

    printf("=== ALL UI TESTS PASSED SUCCESSFULLY ===\n");
}

int main(int argc, char *argv[]) {
    printf("=== TSFi Photorealistic Teddy Bear Generator ===\n");
    
    // Load small 12-byte procedural bear DNA genome
    reload_genome();
    
    precompute_all_sounds();
    start_audio_mixer();

    if (argc > 1 && strcmp(argv[1], "--test") == 0) {
        int W = 1280;
        int H = 720;
        int stride = W * sizeof(Ab4hPixel);
        size_t sz = H * stride;
        void *offscreen_buf = calloc(1, sz);
        assert(offscreen_buf != NULL);
        TsfiAb4hMat canvas = { .rows = H, .cols = W, .stride = stride, .data = (Ab4hPixel *)offscreen_buf };
        run_ui_self_tests(&canvas);
        free(offscreen_buf);
        return 0;
    }

    // Verify standard OBJ loader imports a bear model
    printf("[OBJ] Verifying standard 3D mesh loader...\n");
    TsfiObjMesh *bear_mesh = tsfi_obj_load("assets/teddy_bear.obj");
    assert(bear_mesh != NULL && "Failed to load assets/teddy_bear.obj");
    assert(bear_mesh->vertex_count == 2601 && "Unexpected vertex count loaded from teddy bear");
    assert(bear_mesh->face_count == 4608 && "Unexpected face count loaded from teddy bear");
    
    TsfiRenderVertex *render_verts = NULL;
    uint32_t *render_indices = NULL;
    int r_vert_count = 0;
    int r_ind_count = 0;
    tsfi_obj_generate_buffers(bear_mesh, &render_verts, &render_indices, &r_vert_count, &r_ind_count);
    assert(r_vert_count == 13824 && "Expected 13824 vertices generated for 4608 faces");
    assert(r_ind_count == 13824 && "Expected 13824 indices generated for 4608 faces");
    
    free(render_verts);
    free(render_indices);
    tsfi_obj_free(bear_mesh);
    printf("[OBJ] Successfully imported and validated a regular bear OBJ model!\n");

    printf("[EVM] Retrieving symbolic parameters from local Dysnomia VM...\n");

    int W = 1280;
    int H = 720;
    int stride = W * sizeof(Ab4hPixel);
    size_t sz = H * stride;

    void *framebuffer = NULL;
    int mfd = -1;

    // Check Wayland availability or force headless mode via argument
    const char *run = getenv("XDG_RUNTIME_DIR");
    const char *disp = getenv("WAYLAND_DISPLAY");
    bool headless = false;
    bool benchmark = false;
    for (int idx = 1; idx < argc; idx++) {
        if (strcmp(argv[idx], "--headless") == 0 || strcmp(argv[idx], "--render-once") == 0) {
            headless = true;
        }
        if (strcmp(argv[idx], "--benchmark") == 0) {
            benchmark = true;
            headless = true;
        }
    }
    if (!headless && (!run || !disp)) {
        printf("[WARN] XDG_RUNTIME_DIR or WAYLAND_DISPLAY not set. Headless mode activated.\n");
        headless = true;
    }

    // Allocate offscreen rendering buffer matching OpenCV Mat layout (Ab4hPixel)
    void *offscreen_buf = calloc(1, sz);
    assert(offscreen_buf != NULL);
    TsfiAb4hMat canvas = { .rows = H, .cols = W, .stride = stride, .data = (Ab4hPixel *)offscreen_buf };

    if (headless) {
        if (benchmark) {
            printf("[BENCHMARK] Starting headless benchmark for 50 frames...\n");
            double *frame_times = calloc(50, sizeof(double));
            struct timespec b_start, b_end;
            clock_gettime(CLOCK_MONOTONIC, &b_start);

            twitch_intensity = 0.5f;
            params.identity_pole = 20;
            sickness_intensity = 0.0f;
            fur_length = 0.0f;

            for (int frame = 0; frame < 50; frame++) {
                struct timespec f_start, f_end;
                clock_gettime(CLOCK_MONOTONIC, &f_start);

                if (frame % 30 == 0) {
                    reload_genome();
                }

                render_frame(&canvas, frame);

                clock_gettime(CLOCK_MONOTONIC, &f_end);
                frame_times[frame] = (f_end.tv_sec - f_start.tv_sec) + (f_end.tv_nsec - f_start.tv_nsec) / 1e9;
            }

            clock_gettime(CLOCK_MONOTONIC, &b_end);
            double total_elapsed = (b_end.tv_sec - b_start.tv_sec) + (b_end.tv_nsec - b_start.tv_nsec) / 1e9;

            double sum_frame_times = 0.0;
            for (int i = 0; i < 50; i++) {
                sum_frame_times += frame_times[i];
            }
            double avg_frame_time = sum_frame_times / 50.0;
            double avg_fps = 50.0 / sum_frame_times;

            const char *json_path = "/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite/benchmark_results.json";
            FILE *jf = fopen(json_path, "w");
            if (!jf) {
                mkdir("/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks", 0777);
                mkdir("/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/benchmarks/profiler_suite", 0777);
                jf = fopen(json_path, "w");
            }

            if (jf) {
                fprintf(jf, "{\n");
                fprintf(jf, "  \"summary\": {\n");
                fprintf(jf, "    \"total_frames\": 50,\n");
                fprintf(jf, "    \"elapsed_time_seconds\": %.6f,\n", total_elapsed);
                fprintf(jf, "    \"total_render_time_seconds\": %.6f,\n", sum_frame_times);
                fprintf(jf, "    \"average_fps\": %.6f,\n", avg_fps);
                fprintf(jf, "    \"average_frame_render_time_seconds\": %.6f\n", avg_frame_time);
                fprintf(jf, "  },\n");
                fprintf(jf, "  \"frame_times_seconds\": [\n");
                for (int i = 0; i < 50; i++) {
                    fprintf(jf, "    %.6f%s\n", frame_times[i], (i == 49) ? "" : ",");
                }
                fprintf(jf, "  ],\n");
                fprintf(jf, "  \"audio_latency_records\": [\n");
                pthread_mutex_lock(&g_latency_mutex);
                for (int i = 0; i < g_latency_record_count; i++) {
                    const char *comp = "teddy_bear_editor";
                    const char *t = g_latency_records[i].type;
                    if (strcmp(t, "kick") == 0 || strcmp(t, "snare") == 0 || strcmp(t, "tom") == 0 ||
                        strcmp(t, "hats") == 0 || strcmp(t, "ride") == 0 || strcmp(t, "clap") == 0 ||
                        strcmp(t, "snap") == 0) {
                        comp = "drum_sequencer";
                    }
                    fprintf(jf, "    {\n");
                    fprintf(jf, "      \"component\": \"%s\",\n", comp);
                    fprintf(jf, "      \"sound_type\": \"%s\",\n", t);
                    fprintf(jf, "      \"software_queue_delay_seconds\": %.6f,\n", g_latency_records[i].sw_delay);
                    fprintf(jf, "      \"hardware_buffer_delay_seconds\": %.6f,\n", g_latency_records[i].hw_delay);
                    fprintf(jf, "      \"total_latency_seconds\": %.6f\n", g_latency_records[i].total_latency);
                    fprintf(jf, "    }%s\n", (i == g_latency_record_count - 1) ? "" : ",");
                }
                pthread_mutex_unlock(&g_latency_mutex);
                fprintf(jf, "  ]\n");
                fprintf(jf, "}\n");
                fclose(jf);
                printf("[BENCHMARK] Metrics successfully written to %s\n", json_path);
            } else {
                printf("[ERROR] Failed to write benchmark results to %s: %s\n", json_path, strerror(errno));
            }

            free(frame_times);
            free(offscreen_buf);
            stop_audio_mixer();
            return 0;
        } else {
            twitch_intensity = 0.5f;
            params.identity_pole = 20;
            sickness_intensity = 0.0f;
            fur_length = 0.0f;
            render_frame(&canvas, 0);
            validate_rendering_via_object_recognition(&canvas);
            export_ppm_real(&canvas);
            free(offscreen_buf);
            stop_audio_mixer();
            return 0;
        }
    }

    display = wl_display_connect(NULL);
    if (!display) {
        printf("[WARN] Failed to connect to Wayland display. Falling back to headless mode.\n");
        twitch_intensity = 0.5f;
        params.identity_pole = 20;
        sickness_intensity = 0.0f;
        fur_length = 0.0f;
        render_frame(&canvas, 0);
        validate_rendering_via_object_recognition(&canvas);
        export_ppm_real(&canvas);
        free(offscreen_buf);
        return 0;
    }

    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    if (!compositor || !shm || !xdg_wm_base) {
        printf("[WARN] Missing core Wayland protocols. Falling back to headless mode.\n");
        wl_display_disconnect(display);
        twitch_intensity = 0.5f;
        params.identity_pole = 20;
        sickness_intensity = 0.0f;
        fur_length = 0.0f;
        render_frame(&canvas, 0);
        validate_rendering_via_object_recognition(&canvas);
        export_ppm_real(&canvas);
        free(offscreen_buf);
        return 0;
    }

    xdg_wm_base_add_listener(xdg_wm_base, &xdg_wm_base_listener, NULL);
    if (seat) {
        wl_seat_add_listener(seat, &seat_listener, NULL);
    }

    // Wayland shared memory buffer is 32-bit ARGB (4 bytes per pixel)
    size_t argb_sz = W * H * sizeof(uint32_t);
    mfd = memfd_create("tsfi_teddy_shm", MFD_CLOEXEC);
    if (mfd < 0 || ftruncate(mfd, argb_sz) < 0) {
        printf("[ERROR] Failed to allocate shared memory fd.\n");
        free(offscreen_buf);
        return 1;
    }

    framebuffer = mmap(NULL, argb_sz, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, 0);
    assert(framebuffer != MAP_FAILED);

    surface = wl_compositor_create_surface(compositor);
    xdg_surface = xdg_wm_base_get_xdg_surface(xdg_wm_base, surface);
    xdg_surface_add_listener(xdg_surface, &xdg_surface_listener, NULL);
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(xdg_toplevel, &xdg_toplevel_listener, NULL);
    xdg_toplevel_set_title(xdg_toplevel, "TSFi Vulkan Teddy Bear Tool Palette");

    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    struct wl_shm_pool *pool = wl_shm_create_pool(shm, mfd, argb_sz);
    wl_buf = wl_shm_pool_create_buffer(pool, 0, W, H, W * sizeof(uint32_t), WL_SHM_FORMAT_ARGB8888);
    wl_shm_pool_destroy(pool);

    // Setup terminal escape mode to safely terminate window via escape
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

    printf("[RENDER] Interactive window opened. Press ESC in terminal to exit.\n");
    int frame = 0;
    while (window_running) {
        char esc_char;
        if (read(STDIN_FILENO, &esc_char, 1) == 1 && esc_char == 27) {
            printf("\n[TERMINATE] ESC key detected. Securely closing window.\n");
            window_running = false;
            break;
        }

        struct pollfd fds[1] = {
            { wl_display_get_fd(display), POLLIN, 0 }
        };
        wl_display_dispatch_pending(display);
        wl_display_flush(display);
        if (poll(fds, 1, 0) > 0) {
            wl_display_dispatch(display);
        }

        if (demo_mode) {
            int phase = frame % 300;
            if (phase >= 10 && phase < 40) {
                // Drag Slider 0 (Fur Length)
                int mx = 850 + (phase - 10) * 11;
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(mx), wl_fixed_from_int(150));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            } else if (phase == 40) {
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase >= 50 && phase < 80) {
                // Drag Slider 1 (Scale)
                int mx = 850 + (phase - 50) * 11;
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(mx), wl_fixed_from_int(250));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            } else if (phase == 80) {
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase >= 90 && phase < 120) {
                // Drag Slider 2 (Light Angle)
                int mx = 850 + (phase - 90) * 11;
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(mx), wl_fixed_from_int(350));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
            } else if (phase == 120) {
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 130) {
                // Click Randomize Button
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(450));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 160) {
                // Click Default Button
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(900), wl_fixed_from_int(500));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 190) {
                // Click VLM Critic Cycle
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(545));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 220) {
                // Click Generator Cycle
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(585));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            } else if (phase == 250) {
                // Click Genetic Optimizer trigger
                pointer_handle_motion(NULL, NULL, 0, wl_fixed_from_int(1000), wl_fixed_from_int(630));
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_PRESSED);
                pointer_handle_button(NULL, NULL, 0, 0, BTN_LEFT, WL_POINTER_BUTTON_STATE_RELEASED);
            }
        }

        if (frame % 30 == 0) {
            reload_genome();
        }

        // Render viewport layout to offscreen AB4H canvas
        render_frame(&canvas, frame);

        if (active_slider == -2) {
            validate_rendering_via_object_recognition(&canvas);
            export_ppm_real(&canvas);
            active_slider = -1;
        }

        // Downsample and copy offscreen AB4H canvas to 32-bit Wayland display buffer
        present_ab4h_to_argb(&canvas, (uint32_t *)framebuffer);

        wl_surface_attach(surface, wl_buf, 0, 0);
        wl_surface_damage(surface, 0, 0, W, H);
        wl_surface_commit(surface);
        wl_display_flush(display);

        frame++;
        usleep(33000); // ~30 fps
    }

    validate_rendering_via_object_recognition(&canvas);
    export_ppm_real(&canvas);

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    if (pointer) wl_pointer_destroy(pointer);
    if (keyboard) wl_keyboard_destroy(keyboard);
    if (seat) wl_seat_destroy(seat);
    xdg_toplevel_destroy(xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(surface);
    wl_buffer_destroy(wl_buf);
    wl_display_disconnect(display);

    munmap(framebuffer, argb_sz);
    close(mfd);
    free(offscreen_buf);
    stop_audio_mixer();
    return 0;
}

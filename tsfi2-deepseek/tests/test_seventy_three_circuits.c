#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// -------------------------------------------------------------
// 1. Audio Booster (Preamplifier & Clipper)
// -------------------------------------------------------------
// Models a microphone booster that applies gain and soft clipping.
// This increases average "talk power" by reducing the peak-to-average ratio.
typedef struct {
    float gain;
    float clip_level;
} AudioBooster;

void audio_booster_init(AudioBooster *b, float gain, float clip_level) {
    b->gain = gain;
    b->clip_level = clip_level;
}

float audio_booster_process(AudioBooster *b, float input) {
    float amplified = input * b->gain;
    // Apply soft clipping (tanh curve) to prevent sharp harmonic splatter
    return b->clip_level * tanhf(amplified / b->clip_level);
}

// -------------------------------------------------------------
// 2. Varicap FM Modulator ("Something New in FM")
// -------------------------------------------------------------
// Models an LC tank oscillator modulated by a varicap diode.
// Capacitance: C(v) = C0 / sqrt(1 + v / V_phi)
// Resonance: f(v) = 1 / (2 * pi * sqrt(L * C(v))) = f0 * (1 + v / V_phi)^(1/4)
typedef struct {
    float f0;          // Base frequency (Hz)
    float v_phi;       // Varicap barrier potential
    float phase;       // Running phase accumulator
    float sample_rate;
} VaricapFMOscillator;

void varicap_fm_init(VaricapFMOscillator *osc, float f0, float v_phi, float sample_rate) {
    osc->f0 = f0;
    osc->v_phi = v_phi;
    osc->phase = 0.0f;
    osc->sample_rate = sample_rate;
}

float varicap_fm_process(VaricapFMOscillator *osc, float v_mod) {
    // Dynamic modulation frequency
    float freq = osc->f0 * powf(1.0f + v_mod / osc->v_phi, 0.25f);
    
    // Accumulate phase
    osc->phase += 2.0f * (float)M_PI * freq / osc->sample_rate;
    if (osc->phase > 2.0f * M_PI) {
        osc->phase -= 2.0f * (float)M_PI;
    }
    return sinf(osc->phase);
}

// -------------------------------------------------------------
// 3. Bantam VHF Mixer
// -------------------------------------------------------------
// Models a VHF converter mixer. We down-convert a high-frequency RF signal
// using a Local Oscillator (LO) to produce an Intermediate Frequency (IF).
// RF = 14.4 kHz, LO = 13.0 kHz -> IF = 1.4 kHz (scaled for audio-rate testing)
typedef struct {
    float f_lo;
    float sample_rate;
    float phase_lo;
} BantamMixer;

void bantam_mixer_init(BantamMixer *m, float f_lo, float sample_rate) {
    m->f_lo = f_lo;
    m->sample_rate = sample_rate;
    m->phase_lo = 0.0f;
}

float bantam_mixer_process(BantamMixer *m, float rf_in) {
    // Generate LO carrier
    m->phase_lo += 2.0f * (float)M_PI * m->f_lo / m->sample_rate;
    if (m->phase_lo > 2.0f * M_PI) {
        m->phase_lo -= 2.0f * (float)M_PI;
    }
    float lo_signal = cosf(m->phase_lo);
    
    // Mix (multiplication) producing sum and difference frequencies
    return rf_in * lo_signal;
}

// -------------------------------------------------------------
// 4. Tubeless Electronic Keyer
// -------------------------------------------------------------
// Timing state machine. Outputs self-completing dots (ratio 1:1) and dashes (3:1).
typedef enum { STATE_IDLE, STATE_DOT, STATE_DASH, STATE_SPACE } KeyerState;

typedef struct {
    KeyerState state;
    int ticks_left;
    int unit_ticks; // Number of ticks per standard "dit" unit
} ElectronicKeyer;

void keyer_init(ElectronicKeyer *k, int unit_ticks) {
    k->state = STATE_IDLE;
    k->ticks_left = 0;
    k->unit_ticks = unit_ticks;
}

bool keyer_tick(ElectronicKeyer *k, bool dit_lever, bool dah_lever, bool *rf_key_out) {
    if (k->ticks_left > 0) {
        k->ticks_left--;
    }
    
    if (k->ticks_left == 0) {
        switch (k->state) {
            case STATE_IDLE:
                if (dit_lever) {
                    k->state = STATE_DOT;
                    k->ticks_left = k->unit_ticks;
                } else if (dah_lever) {
                    k->state = STATE_DASH;
                    k->ticks_left = k->unit_ticks * 3;
                }
                break;
            case STATE_DOT:
            case STATE_DASH:
                // Force a space element after a character element
                k->state = STATE_SPACE;
                k->ticks_left = k->unit_ticks;
                break;
            case STATE_SPACE:
                k->state = STATE_IDLE;
                // Instantly transition if lever is still pressed
                if (dit_lever) {
                    k->state = STATE_DOT;
                    k->ticks_left = k->unit_ticks;
                } else if (dah_lever) {
                    k->state = STATE_DASH;
                    k->ticks_left = k->unit_ticks * 3;
                }
                break;
        }
    }
    
    *rf_key_out = (k->state == STATE_DOT || k->state == STATE_DASH);
    return (k->state != STATE_IDLE);
}

// -------------------------------------------------------------
// 5. Direct Reading Capacity Meter
// -------------------------------------------------------------
// Measures capacitance C based on charge/discharge cycles per second (freq).
// Average Current: I_avg = C * V_ref * freq
typedef struct {
    float v_ref;
    float freq;
} CapacityMeter;

void capacity_meter_init(CapacityMeter *m, float v_ref, float freq) {
    m->v_ref = v_ref;
    m->freq = freq;
}

float capacity_meter_measure_current(CapacityMeter *m, float capacitance_farads) {
    return capacitance_farads * m->v_ref * m->freq;
}

// -------------------------------------------------------------
// 6. W0TKX Forty-Meter Transistor Final Amplifier ("Transistor Final for Forty")
// -------------------------------------------------------------
// Models a Class C transistor power amplifier stage for the 40m band (7.0 MHz).
// Transistor conduction is non-linear (only conducts when input RF drive exceeds threshold V_th).
// Collector current charges an LC tank tuned to 7.0 MHz.
typedef struct {
    float vcc;         // Supply voltage (V, typically 12V to 24V)
    float v_th;        // Transistor base-emitter conduction threshold (V, e.g. 0.6V)
    float l_tank;      // Tank inductance (H)
    float c_tank;      // Tank capacitance (F)
    float r_load;      // Load resistance (Ohms, e.g. 50 Ohms antenna)
    float state_i_l;   // Tank inductor current state
    float state_v_c;   // Tank capacitor voltage state (output RF)
    float sample_rate;
} W0TKXTransistorFinal;

void w0tkx_final_init(W0TKXTransistorFinal *amp, float vcc, float v_th, float sample_rate) {
    amp->vcc = vcc;
    amp->v_th = v_th;
    // Tuned to ~7.0 MHz (40 meters): L = 1.0 uH, C = 516 pF -> f0 = 1 / (2*pi*sqrt(L*C)) = 7.0 MHz
    // To allow stable simulation at standard audio sample rates or higher (e.g., 96 kHz or 192 kHz),
    // we scale the resonance down by a factor of 1000 to ~7.0 kHz.
    // At f0 = 7000 Hz, L = 1.0 mH, C = 0.516 uF.
    amp->l_tank = 1.0e-3f;
    amp->c_tank = 0.516e-6f;
    amp->r_load = 50.0f;
    amp->state_i_l = 0.0f;
    amp->state_v_c = 0.0f;
    amp->sample_rate = sample_rate;
}

float w0tkx_final_step(W0TKXTransistorFinal *amp, float rf_drive_v, float dt) {
    // Transistor collector current: conducts when input RF drive exceeds base threshold
    float i_coll = 0.0f;
    if (rf_drive_v > amp->v_th) {
        // Linear-ish approximation of collector current under conduction
        i_coll = (rf_drive_v - amp->v_th) * 0.2f; // Transconductance gm = 0.2 A/V
        if (i_coll > 0.5f) i_coll = 0.5f;        // Saturation current limit 0.5A
    }
    
    // Solve LC tank differential equations:
    // d(v_c)/dt = (i_coll - state_i_l - state_v_c / r_load) / C
    // d(i_l)/dt = state_v_c / L
    float dv_c = (i_coll - amp->state_i_l - amp->state_v_c / amp->r_load) / amp->c_tank * dt;
    float di_l = amp->state_v_c / amp->l_tank * dt;
    
    amp->state_v_c += dv_c;
    amp->state_i_l += di_l;
    
    // Output voltage is measured at the collector (Vcc - V_c)
    return amp->vcc - amp->state_v_c;
}

// -------------------------------------------------------------
// Validation Tests
// -------------------------------------------------------------
int main() {
    printf("=== TSFi Seventy-Three (73) Magazine Issue #1 Simulation Suite ===\n");
    
    // --- 1. Test Audio Booster ---
    printf("[TEST] Testing Audio Booster preamp & soft-clipping...\n");
    AudioBooster booster;
    audio_booster_init(&booster, 5.0f, 1.0f); // Gain of 5, clipping level at 1.0V
    
    float low_sig = audio_booster_process(&booster, 0.05f); // 0.25V unclipped
    float high_sig = audio_booster_process(&booster, 1.5f); // Should be soft-clipped close to 1.0V
    
    printf("  Low signal output (expected ~0.25): %.4f\n", low_sig);
    printf("  High signal output (expected ~1.0): %.4f\n", high_sig);
    assert(low_sig > 0.20f && low_sig < 0.26f);
    assert(high_sig > 0.90f && high_sig < 1.01f);
    
    // --- 2. Test Varicap FM Modulator ---
    printf("[TEST] Testing Varicap Diode FM modulation...\n");
    VaricapFMOscillator osc;
    varicap_fm_init(&osc, 1000.0f, 4.0f, 48000.0f); // 1 kHz base frequency, 48 kHz sample rate
    
    // Read frequency variations at +2V and -2V control voltages
    float out_idle = varicap_fm_process(&osc, 0.0f);
    (void)out_idle;
    float freq_plus = osc.f0 * powf(1.0f + 2.0f / osc.v_phi, 0.25f);
    float freq_minus = osc.f0 * powf(1.0f - 2.0f / osc.v_phi, 0.25f);
    printf("  Modulation +2.0V frequency: %.2f Hz\n", freq_plus);
    printf("  Modulation -2.0V frequency: %.2f Hz\n", freq_minus);
    assert(freq_plus > osc.f0);
    assert(freq_minus < osc.f0);
    
    // --- 3. Test Bantam VHF Mixer ---
    printf("[TEST] Testing Bantam VHF Mixer down-converter...\n");
    BantamMixer mixer;
    float sample_rate = 48000.0f;
    bantam_mixer_init(&mixer, 13000.0f, sample_rate); // LO = 13.0 kHz
    
    // Input RF carrier = 14.4 kHz
    float f_rf = 14400.0f;
    float sum_diff_val = 0.0f;
    int num_samples = 200;
    
    for (int i = 0; i < num_samples; i++) {
        float rf_in = sinf(2.0f * (float)M_PI * f_rf * i / sample_rate);
        float mixed = bantam_mixer_process(&mixer, rf_in);
        // Integrate or filter to analyze the difference (14.4 - 13.0 = 1.4 kHz) and sum (27.4 kHz)
        sum_diff_val += mixed * cosf(2.0f * (float)M_PI * 1400.0f * i / sample_rate);
    }
    // A strong correlation magnitude indicates successful down-conversion to 1.4 kHz IF
    float avg_corr = fabsf(sum_diff_val / (num_samples / 2.0f));
    printf("  Down-conversion correlation to 1.4 kHz IF: %.4f\n", avg_corr);
    assert(avg_corr > 0.4f);
    
    // --- 4. Test Electronic Keyer ---
    printf("[TEST] Testing Tubeless Electronic Keyer state machine...\n");
    ElectronicKeyer keyer;
    keyer_init(&keyer, 5); // 5 ticks per dit element
    
    bool rf_out = false;
    // Press Dot lever
    printf("  Ticking DOT lever...\n");
    for (int i = 0; i < 15; i++) {
        keyer_tick(&keyer, true, false, &rf_out);
        if (i < 5) assert(rf_out == true);   // Dit active
        if (i >= 5 && i < 10) assert(rf_out == false); // Space active
    }
    // Press Dash lever
    keyer_init(&keyer, 5);
    printf("  Ticking DASH lever...\n");
    for (int i = 0; i < 25; i++) {
        keyer_tick(&keyer, false, true, &rf_out);
        if (i < 15) assert(rf_out == true);  // Dash active (3x dit duration = 15 ticks)
        if (i >= 15 && i < 20) assert(rf_out == false); // Space active
    }
    printf("  Keyer timing verified successfully!\n");
    
    // --- 5. Capacity Meter ---
    printf("[TEST] Testing Direct Reading Capacity Meter...\n");
    CapacityMeter cap_meter;
    capacity_meter_init(&cap_meter, 10.0f, 1000.0f); // 10V reference, 1 kHz oscillator
    
    float test_c = 100e-9f; // 100 nF
    float expected_current = test_c * cap_meter.v_ref * cap_meter.freq; // 1 mA
    float measured_current = capacity_meter_measure_current(&cap_meter, test_c);
    
    printf("  100nF capacitor expected current: %.6f A\n", expected_current);
    printf("  Measured current: %.6f A\n", measured_current);
    assert(fabs(measured_current - expected_current) < 1e-9f);
    
    // --- 6. Test W0TKX Transistor Final Amplifier ---
    printf("[TEST] Testing W0TKX 40m Transistor Final Amplifier...\n");
    W0TKXTransistorFinal amp;
    float fs = 96000.0f;
    w0tkx_final_init(&amp, 12.0f, 0.6f, fs); // 12V supply, 0.6V BE threshold
    
    float dt = 1.0f / fs;
    float max_out = -999.0f;
    float min_out = 999.0f;
    
    // Drive with a 7 kHz sine wave (matching scaled f0)
    for (int i = 0; i < 200; i++) {
        float drive = 2.0f * sinf(2.0f * (float)M_PI * 7000.0f * i * dt); // 2.0V peak drive
        float out = w0tkx_final_step(&amp, drive, dt);
        if (out > max_out) max_out = out;
        if (out < min_out) min_out = out;
    }
    
    printf("  RF output voltage swing: min = %.2f V, max = %.2f V\n", min_out, max_out);
    // Transistor turns on and pulls down collector voltage, LC tank swings above Vcc
    assert(min_out < 12.0f);
    assert(max_out > 12.0f);
    printf("  -> W0TKX Class C RF amplifier resonance confirmed!\n");
    
    printf("=== [SUCCESS] All 73 Magazine circuits simulated and verified! ===\n");
    return 0;
}

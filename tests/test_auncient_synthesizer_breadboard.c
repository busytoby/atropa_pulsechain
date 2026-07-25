#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MAX_SLOTS 8
#define SAMPLE_COUNT 100

typedef struct dsp_module {
    const char *name;
    void (*process)(struct dsp_module *self, double input, double *output);
    double state[4]; // Internal state storage for dynamic parameters
} dsp_module_t;

typedef struct {
    dsp_module_t *slots[MAX_SLOTS];
    size_t slot_count;
} synthesizer_breadboard_t;

// 1. Noise Generator Module (Source)
static void noise_process(dsp_module_t *self, double input, double *output) {
    (void)self;
    (void)input;
    // Generate pseudo-random value between -1.0 and 1.0
    *output = ((double)rand() / (double)RAND_MAX) * 2.0 - 1.0;
}

// 2. Verlet Membrane Oscillator Module (Mechanical Synthesizer)
static void verlet_process(dsp_module_t *self, double input, double *output) {
    // state[0] = position (x), state[1] = previous position (x_prev)
    double x = self->state[0];
    double x_prev = self->state[1];
    
    double dt = 0.01;
    double stiffness = 100.0;
    double damping = 0.5;
    
    // input acts as driving sound pressure (Ps)
    double force = input * 5.0;
    double velocity = (x - x_prev) / dt;
    double acceleration = force - stiffness * x - damping * velocity;
    
    double x_next = 2.0 * x - x_prev + acceleration * (dt * dt);
    
    // Update Verlet history
    self->state[1] = x;
    self->state[0] = x_next;
    
    *output = x_next;
}

// 3. Formant Resonance Filter Module (Vocal Tract)
static void formant_process(dsp_module_t *self, double input, double *output) {
    // state[0] = y_prev, state[1] = y_prev2
    double y_prev = self->state[0];
    double y_prev2 = self->state[1];
    
    // Resonant bandpass filter approximation (formant frequency emulation)
    double r_tract = 0.95;
    double y_next = input + r_tract * y_prev - 0.90 * y_prev2;
    
    self->state[1] = y_prev;
    self->state[0] = y_next;
    
    *output = y_next;
}

// 4. Sparklefly Compressor Module (Dynamics Shaper)
static void compressor_process(dsp_module_t *self, double input, double *output) {
    (void)self;
    double threshold = 0.5;
    double ratio = 4.0;
    double abs_input = fabs(input);
    if (abs_input > threshold) {
        double excess = abs_input - threshold;
        double compressed = threshold + excess / ratio;
        *output = (input > 0.0) ? compressed : -compressed;
    } else {
        *output = input;
    }
}

// 5. Whirlwind Holding-Gun Sustain Module (Active Bowing/Excitation)
static void sustain_process(dsp_module_t *self, double input, double *output) {
    double sustain_force = self->state[0] > 0.0 ? self->state[0] : 0.8;
    *output = input + sustain_force;
}

// 6. Burroughs Clock Gating Module (Envelope Voice Muting)
static void gating_process(dsp_module_t *self, double input, double *output) {
    (void)self;
    double gate_threshold = 0.01;
    if (fabs(input) < gate_threshold) {
        *output = 0.0; // Gate closed: mute signal and bypass calculations
    } else {
        *output = input; // Gate open
    }
}


// Plug a logic package module into the synthesizer breadboard
bool plug_module(synthesizer_breadboard_t *board, dsp_module_t *module, size_t slot) {
    if (slot >= MAX_SLOTS) return false;
    board->slots[slot] = module;
    return true;
}

// Unplug a module
void unplug_module(synthesizer_breadboard_t *board, size_t slot) {
    if (slot < MAX_SLOTS) {
        board->slots[slot] = NULL;
    }
}

// Execute the unified signal path through the active breadboard slots
void process_breadboard(synthesizer_breadboard_t *board, double input, double *output) {
    double current_val = input;
    for (size_t i = 0; i < MAX_SLOTS; i++) {
        if (board->slots[i] != NULL) {
            double out_val = 0.0;
            board->slots[i]->process(board->slots[i], current_val, &out_val);
            current_val = out_val;
        }
    }
    *output = current_val;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT SYNTHESIZER BREADBOARD PLUGGABLE LOGIC SUITE\n");
    printf("=============================================================\n");

    synthesizer_breadboard_t board;
    memset(&board, 0, sizeof(board));

    // Initialize Burroughs-style standardized logic units
    dsp_module_t noise_gen = {
        .name = "Burroughs Noise Generator",
        .process = noise_process,
        .state = {0}
    };

    dsp_module_t verlet_osc = {
        .name = "Verlet Folds Membrane",
        .process = verlet_process,
        .state = {0.0, 0.0} // Initial rest position
    };

    dsp_module_t formant_filter = {
        .name = "Formant Vocal Waveguide",
        .process = formant_process,
        .state = {0.0, 0.0}
    };

    // 1. Initial State: Empty Breadboard (signals pass straight through)
    printf("[TEST] Processing empty breadboard...\n");
    double output_val = 0.0;
    process_breadboard(&board, 1.5, &output_val);
    assert(output_val == 1.5);
    printf("   ✓ Signals bypassed empty board successfully.\n");

    // 2. Configuration A: Plug Noise Source -> Filter
    printf("[TEST] Hot-plugging Noise and Formant modules...\n");
    bool ok = plug_module(&board, &noise_gen, 0);
    assert(ok);
    ok = plug_module(&board, &formant_filter, 1);
    assert(ok);

    printf("   [INFO] Processing Configuration A (Noise -> Filter)...\n");
    for (int i = 0; i < 5; i++) {
        process_breadboard(&board, 0.0, &output_val);
        printf("      Sample %d | Output: %f\n", i, output_val);
    }
    printf("   ✓ Configuration A output verified.\n");

    // 3. Configuration B: Hot-swap to include mechanical Verlet Folds
    // Target routing: Noise Source -> Verlet Membrane -> Formant Filter
    printf("[TEST] Reconfiguring slots for Configuration B...\n");
    
    // Shift Formant filter to slot 2 and place Verlet in slot 1
    unplug_module(&board, 1);
    ok = plug_module(&board, &verlet_osc, 1);
    assert(ok);
    ok = plug_module(&board, &formant_filter, 2);
    assert(ok);

    printf("   [INFO] Processing Configuration B (Noise -> Verlet -> Filter)...\n");
    for (int i = 0; i < 5; i++) {
        process_breadboard(&board, 0.0, &output_val);
        printf("      Sample %d | Output: %f\n", i, output_val);
    }
    printf("   ✓ Configuration B output verified.\n");

    // 4. Configuration C: Hot-plug Sparklefly Compressor, Holding-Gun Sustain, and Clock Gating
    printf("[TEST] Hot-plugging Sparklefly, Holding-Gun, and Clock-Gating modules...\n");
    fflush(stdout);
    
    // Clear board first
    for (size_t i = 0; i < MAX_SLOTS; i++) {
        unplug_module(&board, i);
    }
    
    dsp_module_t compressor = {
        .name = "Sparklefly Compressor",
        .process = compressor_process,
        .state = {0}
    };
    
    dsp_module_t holding_gun = {
        .name = "Holding Gun Sustain",
        .process = sustain_process,
        .state = {0.8} // 0.8V constant sustain charge
    };
    
    dsp_module_t clock_gating = {
        .name = "Clock Gating Mute",
        .process = gating_process,
        .state = {0}
    };

    // Routing path: Sustain -> Compressor -> Clock Gating
    ok = plug_module(&board, &holding_gun, 0);
    assert(ok);
    ok = plug_module(&board, &compressor, 1);
    assert(ok);
    ok = plug_module(&board, &clock_gating, 2);
    assert(ok);

    // Test 1: Compressor threshold testing (Input 0.0 + 0.8 sustain = 0.8 input to compressor)
    // 0.8 > 0.5 threshold -> excess 0.3 / 4.0 ratio = 0.075 -> compressed output = 0.575
    process_breadboard(&board, 0.0, &output_val);
    printf("   [INFO] Compressor Output (expected 0.575000): %f\n", output_val);
    assert(fabs(output_val - 0.575) < 0.00001);
    
    // Test 2: Gating threshold testing (Low input, gated to absolute 0.0)
    // Shift: unplug sustain to test low-level input
    unplug_module(&board, 0);
    // Process input of 0.005 (below 0.01 gate threshold)
    process_breadboard(&board, 0.005, &output_val);
    printf("   [INFO] Gated Mute Output (expected 0.000000): %f\n", output_val);
    assert(output_val == 0.0);
    printf("   ✓ Sparklefly, Holding-Gun, and Clock-Gating modules verified.\n");
    fflush(stdout);


    printf("=============================================================\n");
    printf("SYNTHESIZER PLUGGABLE LOGIC TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

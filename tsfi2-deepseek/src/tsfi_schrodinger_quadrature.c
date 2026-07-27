#include "tsfi_schrodinger_quadrature.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static uint64_t schrodinger_mod_exp(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base = base % mod;
    while (exp > 0) {
        if (exp % 2 == 1) {
            __uint128_t temp = (__uint128_t)res * base;
            res = (uint64_t)(temp % mod);
        }
        __uint128_t temp = (__uint128_t)base * base;
        base = (uint64_t)(temp % mod);
        exp /= 2;
    }
    return res;
}

void tsfi_schrodinger_quad_init(tsfi_SchrodingerQuadCircuit* circuit, uint64_t seed_base, uint64_t seed_signal) {
    if (!circuit) return;
    circuit->Base = seed_base % MotzkinPrime;
    if (circuit->Base == 0) {
        circuit->Base = 440;
    }
    circuit->Signal = seed_signal;
    circuit->Channel = schrodinger_mod_exp(circuit->Base, circuit->Signal, MotzkinPrime);
    circuit->Element = 1000;
    circuit->InPhase = 0;
    circuit->QuadPhase = 0;
    circuit->WaveFunction = 0;
    circuit->Time = 0.0;
    circuit->MangleEnabled = true;
}

void tsfi_schrodinger_quad_step(tsfi_SchrodingerQuadCircuit* circuit, double dt, uint64_t input_note) {
    if (!circuit) return;
    circuit->Time += dt;
    
    // Rhythmic velocity/modulus scaling
    uint64_t note_factor = input_note > 0 ? input_note : 1;
    circuit->Element = (circuit->Base % 97) + note_factor;

    // Derived signal frequency (typically 10 - 20 Hz rotor frequency)
    double f_rotor = 16.0 * ((double)(circuit->Channel % 100) / 100.0 + 0.5);
    double angle = 2.0 * M_PI * f_rotor * circuit->Time;

    // Split-phase Quadrature resolution (I and Q orthogonals)
    double scale = (double)(circuit->Base % 1000);
    double i_val = scale * cos(angle);
    double q_val = scale * sin(angle);

    // Write back to hardware registers (mapping real/imaginary values)
    circuit->InPhase = (uint64_t)fabs(i_val) % MotzkinPrime;
    circuit->QuadPhase = (uint64_t)fabs(q_val) % MotzkinPrime;
    
    // Superposition wave function calculation: Psi = I * cos(theta) - Q * sin(theta)
    double psi_val = i_val * cos(angle * 0.5) - q_val * sin(angle * 0.5);
    circuit->WaveFunction = (uint64_t)fabs(psi_val) % MotzkinPrime;
}

void tsfi_schrodinger_quad_resolve(const tsfi_SchrodingerQuadCircuit* circuit, double* out_freq_l, double* out_freq_r) {
    if (!circuit || !out_freq_l || !out_freq_r) return;
    
    // Left channel maps to InPhase vector resolving
    double f_l = (double)(circuit->InPhase % 600) + 150.0;
    // Right channel maps to QuadPhase vector resolving
    double f_r = (double)(circuit->QuadPhase % 600) + 150.0;
    
    if (circuit->MangleEnabled) {
        // Apply tonewheel mangling boundary
        f_l += (double)(circuit->WaveFunction % 50);
        f_r -= (double)(circuit->WaveFunction % 50);
    }
    
    *out_freq_l = f_l;
    *out_freq_r = f_r;
}

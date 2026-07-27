#ifndef TSFI_SCHRODINGER_QUADRATURE_H
#define TSFI_SCHRODINGER_QUADRATURE_H

#include <stdint.h>
#include <stdbool.h>

#define MotzkinPrime 953467954114363ULL

/*
 * Schrodinger's Quadrature Circuit State.
 * Implements a thread-safe, low-level Yul virtual hardware split-phase quadrature resolver.
 */
typedef struct {
    uint64_t Base;          /* Root reference alignment register */
    uint64_t Signal;        /* Wave carrier register */
    uint64_t Channel;       /* Transmission channel configuration */
    uint64_t Element;       /* Structural layout layout modulus */
    uint64_t InPhase;       /* I (real-axis cosine resolver) */
    uint64_t QuadPhase;     /* Q (imaginary-axis sine resolver) */
    uint64_t WaveFunction;  /* Combined complex state tracking superposition */
    double Time;            /* Time parameter for time-dependency */
    bool MangleEnabled;     /* Tonewheel pre-mangler toggle state */
} tsfi_SchrodingerQuadCircuit;

/* Initialize the Quadrature Circuit */
void tsfi_schrodinger_quad_init(tsfi_SchrodingerQuadCircuit* circuit, uint64_t seed_base, uint64_t seed_signal);

/* Step the quadrature circuit emulated clock cycle */
void tsfi_schrodinger_quad_step(tsfi_SchrodingerQuadCircuit* circuit, double dt, uint64_t input_note);

/* Retrieve the panned binaural output frequencies (Left and Right formants) */
void tsfi_schrodinger_quad_resolve(const tsfi_SchrodingerQuadCircuit* circuit, double* out_freq_l, double* out_freq_r);

#endif /* TSFI_SCHRODINGER_QUADRATURE_H */

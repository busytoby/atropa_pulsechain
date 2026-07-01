#ifndef TSFI_DYSNOMIA_CIRCUIT_SIMULATION_H
#define TSFI_DYSNOMIA_CIRCUIT_SIMULATION_H

#include <stdint.h>
#include <stdbool.h>

// WinchesterMQ Virtual Hardware Register Latch / Lock Accumulators
typedef struct {
    // Primary locked inputs (FUSE parameters)
    uint64_t prime;
    uint64_t base;
    uint64_t secret_rod;
    uint64_t secret_cone;
    uint64_t signal;
    uint64_t alpha;
    uint64_t beta;

    // Secondary Lock Accumulators
    uint64_t acc_npn_latched;
    uint64_t acc_pnp_latched;
    bool is_locked;

    // Output Registers
    uint64_t manifold;
    uint64_t monopole;
    uint64_t ring;
    uint64_t barn;
} DysnomiaLockCircuit;

// Core functions
void tsfi_circuit_init(DysnomiaLockCircuit *circuit);
void tsfi_circuit_lock(DysnomiaLockCircuit *circuit, uint64_t npn_input, uint64_t pnp_input);
void tsfi_circuit_unlock(DysnomiaLockCircuit *circuit);
bool tsfi_circuit_execute_cascade(DysnomiaLockCircuit *circuit);

#endif // TSFI_DYSNOMIA_CIRCUIT_SIMULATION_H

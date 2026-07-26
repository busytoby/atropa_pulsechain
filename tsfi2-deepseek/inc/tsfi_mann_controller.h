#ifndef TSFI_MANN_CONTROLLER_H
#define TSFI_MANN_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>
#include "../../tests/auncient_cactus_schema.h"

#define MANN_MEMORY_SLOTS 16
#define MANN_SLOT_WIDTH 32

// MANN Memory Controller representing a Memory Augmented Neural Network controller on virtual Yul registers
typedef struct {
    float memory_matrix[MANN_MEMORY_SLOTS][MANN_SLOT_WIDTH];
    float usage_vector[MANN_MEMORY_SLOTS];
    float temporal_link[MANN_MEMORY_SLOTS][MANN_MEMORY_SLOTS];
    
    // Gating parameters
    float write_gate;
    float read_gate;
    float allocation_gate;
} TSFiMANNController;

// Initialize the MANN memory controller to default states
void tsfi_mann_controller_init(TSFiMANNController *mann);

// Differentiable write head operation updating memory matrix based on key, value, and gates
void tsfi_mann_controller_write(TSFiMANNController *mann, const float *key_vector, float strength, const float *erase_vector, const float *write_vector);

// Differentiable read head operation reading from memory matrix using cosine similarity matching key vector
void tsfi_mann_controller_read(TSFiMANNController *mann, const float *key_vector, float strength, float *out_read_vector);

// Bridge function loading gate variables from custom USD MANN API schema
void tsfi_mann_controller_load_usd_gates(TSFiMANNController *mann, const usd_auncient_mann_api_t *mann_api);

#endif // TSFI_MANN_CONTROLLER_H

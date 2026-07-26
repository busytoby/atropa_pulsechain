#ifndef TSFI_COMP_PASS_H
#define TSFI_COMP_PASS_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_COMP_LAYERS 8

typedef struct {
    float color[3]; // RGB
    float alpha;
} CompLayer;

// WinchesterMQ Composition Pass (CompPass) context
typedef struct {
    CompLayer layers[MAX_COMP_LAYERS];
    int layer_count;
} TSFiCompPass;

// Initialize the composition pass context
void tsfi_comp_pass_init(TSFiCompPass *pass);

// Add a volumetric or background rendering layer to the composition stack
void tsfi_comp_pass_add_layer(TSFiCompPass *pass, float r, float g, float b, float alpha);

// Resolves composited pixel colors using WinchesterMQ multi-layer composition formula
void tsfi_comp_pass_resolve(const TSFiCompPass *pass, float *out_color);

#endif // TSFI_COMP_PASS_H

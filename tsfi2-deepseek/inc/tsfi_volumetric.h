#ifndef TSFI_VOLUMETRIC_H
#define TSFI_VOLUMETRIC_H

#include "tsfi_shadowmap.h"

typedef struct {
    double density;
    double extinction_coeff;
} TSFiVolumetric;

// Initializes the volumetric atmosphere properties
void tsfi_volumetric_init(TSFiVolumetric *vol, double density, double extinction_coeff);

// Performs raymarching through shadow map to calculate volumetric light intensity
double tsfi_volumetric_raymarch(const TSFiVolumetric *vol, const TSFiShadowMap *sm, double start_depth, double end_depth, int steps);

#endif // TSFI_VOLUMETRIC_H

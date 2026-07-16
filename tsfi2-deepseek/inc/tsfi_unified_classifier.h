#ifndef TSFI_UNIFIED_CLASSIFIER_H
#define TSFI_UNIFIED_CLASSIFIER_H

#include "tsfi_doxastic_classifier.h"
#include "tsfi_bayesian.h"

// Comprehensive classification of a 3D voxel coordinate region
typedef struct {
    TSFiDoxasticClass density_class; // SEED, BOND, or VOID
    TSFiStrainClass strain_class;     // STABLE, FRONTIER, or FRACTURE
    float confidence;                 // Voxel density ratio
    float strain_gradient;            // Rate of coordinate value transitions
    float posterior_probability;      // Bayesian posterior check (P(H|E))
} TSFiUnifiedVoxelState;

// Execute the unified classification pipeline over a voxel coordinate range
TSFiUnifiedVoxelState tsfi_unified_classify_region(tsfi_dat *dat, int x_start, int x_end, int y, int z,
                                                   const char *hypothesis_path, const char *evidence_path);

#endif // TSFI_UNIFIED_CLASSIFIER_H

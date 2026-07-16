#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_unified_classifier.h"

TSFiUnifiedVoxelState tsfi_unified_classify_region(tsfi_dat *dat, int x_start, int x_end, int y, int z,
                                                   const char *hypothesis_path, const char *evidence_path) {
    TSFiUnifiedVoxelState state = {
        .density_class = CLASS_UNKNOWN,
        .strain_class = CLASS_STABLE,
        .confidence = 0.0f,
        .strain_gradient = 0.0f,
        .posterior_probability = 0.0f
    };

    if (!dat) return state;

    // 1. Run Voxel Density Classifier
    TSFiDoxasticClassification dens = tsfi_doxastic_classify_region(dat, x_start, x_end, y, z);
    state.density_class = dens.class_id;
    state.confidence = dens.confidence;

    // 2. Run Voxel Strain Gradient Classifier
    TSFiStrainClassification str = tsfi_doxastic_classify_strain(dat, x_start, x_end, y, z);
    state.strain_class = str.class_id;
    state.strain_gradient = str.boundary_gradient;

    // 3. Run Bayesian Posterior Evaluation
    if (hypothesis_path && evidence_path) {
        state.posterior_probability = tsfi_bayesian_infer(dat, hypothesis_path, evidence_path);
    }

    return state;
}

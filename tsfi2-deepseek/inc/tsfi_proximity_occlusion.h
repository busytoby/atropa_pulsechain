#ifndef TSFI_PROXIMITY_OCCLUSION_H
#define TSFI_PROXIMITY_OCCLUSION_H

typedef struct {
    double max_distance;
    double intensity;
} TSFiProximityOcclusion;

// Initializes the proximity occlusion parameters
void tsfi_proximity_occlusion_init(TSFiProximityOcclusion *po, double max_distance, double intensity);

// Evaluates the ambient proximity occlusion factor based on geometric distance
double tsfi_proximity_occlusion_eval(const TSFiProximityOcclusion *po, double distance);

#endif // TSFI_PROXIMITY_OCCLUSION_H

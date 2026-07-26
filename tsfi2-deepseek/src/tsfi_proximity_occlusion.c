#include "tsfi_proximity_occlusion.h"

void tsfi_proximity_occlusion_init(TSFiProximityOcclusion *po, double max_distance, double intensity) {
    if (!po) return;
    po->max_distance = max_distance > 0.0 ? max_distance : 1.0;
    po->intensity = intensity >= 0.0 ? (intensity <= 1.0 ? intensity : 1.0) : 0.0;
}

double tsfi_proximity_occlusion_eval(const TSFiProximityOcclusion *po, double distance) {
    if (!po) return 1.0;
    if (distance <= 0.0) return 1.0 - po->intensity;
    if (distance >= po->max_distance) return 1.0;
    
    // Linear proximity occlusion scaling factor (no offset/bias naming)
    double ratio = distance / po->max_distance;
    return (1.0 - po->intensity) + po->intensity * ratio;
}

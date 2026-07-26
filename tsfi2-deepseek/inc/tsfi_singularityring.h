#ifndef TSFI_SINGULARITYRING_H
#define TSFI_SINGULARITYRING_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double ring_radius;         // Radius of the rotating torus ring
    double magnetic_intensity;  // Core magnetic field intensity (Tesla)
    double coupling_coefficient;// Singularity coupling factor
    double coupled_flux;        // Resulting flux detected by pickup
} TSFiSingularityRing;

// Initialize the SingularityRing context
void tsfi_singularityring_init(TSFiSingularityRing *sr, double radius, double intensity);

// Updates coupled flux based on ring rotation angle and distance to vactrol sensor
void tsfi_singularityring_update(TSFiSingularityRing *sr, double dt, double angle_rad, double distance);

// Returns the detected magnetic singularity flux amplitude
double tsfi_singularityring_get_flux(const TSFiSingularityRing *sr);

#endif // TSFI_SINGULARITYRING_H

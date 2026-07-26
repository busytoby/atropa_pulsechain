#include "tsfi_singularityring.h"
#include <math.h>

void tsfi_singularityring_init(TSFiSingularityRing *sr, double radius, double intensity) {
    if (!sr) return;
    
    sr->ring_radius = radius;
    sr->magnetic_intensity = intensity;
    sr->coupling_coefficient = 0.85;
    sr->coupled_flux = 0.0;
}

void tsfi_singularityring_update(TSFiSingularityRing *sr, double dt, double angle_rad, double distance) {
    if (!sr) return;
    (void)dt;
    
    // Magnetic flux is modulated by the rotation angle (sin) and decays with distance squared
    double spatial_attenuation = 1.0 / (distance * distance + 1.0);
    double rotational_modulation = 0.5 * (1.0 + cos(angle_rad));
    
    sr->coupled_flux = sr->magnetic_intensity * sr->ring_radius * sr->coupling_coefficient * rotational_modulation * spatial_attenuation;
}

double tsfi_singularityring_get_flux(const TSFiSingularityRing *sr) {
    if (!sr) return 0.0;
    
    return sr->coupled_flux;
}

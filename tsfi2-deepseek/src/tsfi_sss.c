#include "tsfi_sss.h"
#include <math.h>

void tsfi_sss_init(TSFiSubsurfaceScattering *sss, double scatter_radius, double absorption_coeff) {
    if (!sss) return;
    sss->scatter_radius = scatter_radius > 0.0 ? scatter_radius : 1.0;
    sss->absorption_coeff = absorption_coeff >= 0.0 ? absorption_coeff : 0.1;
}

double tsfi_sss_eval(const TSFiSubsurfaceScattering *sss, double thickness, double incoming_light) {
    if (!sss || thickness < 0.0) return 0.0;
    
    // Dipole-decay approximation mapping transmittance attenuation
    double decay = exp(-thickness * sss->absorption_coeff);
    double diffusion = 1.0 / (1.0 + (thickness * thickness) / (sss->scatter_radius * sss->scatter_radius));
    
    return incoming_light * decay * diffusion;
}

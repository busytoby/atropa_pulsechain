#ifndef TSFI_SSS_H
#define TSFI_SSS_H

typedef struct {
    double scatter_radius;
    double absorption_coeff;
} TSFiSubsurfaceScattering;

// Initializes sub-surface scattering parameters
void tsfi_sss_init(TSFiSubsurfaceScattering *sss, double scatter_radius, double absorption_coeff);

// Evaluates the light transmissive scattering intensity through translucent media
double tsfi_sss_eval(const TSFiSubsurfaceScattering *sss, double thickness, double incoming_light);

#endif // TSFI_SSS_H

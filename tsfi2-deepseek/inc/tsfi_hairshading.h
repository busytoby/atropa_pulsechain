#ifndef TSFI_HAIRSHADING_H
#define TSFI_HAIRSHADING_H

typedef struct {
    double specular_exponent;
    double reflection_coeff;
    double secondary_shift;     // Shift offset for concentric secondary reflection peak
    double jitter_amplitude;    // Vibration scale mapping mechanical jitter
} TSFiHairShading;

// Initializes hair/fiber shading parameters with animatronic configurations
void tsfi_hairshading_init(TSFiHairShading *hs, double specular_exponent, double reflection_coeff);

// Configures animatronic parameters for synthetic fiber reflections
void tsfi_hairshading_set_animatronic(TSFiHairShading *hs, double secondary_shift, double jitter_amplitude);

// Evaluates Kajiya-Kay anisotropic fiber reflection intensity
double tsfi_hairshading_eval(const TSFiHairShading *hs, const double *tangent, const double *light_dir, const double *view_dir);

#endif // TSFI_HAIRSHADING_H

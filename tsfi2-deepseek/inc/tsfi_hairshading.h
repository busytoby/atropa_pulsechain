#ifndef TSFI_HAIRSHADING_H
#define TSFI_HAIRSHADING_H

typedef struct {
    double specular_exponent;
    double reflection_coeff;
} TSFiHairShading;

// Initializes hair/fiber shading parameters
void tsfi_hairshading_init(TSFiHairShading *hs, double specular_exponent, double reflection_coeff);

// Evaluates Kajiya-Kay anisotropic fiber reflection intensity
double tsfi_hairshading_eval(const TSFiHairShading *hs, const double *tangent, const double *light_dir, const double *view_dir);

#endif // TSFI_HAIRSHADING_H

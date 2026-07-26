#include "tsfi_hairshading.h"
#include <math.h>

void tsfi_hairshading_init(TSFiHairShading *hs, double specular_exponent, double reflection_coeff) {
    if (!hs) return;
    hs->specular_exponent = specular_exponent > 0.0 ? specular_exponent : 10.0;
    hs->reflection_coeff = reflection_coeff >= 0.0 ? reflection_coeff : 0.5;
}

static double dot_vectors(const double *a, const double *b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

double tsfi_hairshading_eval(const TSFiHairShading *hs, const double *tangent, const double *light_dir, const double *view_dir) {
    if (!hs || !tangent || !light_dir || !view_dir) return 0.0;
    
    // Kajiya-Kay anisotropic specularity calculation
    double dot_tl = dot_vectors(tangent, light_dir);
    double dot_tv = dot_vectors(tangent, view_dir);
    
    double sin_tl = sqrt(1.0 - dot_tl * dot_tl);
    double sin_tv = sqrt(1.0 - dot_tv * dot_tv);
    
    double spec = sin_tl * sin_tv - dot_tl * dot_tv;
    if (spec < 0.0) spec = 0.0;
    if (spec > 1.0) spec = 1.0;
    
    return hs->reflection_coeff * pow(spec, hs->specular_exponent);
}

#include "tsfi_hairshading.h"
#include <math.h>

void tsfi_hairshading_init(TSFiHairShading *hs, double specular_exponent, double reflection_coeff) {
    if (!hs) return;
    hs->specular_exponent = specular_exponent > 0.0 ? specular_exponent : 10.0;
    hs->reflection_coeff = reflection_coeff >= 0.0 ? reflection_coeff : 0.5;
    hs->secondary_shift = 0.0;
    hs->jitter_amplitude = 0.0;
}

void tsfi_hairshading_set_animatronic(TSFiHairShading *hs, double secondary_shift, double jitter_amplitude) {
    if (!hs) return;
    hs->secondary_shift = secondary_shift;
    hs->jitter_amplitude = jitter_amplitude;
}

void tsfi_hairshading_verlet_step(TSFiHairShading *hs, double *x, double *x_prev, double force, double dt) {
    if (!hs || !x || !x_prev || dt <= 0.0) return;
    
    // Dead-zone gate to avoid excessive dynamic node calculations under low forces
    if (fabs(force) < 1e-4 && fabs(*x - *x_prev) < 1e-4) {
        return;
    }
    
    // Verlet integration step: x_next = 2 * x - x_prev + acc * dt^2
    double stiffness = 10.0;
    double damping = 0.5;
    double velocity = (*x - *x_prev) / dt;
    double acceleration = force - stiffness * (*x) - damping * velocity;
    
    double x_next = 2.0 * (*x) - (*x_prev) + acceleration * (dt * dt);
    *x_prev = *x;
    *x = x_next;
    
    // Bind resulting displacement value directly to the jitter parameter
    hs->jitter_amplitude = fabs(x_next);
}

static double dot_vectors(const double *a, const double *b) {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

double tsfi_hairshading_eval(const TSFiHairShading *hs, const double *tangent, const double *light_dir, const double *view_dir) {
    if (!hs || !tangent || !light_dir || !view_dir) return 0.0;
    
    // Simulate mechanical animatronic frame vibration jitter applying displacement to tangent
    double mod_tangent[3];
    mod_tangent[0] = tangent[0] + hs->jitter_amplitude * 0.1;
    mod_tangent[1] = tangent[1];
    mod_tangent[2] = tangent[2];
    
    double length = sqrt(mod_tangent[0] * mod_tangent[0] + mod_tangent[1] * mod_tangent[1] + mod_tangent[2] * mod_tangent[2]);
    if (length > 1e-6) {
        mod_tangent[0] /= length;
        mod_tangent[1] /= length;
        mod_tangent[2] /= length;
    }
    
    // Primary Kajiya-Kay peak
    double dot_tl = dot_vectors(mod_tangent, light_dir);
    double dot_tv = dot_vectors(mod_tangent, view_dir);
    
    double sin_tl = sqrt(1.0 - dot_tl * dot_tl);
    double sin_tv = sqrt(1.0 - dot_tv * dot_tv);
    
    double spec1 = sin_tl * sin_tv - dot_tl * dot_tv;
    if (spec1 < 0.0) spec1 = 0.0;
    if (spec1 > 1.0) spec1 = 1.0;
    
    // Secondary shifted peak representing synthetic/acrylic animatronic concentric reflection
    double shifted_dot_tl = dot_tl + hs->secondary_shift;
    if (shifted_dot_tl > 1.0) shifted_dot_tl = 1.0;
    if (shifted_dot_tl < -1.0) shifted_dot_tl = -1.0;
    
    double shifted_sin_tl = sqrt(1.0 - shifted_dot_tl * shifted_dot_tl);
    double spec2 = shifted_sin_tl * sin_tv - shifted_dot_tl * dot_tv;
    if (spec2 < 0.0) spec2 = 0.0;
    if (spec2 > 1.0) spec2 = 1.0;
    
    double final_spec = 0.7 * pow(spec1, hs->specular_exponent) + 0.3 * pow(spec2, hs->specular_exponent * 0.5);
    return hs->reflection_coeff * final_spec;
}

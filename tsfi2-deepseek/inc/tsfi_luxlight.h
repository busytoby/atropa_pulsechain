#ifndef TSFI_LUXLIGHT_H
#define TSFI_LUXLIGHT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char light_path[64];
    float intensity;
    float exposure;
    float color[3];
    float position[3];
    float direction[3];
} TSFiLuxLight;

// Initialize the LuxLight light source context
void tsfi_luxlight_init(TSFiLuxLight *light, const char *path);

// Sets the light intensity and exposure variables
void tsfi_luxlight_set_energy(TSFiLuxLight *light, float intensity, float exposure);

// Calculates the total radiated light energy on a target surface vertex coordinate
float tsfi_luxlight_calculate_attenuation(const TSFiLuxLight *light, const float vertex[3]);

#endif // TSFI_LUXLIGHT_H

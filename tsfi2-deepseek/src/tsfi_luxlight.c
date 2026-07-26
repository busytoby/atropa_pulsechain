#include "tsfi_luxlight.h"
#include <string.h>
#include <math.h>

void tsfi_luxlight_init(TSFiLuxLight *light, const char *path) {
    if (!light) return;
    
    if (path) {
        strncpy(light->light_path, path, sizeof(light->light_path) - 1);
    } else {
        strcpy(light->light_path, "/auncient/light/main");
    }
    
    light->intensity = 1.0f;
    light->exposure = 0.0f;
    light->color[0] = 1.0f;
    light->color[1] = 1.0f;
    light->color[2] = 1.0f;
    
    memset(light->position, 0, sizeof(light->position));
    memset(light->direction, 0, sizeof(light->direction));
    light->direction[2] = -1.0f; // Pointing downward
}

void tsfi_luxlight_set_energy(TSFiLuxLight *light, float intensity, float exposure) {
    if (!light) return;
    light->intensity = intensity;
    light->exposure = exposure;
}

float tsfi_luxlight_calculate_attenuation(const TSFiLuxLight *light, const float vertex[3]) {
    if (!light || !vertex) return 0.0f;
    
    float dx = vertex[0] - light->position[0];
    float dy = vertex[1] - light->position[1];
    float dz = vertex[2] - light->position[2];
    float distance_sq = dx*dx + dy*dy + dz*dz;
    
    // Total energy combining intensity and logarithmic exposure (Pixar standard)
    float base_energy = light->intensity * powf(2.0f, light->exposure);
    
    // Inverse-square law attenuation
    return base_energy / (distance_sq + 1.0f);
}

#include "tsfi_volumetric.h"
#include <math.h>

void tsfi_volumetric_init(TSFiVolumetric *vol, double density, double extinction_coeff) {
    if (!vol) return;
    vol->density = density;
    vol->extinction_coeff = extinction_coeff;
}

double tsfi_volumetric_raymarch(const TSFiVolumetric *vol, const TSFiShadowMap *sm, double start_depth, double end_depth, int steps) {
    if (!vol || !sm || steps <= 0) return 0.0;
    
    double step_size = (end_depth - start_depth) / steps;
    double accumulated_light = 0.0;
    double transmittance = 1.0;
    
    for (int i = 0; i < steps; i++) {
        double current_depth = start_depth + i * step_size;
        
        // Grid mapping coordinate simulation
        int x = (int)(current_depth) % TSFI_SHADOW_SIZE;
        int y = ((int)(current_depth) / TSFI_SHADOW_SIZE) % TSFI_SHADOW_SIZE;
        
        // Raymarch lookup checking light occlusion in shadow map
        bool in_shadow = tsfi_shadowmap_is_in_shadow(sm, x, y, current_depth);
        
        if (!in_shadow) {
            // Light scattering update based on transmittance and density attenuation
            double step_scattering = vol->density * step_size;
            accumulated_light += step_scattering * transmittance;
        }
        
        // Extinction update
        transmittance *= exp(-vol->extinction_coeff * step_size);
    }
    
    return accumulated_light;
}

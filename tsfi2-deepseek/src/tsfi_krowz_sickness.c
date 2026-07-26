#include "tsfi_krowz_sickness.h"
#include <math.h>

void tsfi_krowz_sickness_init(TSFiKrowzSickness *ks, double saturation_threshold, double glare_factor) {
    if (!ks) return;
    ks->saturation_threshold = saturation_threshold > 0.0 ? saturation_threshold : 0.8;
    ks->glare_factor = glare_factor >= 0.0 ? glare_factor : 0.5;
}

double tsfi_krowz_sickness_eval(const TSFiKrowzSickness *ks, double specular_intensity) {
    if (!ks || specular_intensity < 0.0) return 0.0;
    
    // Evaluate glare saturation threshold mapping kr0wZ sickness concept
    if (specular_intensity >= ks->saturation_threshold) {
        double delta = specular_intensity - ks->saturation_threshold;
        return specular_intensity * (1.0 + ks->glare_factor * exp(delta));
    }
    
    return specular_intensity;
}

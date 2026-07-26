#ifndef TSFI_KROWZ_SICKNESS_H
#define TSFI_KROWZ_SICKNESS_H

typedef struct {
    double saturation_threshold;
    double glare_factor;
} TSFiKrowzSickness;

// Initializes the kr0wZ sickness glare parameters
void tsfi_krowz_sickness_init(TSFiKrowzSickness *ks, double saturation_threshold, double glare_factor);

// Evaluates the specular intensity glare saturation mapping kr0wZ sickness
double tsfi_krowz_sickness_eval(const TSFiKrowzSickness *ks, double specular_intensity);

#endif // TSFI_KROWZ_SICKNESS_H

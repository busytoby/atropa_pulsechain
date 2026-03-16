#ifndef TSFI_TEMPORAL_WARPER_H
#define TSFI_TEMPORAL_WARPER_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void tsfi_latent_warp_translate(float *latent, int w, int h, int dx, int dy);
void tsfi_latent_mix(float *target, const float *source, float alpha, size_t count);

#ifdef __cplusplus
}
#endif

#endif // TSFI_TEMPORAL_WARPER_H

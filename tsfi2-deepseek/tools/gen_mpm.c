#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_types.h"

int main() {
    int states = 5;   
    int samples = 10; 
    
    TSFiPropertyVector *data = (TSFiPropertyVector*)malloc(states * samples * sizeof(TSFiPropertyVector));
    if (!data) return 1;
    memset(data, 0, states * samples * sizeof(TSFiPropertyVector));

    for (int s = 0; s < states; s++) {
        for (int p = 0; p < samples; p++) {
            TSFiPropertyVector *v = &data[s * samples + p];
            float progress = (float)p / (samples - 1.0f);
            
            if (s == 0) { 
                v->spectral_secret[0] = 0.2f; v->spectral_secret[1] = 0.5f; v->spectral_secret[2] = 0.1f;
                v->scale = 0.05f + progress * 0.1f;
                v->grain = 0.1f;
            } else if (s == 1) { 
                v->spectral_secret[0] = 0.1f; v->spectral_secret[1] = 0.9f; v->spectral_secret[2] = 0.2f;
                v->scale = 0.15f + progress * 0.5f;
                v->grain = 0.2f;
            } else if (s == 2) { 
                v->spectral_secret[0] = 0.6f; v->spectral_secret[1] = 0.7f; v->spectral_secret[2] = 0.5f;
                v->scale = 0.65f + progress * 0.25f;
                v->grain = 0.4f;
            } else if (s == 3) { 
                v->spectral_secret[0] = 1.0f; v->spectral_secret[1] = 0.6f; v->spectral_secret[2] = 1.2f; 
                v->scale = 0.9f + progress * 0.1f;
                v->grain = 0.8f; // High detail veins
            } else if (s == 4) { 
                v->spectral_secret[0] = 0.5f; v->spectral_secret[1] = 0.4f; v->spectral_secret[2] = 0.3f;
                v->scale = 1.0f - progress * 0.6f;
                v->grain = 1.2f; // Weathered texture
            }
            v->resonance = 1.0f + progress * 0.5f;
            v->density = 1.0f;
        }
    }

    FILE *f = fopen("assets/spectral/base_biological.mpm", "wb");
    if (f) {
        fwrite(&states, sizeof(int), 1, f);
        fwrite(&samples, sizeof(int), 1, f);
        fwrite(data, sizeof(TSFiPropertyVector), (size_t)(states * samples), f);
        fclose(f);
        printf("Successfully generated assets/spectral/base_biological.mpm\n");
    } else {
        perror("Failed to open file for writing");
    }

    free(data);
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tsfi_types.h"

void save_mpm(const char* path, int states, int samples, TSFiPropertyVector* data) {
    FILE *f = fopen(path, "wb");
    if (f) {
        fwrite(&states, sizeof(int), 1, f);
        fwrite(&samples, sizeof(int), 1, f);
        fwrite(data, sizeof(TSFiPropertyVector), (size_t)(states * samples), f);
        fclose(f);
        printf("Generated asset.\n");
    }
}

int main() {
    int states = 2; 
    int samples = 5;
    TSFiPropertyVector *data = (TSFiPropertyVector*)malloc(states * samples * sizeof(TSFiPropertyVector));

    memset(data, 0, states * samples * sizeof(TSFiPropertyVector));
    for (int i=0; i<states*samples; i++) {
        data[i].spectral_secret[0] = 0.3f; data[i].spectral_secret[1] = 0.5f; data[i].spectral_secret[2] = 0.1f;
        data[i].scale = 1.0f;
        data[i].grain = 0.8f;  
        data[i].density = 1.5f; 
    }
    save_mpm("assets/tissue/wood.mpm", states, samples, data);

    memset(data, 0, states * samples * sizeof(TSFiPropertyVector));
    for (int i=0; i<states*samples; i++) {
        data[i].spectral_secret[0] = 1.0f; data[i].spectral_secret[1] = 1.0f; data[i].spectral_secret[2] = 1.0f;
        data[i].scale = 1.0f;
        data[i].grain = 0.2f;  
        data[i].density = 0.4f; 
    }
    save_mpm("assets/tissue/petal.mpm", states, samples, data);

    free(data);
    return 0;
}
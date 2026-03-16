#include "tsfi_safetensors_cache.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s <model_path_1> [model_path_2] ...\n", argv[0]);
        return 1;
    }

    printf("=== TSFi Safetensors Warming System ===\n");
    for (int i = 1; i < argc; i++) {
        printf("[WARM] Attaching %s...\n", argv[i]);
        TsfiSafetensorsAsset *asset = tsfi_safetensors_cache_attach(argv[i]);
        if (asset) {
            printf("  [PASS] Matrix live in SHM (%zu bytes, ref_count=%u)\n", asset->size, asset->ref_count);
        } else {
            printf("  [FAIL] Could not warm model matrix.\n");
        }
    }

    printf("=== Warming Complete ===\n");
    return 0;
}

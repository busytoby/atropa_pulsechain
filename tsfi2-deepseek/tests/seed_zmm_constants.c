#include <stdio.h>
#include "tsfi_wave512.h"

int main() {
    // Verify wave512 structure size (must be 512 bytes / 64 bytes aligned)
    printf("[SEED] Wave512 Size: %zu\n", sizeof(wave512));
    return (sizeof(wave512) == 512) ? 0 : 1;
}

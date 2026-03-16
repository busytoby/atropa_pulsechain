#include <stdio.h>
#include "tsfi_hilbert.h"

int main() {
    int order = TSFI_HILBERT_ORDER;
    printf("[SEED] Hilbert Order: %d\n", order);
    return (order == 8) ? 0 : 1;
}

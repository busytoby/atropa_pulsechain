#include <stdio.h>
#include "lau_memory.h"

int main() {
    // Verify the fixed offset for Wired Payloads
    printf("[SEED] Wired Metadata Size: %zu\n", sizeof(LauWiredHeader));
    return 0;
}

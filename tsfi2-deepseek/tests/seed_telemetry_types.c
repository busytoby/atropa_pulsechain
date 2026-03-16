#include <stdio.h>
#include "lau_telemetry.h"

int main() {
    printf("[SEED] Telemetry Ring Size: %d\n", LAU_TELEM_RING_SIZE);
    return (LAU_TELEM_RING_SIZE == 1024) ? 0 : 1;
}

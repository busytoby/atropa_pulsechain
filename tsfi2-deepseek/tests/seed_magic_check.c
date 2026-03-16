#include <stdio.h>
#include <stdint.h>
#include "tsfi_types.h"

int main() {
    uint64_t magic = LAU_TELEMETRY_MAGIC;
    printf("[SEED] Magic: 0x%lx\n", magic);
    return (magic == 0xCAFEBABE12345678) ? 0 : 1;
}

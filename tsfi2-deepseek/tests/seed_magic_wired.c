#include <stdio.h>
#include <stdint.h>
#include "lau_memory.h"
int main() {
    uint64_t magic = LAU_MAGIC;
    printf("[SEED] Magic: 0x%lx\n", magic);
    return 0;
}

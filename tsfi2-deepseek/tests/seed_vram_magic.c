#include <stdio.h>
#include "lau_memory.h"

int main() {
    printf("[SEED] LAU_MAGIC: 0x%x\n", LAU_MAGIC);
    return (LAU_MAGIC == 0x42) ? 0 : 1;
}

#include <stdio.h>
#include <stdint.h>
int main() {
    uint64_t val = 0xCAFEBABE;
    printf("[RECOVERY] UInt64: 0x%lx\n", val);
    return (sizeof(uint64_t) == 8) ? 0 : 1;
}

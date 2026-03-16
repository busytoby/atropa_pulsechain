#include "tsfi_pulsechain.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    FILE *f = fopen("/tmp/rpc_cert.der", "rb");
    if(!f) return 1;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *buf = malloc(size);
    fread(buf, 1, size, f);
    fclose(f);
    TsfiPulseHash hash;
    tsfi_pulse_keccak256(buf, size, &hash);
    printf("KECCAK256: ");
    for(int i=0; i<32; i++) printf("%02x", hash.data[i]);
    printf("\nSIZE: %ld bytes\n", size);
    free(buf);
    return 0;
}

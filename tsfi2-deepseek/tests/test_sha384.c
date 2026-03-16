#include "tsfi_tls.h"
#include <stdio.h>
#include <string.h>

// External declarations of my internal functions for testing
typedef struct {
    uint64_t state[8];
    uint64_t count[2];
    uint8_t buffer[128];
} tsfi_sha384_context;

extern void tsfi_sha384_init(tsfi_sha384_context *ctx);
extern void tsfi_sha384_update(tsfi_sha384_context *ctx, const uint8_t *in, size_t len);
extern void tsfi_sha384_final(tsfi_sha384_context *ctx, uint8_t *out);

int main() {
    printf("[TSFI] Verifying SHA-384 Mathematical Correctness...\n");
    
    const char *msg = "abc";
    uint8_t hash[48];
    tsfi_sha384_context ctx;
    
    tsfi_sha384_init(&ctx);
    tsfi_sha384_update(&ctx, (uint8_t*)msg, 3);
    tsfi_sha384_final(&ctx, hash);
    
    // Expected SHA-384 for "abc":
    // cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7
    
    printf("Actual:   ");
    for(int i=0; i<48; i++) printf("%02x", hash[i]);
    printf("\nExpected: cb00753f45a35e8bb5a03d699ac65007272c32ab0eded1631a8b605a43ff5bed8086072ba1e7cc2358baeca134c825a7\n");
    
    if (memcmp(hash, "\xcb\x00\x75\x3f\x45\xa3\x5e\x8b\xb5\xa0\x3d\x69\x9a\xc6\x50\x07\x27\x2c\x32\xab\x0e\xde\xd1\x63\x1a\x8b\x60\x5a\x43\xff\x5b\xed\x80\x86\x07\x2b\xa1\xe7\xcc\x23\x58\xba\xec\xa1\x34\xc8\x25\xa7", 48) == 0) {
        printf("[SUCCESS] SHA-384 Math Verified.\n");
        return 0;
    } else {
        printf("[FAILURE] SHA-384 Math Mismatch!\n");
        return 1;
    }
}

#include "tsfi_math.h"
#include "lau_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

static void hex_to_bn(TSFiBigInt *bn, const char *hex) {
    if (strncmp(hex, "0x", 2) == 0) hex += 2;
    size_t len = strlen(hex);
    size_t byte_len = (len + 1) / 2;
    uint8_t *bytes = (uint8_t*)lau_malloc(byte_len);
    memset(bytes, 0, byte_len);
    for (size_t i = 0; i < len; i++) {
        char c = hex[len - 1 - i];
        int val = 0;
        if (c >= '0' && c <= '9') val = c - '0';
        else if (c >= 'a' && c <= 'f') val = c - 'a' + 10;
        else if (c >= 'A' && c <= 'F') val = c - 'A' + 10;
        bytes[byte_len - 1 - (i / 2)] |= (val << ((i % 2) * 4));
    }
    tsfi_bn_from_bytes(bn, bytes, byte_len);
    lau_free(bytes);
}

int main() {
    TSFiBigInt *a = tsfi_bn_alloc();
    TSFiBigInt *b = tsfi_bn_alloc();
    TSFiBigInt *mod = tsfi_bn_alloc();
    TSFiBigInt *res = tsfi_bn_alloc();

    hex_to_bn(a, "0x4805915ad7608bdf0a51b5f30cbfdbb924c5c90c4b510450d4fec3e2b1c59d79");
    hex_to_bn(b, "0x53f8d42d4159e7cb5094d08ceaaf7fc3");
    hex_to_bn(mod, "0xcc3a7079fe446dba713ea03991ade7381d1bac58d4fd4732623824f036d98c01");

    int iters = 100000;
    double t0, t_head;

    printf("=== TSFi Math Benchmarks ===\n");
    printf("Iterations: %d\n\n", iters);

    // ADD
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_add_avx512(res, a, b);
    t_head = get_time() - t0;
    printf("Add    : %.6f s\n", t_head);

    // SUB
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_sub_avx512(res, a, b);
    t_head = get_time() - t0;
    printf("Sub    : %.6f s\n", t_head);

    // MUL
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_mul_avx512(res, a, b);
    t_head = get_time() - t0;
    printf("Mul    : %.6f s\n", t_head);

    // LSHIFT
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_lshift_avx512(res, a, 10);
    t_head = get_time() - t0;
    printf("LShift : %.6f s\n", t_head);

    // RSHIFT
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_rshift_avx512(res, a, 10);
    t_head = get_time() - t0;
    printf("RShift : %.6f s\n", t_head);

    // DIV
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_div_avx512(res, NULL, a, b);
    t_head = get_time() - t0;
    printf("Div    : %.6f s\n", t_head);

    // MODPOW
    t0 = get_time();
    for (int i = 0; i < iters; i++) tsfi_bn_modpow_avx512(res, a, b, mod);
    t_head = get_time() - t0;
    printf("ModPow : %.6f s\n", t_head);

    printf("\n=== Memory Allocation Benchmarks ===\n");
    int mem_iters = 1000000;

    // Alloc + Free loop
    t0 = get_time();
    for (int i = 0; i < mem_iters; i++) {
        TSFiBigInt *tmp = tsfi_bn_alloc();
        tsfi_bn_free(tmp);
    }
    double t_alloc_free = get_time() - t0;
    printf("Alloc+Free (%d iters) : %.6f s\n", mem_iters, t_alloc_free);

    // Realloc loop
    TSFiBigInt *ptr = tsfi_bn_alloc();
    t0 = get_time();
    for (int i = 0; i < mem_iters; i++) {
        ptr = tsfi_bn_realloc(ptr);
    }
    double t_realloc = get_time() - t0;
    tsfi_bn_free(ptr);
    printf("Realloc    (%d iters) : %.6f s\n", mem_iters, t_realloc);
    printf("Speedup factor        : %.2fx\n", t_alloc_free / t_realloc);

    tsfi_bn_free(a);
    tsfi_bn_free(b);
    tsfi_bn_free(mod);
    tsfi_bn_free(res);
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

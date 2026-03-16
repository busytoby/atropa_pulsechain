#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>
#include "tsfi_broadcaster.h"
#include "lau_thunk.h"
#include "lau_memory.h"

extern void* ThunkProxy_emit_ansi_parser(ThunkProxy *p, int BASE_secret_bytes);

void test_parallel_BASE_update() {
    printf("[TEST] Verifying Parallel ANSI BASE Update (AVX-512)...\n");

    LauBroadcaster *lb = (LauBroadcaster *)lau_malloc_wired(sizeof(LauBroadcaster));
    assert(lb != NULL);
    memset(lb, 0, sizeof(LauBroadcaster));

    ThunkProxy *p = ThunkProxy_create();
    int BASE_offset = (int)offsetof(LauBroadcaster, char_BASE);
    printf("[TEST] BASE Offset: %d\n", BASE_offset);
    void *thunk = ThunkProxy_emit_ansi_parser(p, BASE_offset);
    assert(thunk != NULL);

    char *test_data = (char *)lau_malloc(64);
    for(int i=0; i<64; i++) test_data[i] = (char)('A' + (i % 26));
    printf("[TEST] lb: %p, test_data: %p\n", (void*)lb, (void*)test_data);

    typedef void (*parser_fn)(const void*);
    parser_fn run_parser = (parser_fn)thunk;

    __asm__ __volatile__(
        "push %%r15\n\t"
        "mov %0, %%r15\n\t"
        "mov %1, %%rdi\n\t"
        "call *%2\n\t"
        "pop %%r15\n\t"
        :
        : "r"(lb), "r"(test_data), "r"(run_parser)
        : "rax", "rdi", "memory"
    );

    printf("[TEST] First 16 bytes of BASE: ");
    for(int i=0; i<16; i++) printf("%02x ", (unsigned char)lb->char_BASE[0][i]);
    printf("\n");

    assert(memcmp(lb->char_BASE[0], test_data, 64) == 0);
    printf("[PASS] BASE Data Parity Verified (64-byte AVX-512 Burst).\n");

    ThunkProxy_destroy(p);
    lau_free(lb);
    lau_free(test_data);
}

int main() {
    printf("=== TSFi ANSI Parser Unit Test ===\n");
    test_parallel_BASE_update();
    printf("ALL ANSI PARSER TESTS PASSED\n");
        extern void lau_registry_teardown(void);
    lau_registry_teardown();
    extern void lau_report_memory_metrics(void);
    lau_report_memory_metrics();
    return 0;
}

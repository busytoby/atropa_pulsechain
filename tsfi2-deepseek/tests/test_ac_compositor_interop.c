#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include <assert.h>

// Functions declared in libmozilla_interop.c
extern void tsfi_compositor_cache_register(const char *key, const void *pre_data, size_t pre_size, const void *post_data, size_t post_size);
extern bool tsfi_compositor_cache_lookup(const char *pipeline_signature, void *pre_dest, size_t *pre_size_out, void *post_dest, size_t *post_size_out);

// Thread argument struct for parallel lookup stress testing
typedef struct {
    const char *signature;
    const char *expected_key;
    bool expected_found;
} StressThreadArg;

static void* lookup_stress_thread(void *arg) {
    StressThreadArg *targ = (StressThreadArg*)arg;
    uint8_t pre_buf[256] = {0};
    uint8_t post_buf[256] = {0};
    size_t pre_size = 0;
    size_t post_size = 0;

    for (int i = 0; i < 1000; i++) {
        bool found = tsfi_compositor_cache_lookup(targ->signature, pre_buf, &pre_size, post_buf, &post_size);
        assert(found == targ->expected_found);
        if (found) {
            // Verify cache contents match signature
            assert(pre_size == strlen(targ->expected_key) + 1);
            assert(memcmp(pre_buf, targ->expected_key, strlen(targ->expected_key)) == 0);
        }
    }
    return NULL;
}

int main() {
    printf("=== Auncient Compositor Caching Aho-Corasick Interop Test ===\n");

    // Dummy cache payloads
    const char *shader1_sig = "VS:layout(location=0) in vec3 pos; FS:out vec4 color;";
    const char *shader2_sig = "VS:layout(location=0) in vec2 uv; FS:uniform sampler2D tex; FS:out vec4 color;";
    const char *mismatch_sig = "VS:layout(location=0) in vec4 weights; FS:out vec4 color;";

    char pre1[256], post1[256];
    char pre2[256], post2[256];

    sprintf(pre1, "DATA:pre_%s", shader1_sig);
    sprintf(post1, "DATA:post_%s", shader1_sig);
    sprintf(pre2, "DATA:pre_%s", shader2_sig);
    sprintf(post2, "DATA:post_%s", shader2_sig);

    // 1. Register shader caching payloads using AC interop
    printf("[ACT] Registering layout signatures in compositor cache...\n");
    tsfi_compositor_cache_register(shader1_sig, pre1, strlen(pre1) + 1, post1, strlen(post1) + 1);
    tsfi_compositor_cache_register(shader2_sig, pre2, strlen(pre2) + 1, post2, strlen(post2) + 1);

    // 2. Perform verification lookups
    printf("[ACT] Performing cache lookups...\n");
    uint8_t rx_pre[256] = {0};
    uint8_t rx_post[256] = {0};
    size_t rx_pre_sz = 0;
    size_t rx_post_sz = 0;

    // Test lookup 1
    bool ok1 = tsfi_compositor_cache_lookup("Stream: VS:layout(location=0) in vec3 pos; FS:out vec4 color;", rx_pre, &rx_pre_sz, rx_post, &rx_post_sz);
    assert(ok1);
    assert(strcmp((char*)rx_pre, pre1) == 0);
    assert(strcmp((char*)rx_post, post1) == 0);
    printf("   - Lookup 1 (vec3 pos) MATCHED successfully.\n");

    // Test lookup 2
    bool ok2 = tsfi_compositor_cache_lookup("Stream: VS:layout(location=0) in vec2 uv; FS:uniform sampler2D tex; FS:out vec4 color; End", rx_pre, &rx_pre_sz, rx_post, &rx_post_sz);
    assert(ok2);
    assert(strcmp((char*)rx_pre, pre2) == 0);
    assert(strcmp((char*)rx_post, post2) == 0);
    printf("   - Lookup 2 (vec2 uv texture) MATCHED successfully.\n");

    // Test mismatch lookup
    bool ok_mismatch = tsfi_compositor_cache_lookup(mismatch_sig, rx_pre, &rx_pre_sz, rx_post, &rx_post_sz);
    assert(!ok_mismatch);
    printf("   - Mismatch lookup safely REJECTED successfully.\n");

    // 3. Thread safety stress testing
    printf("[ACT] Spawning parallel threads to stress-test concurrent cache access...\n");
    pthread_t thread1, thread2;
    StressThreadArg arg1 = { .signature = "Stream: VS:layout(location=0) in vec3 pos; FS:out vec4 color;", .expected_key = "DATA:pre_VS:layout(location=0) in vec3 pos; FS:out vec4 color;", .expected_found = true };
    StressThreadArg arg2 = { .signature = mismatch_sig, .expected_key = "", .expected_found = false };

    pthread_create(&thread1, NULL, lookup_stress_thread, &arg1);
    pthread_create(&thread2, NULL, lookup_stress_thread, &arg2);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    printf("[SUCCESS] Thread-safety and lookup integrity verified with zero issues.\n");
    printf("=== Aho-Corasick Compositor Caching Interop Test PASSED ===\n");
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <openssl/sha.h>

#define HASH_SIZE 32
#define CACHE_CAPACITY 8
#define REQUIRED_CLEARANCE 2

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
static void sha256(const void *data, size_t len, uint8_t *out) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, len);
    SHA256_Final(out, &ctx);
}
#pragma GCC diagnostic pop

typedef struct {
    uint8_t op_hash[HASH_SIZE];
    bool is_pure;
    bool active;
} vpp_cache_entry_t;

typedef struct {
    vpp_cache_entry_t entries[CACHE_CAPACITY];
    int size;
} vpp_verify_cache_t;

typedef struct {
    uint64_t sequence;
    char operation_cmd[64];
    uint8_t clearance;
} tx_packet_t;

static void init_verify_cache(vpp_verify_cache_t *cache) {
    memset(cache->entries, 0, sizeof(cache->entries));
    cache->size = 0;
}

// Fast lookup: check if bytecode/operation result is already verified in cache
static bool lookup_cache(const vpp_verify_cache_t *cache, const uint8_t *hash, bool *out_pure) {
    for (int i = 0; i < CACHE_CAPACITY; i++) {
        if (cache->entries[i].active && memcmp(cache->entries[i].op_hash, hash, HASH_SIZE) == 0) {
            *out_pure = cache->entries[i].is_pure;
            return true; // Cache hit
        }
    }
    return false; // Cache miss
}

// Insert verification result into cache
static void insert_cache(vpp_verify_cache_t *cache, const uint8_t *hash, bool is_pure) {
    int target_idx = cache->size % CACHE_CAPACITY;
    memcpy(cache->entries[target_idx].op_hash, hash, HASH_SIZE);
    cache->entries[target_idx].is_pure = is_pure;
    cache->entries[target_idx].active = true;
    cache->size++;
}

// Verification Node with Cache Bypass Shortcut
static bool verify_transaction_with_cache(vpp_verify_cache_t *cache, const tx_packet_t *pkt, int *out_cache_hits) {
    uint8_t op_hash[HASH_SIZE];
    sha256(pkt->operation_cmd, strlen(pkt->operation_cmd), op_hash);

    bool cached_pure = false;
    if (lookup_cache(cache, op_hash, &cached_pure)) {
        (*out_cache_hits)++;
        return cached_pure; // Fast shortcut path
    }

    // Slow path: full clearance and purity check
    bool is_pure = (pkt->clearance >= REQUIRED_CLEARANCE);
    
    // Store verification status in cache
    insert_cache(cache, op_hash, is_pure);
    return is_pure;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT VPP STATIC VERIFICATION CACHE SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    vpp_verify_cache_t cache;
    init_verify_cache(&cache);

    tx_packet_t tx1 = { .sequence = 1, .operation_cmd = "BYTE(62208) = 5;", .clearance = 2 };
    tx_packet_t tx2 = { .sequence = 2, .operation_cmd = "BYTE(100) = 2;", .clearance = 1 }; // Fails purity

    int cache_hits = 0;

    // 1. First run: Misses
    printf("[TEST] Executing first pass on transactions (expecting cache misses)...\n");
    fflush(stdout);

    bool ok1 = verify_transaction_with_cache(&cache, &tx1, &cache_hits);
    bool ok2 = verify_transaction_with_cache(&cache, &tx2, &cache_hits);

    assert(ok1 == true);
    assert(ok2 == false);
    assert(cache_hits == 0);
    printf("   ✓ Slow path processed successfully. Cache populated.\n");
    fflush(stdout);

    // 2. Second run: Hits (Bypass slow path)
    printf("[TEST] Executing second pass on identical transactions (expecting cache hits)...\n");
    fflush(stdout);

    ok1 = verify_transaction_with_cache(&cache, &tx1, &cache_hits);
    ok2 = verify_transaction_with_cache(&cache, &tx2, &cache_hits);

    assert(ok1 == true);
    assert(ok2 == false);
    assert(cache_hits == 2); // Both hits
    printf("   ✓ Cache hits verified. Slow verification path bypassed successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("VPP VERIFICATION CACHE TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

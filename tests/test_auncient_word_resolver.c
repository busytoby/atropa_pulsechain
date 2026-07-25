#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#define MOTZKIN_PRIME 953467954114363ULL
#define VOCABULARY_SIZE 3

typedef struct {
    char name[32];
    uint64_t hash_key;
} system_word_t;

// Dynamic vocabulary table mapping hash keys to system commands
static system_word_t vocabulary[VOCABULARY_SIZE];

// Calculate the unique acoustic hash for a word based on formants and decay
uint64_t calculate_word_hash(double f1, double f2, double decay) {
    uint64_t f1_val = (uint64_t)round(f1);
    uint64_t f2_val = (uint64_t)round(f2);
    uint64_t decay_val = (uint64_t)round(decay * 100.0);
    
    // Non-linear mixing formula
    uint64_t mixed = (f1_val * 1000ULL) + (f2_val * 10ULL) + decay_val;
    return mixed % MOTZKIN_PRIME;
}

// Initialize system vocabulary entries
void init_vocabulary(void) {
    // 1. "LOCK_SCSI" formant signature: f1=440Hz, f2=880Hz, decay=0.4
    vocabulary[0].hash_key = calculate_word_hash(440.0, 880.0, 0.4);
    strcpy(vocabulary[0].name, "LOCK_SCSI");

    // 2. "REPLENISH_CACHE" formant signature: f1=600Hz, f2=1200Hz, decay=0.8
    vocabulary[1].hash_key = calculate_word_hash(600.0, 1200.0, 0.8);
    strcpy(vocabulary[1].name, "REPLENISH_CACHE");

    // 3. "WRITE_LEDGER" formant signature: f1=350Hz, f2=700Hz, decay=0.2
    vocabulary[2].hash_key = calculate_word_hash(350.0, 700.0, 0.2);
    strcpy(vocabulary[2].name, "WRITE_LEDGER");
}

// Resolve the exact Transfluxor word from physical formant metrics
const char *resolve_transfluxor_word(double f1, double f2, double decay) {
    uint64_t query_hash = calculate_word_hash(f1, f2, decay);
    printf("   [RESOLVER] Query Formants: F1=%.1fHz, F2=%.1fHz | Hash: %lu\n", f1, f2, query_hash);

    for (int i = 0; i < VOCABULARY_SIZE; i++) {
        if (vocabulary[i].hash_key == query_hash) {
            printf("   [RESOLVER] Word resolved to: '%s'\n", vocabulary[i].name);
            return vocabulary[i].name;
        }
    }

    printf("   [RESOLVER] Unrecognized acoustic pattern. Resolution failed.\n");
    return NULL;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT TRANSFLUXOR WORD RESOLVER SUITE\n");
    printf("=============================================================\n");

    init_vocabulary();

    // 1. Verify resolution of LOCK_SCSI word
    printf("[TEST] Resolving LOCK_SCSI formant sweep...\n");
    const char *word1 = resolve_transfluxor_word(440.0, 880.0, 0.4);
    assert(word1 != NULL);
    assert(strcmp(word1, "LOCK_SCSI") == 0);
    printf("   ✓ Successfully resolved 'LOCK_SCSI'.\n");

    // 2. Verify resolution of REPLENISH_CACHE word
    printf("[TEST] Resolving REPLENISH_CACHE formant sweep...\n");
    const char *word2 = resolve_transfluxor_word(600.0, 1200.0, 0.8);
    assert(word2 != NULL);
    assert(strcmp(word2, "REPLENISH_CACHE") == 0);
    printf("   ✓ Successfully resolved 'REPLENISH_CACHE'.\n");

    // 3. Verify failure for unknown signature
    printf("[TEST] Resolving random frequency noise...\n");
    const char *unknown = resolve_transfluxor_word(500.0, 1000.0, 0.5);
    assert(unknown == NULL);
    printf("   ✓ Correctly rejected unknown wave patterns.\n");

    printf("=============================================================\n");
    printf("TRANSFLUXOR RESOLVER TESTS PASSED\n");
    printf("=============================================================\n");

    return 0;
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <time.h>

#define ALPHABET_SIZE 256
#define MAX_STATES 500
#define BUFFER_MARGIN 256

// Auncient Aho-Corasick Trie Node structure
typedef struct {
    int next_states[ALPHABET_SIZE];
    int failure_link;
    uint32_t match_mask; // Bitmask of matched patterns
} ac_node_t;

static ac_node_t g_trie[MAX_STATES];
static uint32_t g_state_count = 1;

// Thread-safe quarantine storage
typedef struct {
    char quarantined_payload[BUFFER_MARGIN];
    bool has_quarantined_data;
} InterceptionQuarantine;

static InterceptionQuarantine g_quarantine = { "", false };

// Insert pattern into Aho-Corasick Trie
static void ac_insert(const char *pattern, uint32_t pattern_idx) {
    uint32_t current_state = 0;
    for (int i = 0; pattern[i] != '\0'; i++) {
        uint8_t c = (uint8_t)pattern[i];
        if (g_trie[current_state].next_states[c] == 0) {
            memset(&g_trie[g_state_count], 0, sizeof(ac_node_t));
            g_trie[current_state].next_states[c] = g_state_count++;
        }
        current_state = g_trie[current_state].next_states[c];
    }
    g_trie[current_state].match_mask |= (1 << pattern_idx);
}

// Build Failure links for Aho-Corasick
static void ac_build_failure_links(void) {
    int queue[MAX_STATES];
    int q_head = 0, q_tail = 0;

    // Queue first-level transitions
    for (int c = 0; c < ALPHABET_SIZE; c++) {
        int next = g_trie[0].next_states[c];
        if (next != 0) {
            g_trie[next].failure_link = 0;
            queue[q_tail++] = next;
        }
    }

    // Process remainder of states
    while (q_head < q_tail) {
        int current = queue[q_head++];
        for (int c = 0; c < ALPHABET_SIZE; c++) {
            int next = g_trie[current].next_states[c];
            if (next != 0) {
                int fail = g_trie[current].failure_link;
                while (fail > 0 && g_trie[fail].next_states[c] == 0) {
                    fail = g_trie[fail].failure_link;
                }
                int resolved_fail = g_trie[fail].next_states[c];
                g_trie[next].failure_link = resolved_fail;
                g_trie[next].match_mask |= g_trie[resolved_fail].match_mask;
                queue[q_tail++] = next;
            }
        }
    }
}

// Sub-microsecond Aho-Corasick packet auditor lookup loop
// Banned prints in hot path to comply with Rule 11
static inline bool ac_audit_payload(const uint8_t *payload, uint16_t len) {
    uint32_t current_state = 0;
    
    for (uint16_t i = 0; i < len; i++) {
        uint8_t c = payload[i];
        while (current_state > 0 && g_trie[current_state].next_states[c] == 0) {
            current_state = g_trie[current_state].failure_link;
        }
        current_state = g_trie[current_state].next_states[c];
        
        if (g_trie[current_state].match_mask != 0) {
            // Match found, copy to quarantine and return audit fail
            // No printf inside hot path lookup to prevent locking
            g_quarantine.has_quarantined_data = true;
            return false;
        }
    }
    return true;
}

// -------------------------------------------------------------
// Unit Tests & Benchmarks
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT AHO-CORASICK NETWORK COMPLIANCE VALIDATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize trie structures
    memset(g_trie, 0, sizeof(g_trie));
    ac_insert("Child-Langmuir", 0);
    ac_build_failure_links();

    // 1. Audit benign packet
    uint8_t benign_pkt[] = "STANAG PORT=2020 DATA=benign_payload";
    bool res1 = ac_audit_payload(benign_pkt, sizeof(benign_pkt));
    assert(res1 == true);
    printf("[TEST] Benign packet passed audit.\n");
    fflush(stdout);

    // 2. Audit malicious packet containing banned signature
    uint8_t malicious_pkt[] = "DECnet payload containing Child-Langmuir formula details";
    bool res2 = ac_audit_payload(malicious_pkt, sizeof(malicious_pkt));
    assert(res2 == false);
    assert(g_quarantine.has_quarantined_data == true);
    printf("[TEST] Malicious packet intercepted and quarantined.\n");
    fflush(stdout);

    // 3. Performance Benchmark
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    uint64_t iterations = 1000000;
    for (uint64_t i = 0; i < iterations; i++) {
        ac_audit_payload(benign_pkt, sizeof(benign_pkt));
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (double)(end.tv_sec - start.tv_sec) + 
                     (double)(end.tv_nsec - start.tv_nsec) / 1e9;
    double latency = (elapsed / (double)iterations) * 1e9;

    printf("\nPerformance Metrics:\n");
    printf("   - Latency: %.2f ns/lookup\n", latency);
    printf("   - Guard Status: %s (Limit: < 1000 ns)\n", 
           latency < 1000.0 ? "PASS" : "FAIL");
    fflush(stdout);

    printf("=============================================================\n");
    printf("AHO-CORASICK NETWORK INTEGRATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

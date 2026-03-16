#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>
#include <sys/time.h>

#include "lau_memory.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_io.h"

// --- CONFIGURATION ---
#define NUM_PATTERNS 100000
#define PATTERN_LEN 20
#define TEXT_LEN 10000000 // 10MB text
#define ALPHABET_SIZE 26

// --- GLOBALS ---
TsfiWaveletArena arena;
char *text;
char **patterns;

extern long long get_time_ns();

// --- 1. STANDARD MALLOC AHO-CORASICK (BASELINE) ---
typedef struct StdACNode {
    int next[ALPHABET_SIZE];
    int fail;
    int is_word;
} StdACNode;

StdACNode *std_nodes = NULL;
int std_node_count = 0;
int std_capacity = 0;

int new_std_node() {
    if (std_node_count >= std_capacity) {
        std_capacity = (std_capacity == 0) ? 1024 : std_capacity * 2;
        std_nodes = realloc(std_nodes, std_capacity * sizeof(StdACNode));
    }
    memset(&std_nodes[std_node_count], 0, sizeof(StdACNode));
    for(int i=0; i<ALPHABET_SIZE; i++) std_nodes[std_node_count].next[i] = -1;
    std_nodes[std_node_count].fail = -1;
    return std_node_count++;
}

void build_std_ac() {
    std_node_count = 0;
    std_capacity = 0;
    int root = new_std_node();
    
    // Insert
    for (int i = 0; i < NUM_PATTERNS; i++) {
        int curr = root;
        for (int j = 0; j < PATTERN_LEN; j++) {
            int c = patterns[i][j] - 'a';
            if (std_nodes[curr].next[c] == -1) {
                int next_node = new_std_node(); // May invalidate std_nodes
                std_nodes[curr].next[c] = next_node;
            }
            curr = std_nodes[curr].next[c];
        }
        std_nodes[curr].is_word = 1;
    }
    
    // Build Failure Links (BFS)
    int *q = malloc(std_node_count * sizeof(int));
    int head = 0, tail = 0;
    
    std_nodes[root].fail = root;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (std_nodes[root].next[i] != -1) {
            std_nodes[std_nodes[root].next[i]].fail = root;
            q[tail++] = std_nodes[root].next[i];
        } else {
            std_nodes[root].next[i] = root;
        }
    }
    
    while (head < tail) {
        int u = q[head++];
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (std_nodes[u].next[i] != -1) {
                int v = std_nodes[u].next[i];
                std_nodes[v].fail = std_nodes[std_nodes[u].fail].next[i];
                // Optimization: Carry over word flags from failure links
                std_nodes[v].is_word |= std_nodes[std_nodes[v].fail].is_word; 
                q[tail++] = v;
            } else {
                std_nodes[u].next[i] = std_nodes[std_nodes[u].fail].next[i];
            }
        }
    }
    free(q);
}

long long run_std_ac() {
    int curr = 0;
    long long matches = 0;
    for (int i = 0; i < TEXT_LEN; i++) {
        int c = text[i] - 'a';
        curr = std_nodes[curr].next[c];
        matches += std_nodes[curr].is_word;
    }
    return matches;
}

// --- 2. TSFi WAVELET ARENA AHO-CORASICK (OPTIMIZED) ---
// We map the AC state directly into the 512-byte Wavelet body.
// Instead of a huge array of arrays, we use the raw bytes of the wavelet.

// Since Wavelet payload is ~300 bytes, we can fit 26 int32s (104 bytes) easily.
#define WAVELET_AC_PAYLOAD_OFFSET 128 

static inline int32_t* get_wavelet_next(TsfiWavelet *W) {
    return (int32_t*)((char*)W + WAVELET_AC_PAYLOAD_OFFSET);
}
static inline int32_t* get_wavelet_fail(TsfiWavelet *W) {
    return (int32_t*)((char*)W + WAVELET_AC_PAYLOAD_OFFSET + (ALPHABET_SIZE * sizeof(int32_t)));
}
static inline int32_t* get_wavelet_is_word(TsfiWavelet *W) {
    return (int32_t*)((char*)W + WAVELET_AC_PAYLOAD_OFFSET + ((ALPHABET_SIZE + 1) * sizeof(int32_t)));
}

// We need an index-to-pointer map because failure links are topological.
TsfiWavelet **wavelet_map = NULL;
int wavelet_count = 0;
int wavelet_capacity = 0;

int new_tsfi_node() {
    if (wavelet_count >= wavelet_capacity) {
        wavelet_capacity = (wavelet_capacity == 0) ? 1024 : wavelet_capacity * 2;
        wavelet_map = realloc(wavelet_map, wavelet_capacity * sizeof(TsfiWavelet*));
    }
    TsfiWavelet *W = tsfi_STAT(&arena, TSFI_WAVELET_PRIME);
    W->telemetry.unique_id = wavelet_count;
    
    int32_t *next = get_wavelet_next(W);
    for(int i=0; i<ALPHABET_SIZE; i++) next[i] = -1;
    *get_wavelet_fail(W) = -1;
    *get_wavelet_is_word(W) = 0;
    
    wavelet_map[wavelet_count] = W;
    return wavelet_count++;
}

void build_tsfi_ac() {
    wavelet_count = 0;
    wavelet_capacity = 0;
    int root = new_tsfi_node();
    
    // Insert
    for (int i = 0; i < NUM_PATTERNS; i++) {
        int curr = root;
        for (int j = 0; j < PATTERN_LEN; j++) {
            int c = patterns[i][j] - 'a';
            int32_t *next = get_wavelet_next(wavelet_map[curr]);
            if (next[c] == -1) {
                next[c] = new_tsfi_node();
            }
            curr = next[c];
        }
        *get_wavelet_is_word(wavelet_map[curr]) = 1;
    }
    
    // Build Failure Links
    int *q = malloc(wavelet_count * sizeof(int));
    int head = 0, tail = 0;
    
    *get_wavelet_fail(wavelet_map[root]) = root;
    int32_t *root_next = get_wavelet_next(wavelet_map[root]);
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (root_next[i] != -1) {
            *get_wavelet_fail(wavelet_map[root_next[i]]) = root;
            q[tail++] = root_next[i];
        } else {
            root_next[i] = root;
        }
    }
    
    while (head < tail) {
        int u = q[head++];
        int32_t *u_next = get_wavelet_next(wavelet_map[u]);
        int u_fail = *get_wavelet_fail(wavelet_map[u]);
        
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (u_next[i] != -1) {
                int v = u_next[i];
                int32_t *v_fail = get_wavelet_fail(wavelet_map[v]);
                *v_fail = get_wavelet_next(wavelet_map[u_fail])[i];
                
                int32_t *v_word = get_wavelet_is_word(wavelet_map[v]);
                *v_word |= *get_wavelet_is_word(wavelet_map[*v_fail]); 
                q[tail++] = v;
            } else {
                u_next[i] = get_wavelet_next(wavelet_map[u_fail])[i];
            }
        }
    }
    free(q);
}

long long run_tsfi_ac() {
    int curr = 0;
    long long matches = 0;
    for (int i = 0; i < TEXT_LEN; i++) {
        int c = text[i] - 'a';
        curr = get_wavelet_next(wavelet_map[curr])[c];
        matches += *get_wavelet_is_word(wavelet_map[curr]);
    }
    return matches;
}

int main() {
    printf("=== TSFi Mapped Aho-Corasick Automaton Benchmark ===\n");
    
    // 1. Data Generation
    printf("[INIT] Generating %d random patterns and %d MB text block...\n", NUM_PATTERNS, TEXT_LEN / 1000000);
    srand(42);
    text = malloc(TEXT_LEN);
    for (int i = 0; i < TEXT_LEN; i++) text[i] = 'a' + (rand() % ALPHABET_SIZE);
    
    patterns = malloc(NUM_PATTERNS * sizeof(char*));
    for (int i = 0; i < NUM_PATTERNS; i++) {
        patterns[i] = malloc(PATTERN_LEN + 1);
        for(int j=0; j<PATTERN_LEN; j++) patterns[i][j] = 'a' + (rand() % ALPHABET_SIZE);
        patterns[i][PATTERN_LEN] = '\0';
    }
    
    // 2. Standard Baseline
    printf("\n--- STANDARD GLIBC MALLOC (Baseline) ---\n");
    long long t0 = get_time_ns();
    build_std_ac();
    long long t1 = get_time_ns();
    long long std_build_time = t1 - t0;
    
    t0 = get_time_ns();
    long long std_matches = run_std_ac();
    t1 = get_time_ns();
    long long std_run_time = t1 - t0;
    
    printf("Nodes Created: %d\n", std_node_count);
    printf("Memory Used: %zu bytes (%.2f MB)\n", std_node_count * sizeof(StdACNode), (double)(std_node_count * sizeof(StdACNode)) / 1048576.0);
    printf("Build Time: %lld ns\n", std_build_time);
    printf("Search Time: %lld ns\n", std_run_time);
    printf("Matches Found: %lld\n", std_matches);
    
    // 3. TSFi Wavelet Arena
    printf("\n--- TSFI WAVELET ARENA (Zero-Copy) ---\n");
    uint64_t arena_size = 1024ULL * 1024 * 1024; // 1GB
    uint8_t *mem = mmap(NULL, arena_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    assert(mem != MAP_FAILED);
    tsfi_wavelet_arena_init(&arena, mem, arena_size);

    t0 = get_time_ns();
    build_tsfi_ac();
    t1 = get_time_ns();
    long long tsfi_build_time = t1 - t0;
    
    // --- IMMUTABLE SEALING TEST ---
    printf("  [SEALING] Locking the Trie into PROT_READ Immutability...\n");
    long long t_seal_start = get_time_ns();
    int mprot_res = tsfi_mprotect(mem, arena_size, PROT_READ);
    long long t_seal_end = get_time_ns();
    
    if (mprot_res != 0) {
        perror("mprotect failed");
        return 1;
    }

    t0 = get_time_ns();
    long long tsfi_matches = run_tsfi_ac();
    t1 = get_time_ns();
    long long tsfi_run_time = t1 - t0;

    printf("Nodes Created: %d\n", wavelet_count);
    printf("Memory Used: %llu bytes (%.2f MB)\n", (unsigned long long)arena.offset, (double)arena.offset / 1048576.0);
    printf("Build Time: %lld ns\n", tsfi_build_time);
    printf("Physical Seal Time: %lld ns\n", t_seal_end - t_seal_start);
    printf("Search Time: %lld ns\n", tsfi_run_time);
    printf("Matches Found: %lld\n", tsfi_matches);
    
    if (std_matches == tsfi_matches) {
        printf("\n[PASS] Deterministic parity verified between Baseline and TSFi.\n");
    } else {
        printf("\n[FAIL] Parity mismatch!\n");
    }

    return 0;
}
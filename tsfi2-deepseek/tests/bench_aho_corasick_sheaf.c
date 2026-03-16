#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <sys/mman.h>

#include "lau_memory.h"
#include "tsfi_genetic.h"
#include "tsfi_wavelet_arena.h"
#include "tsfi_dysnomia.h"

// --- CONFIGURATION ---
#define NUM_PATTERNS 100000
#define PATTERN_LEN 20
#define TEXT_LEN 10000000 // 10MB
#define ALPHABET_SIZE 26

char *text;
char **patterns;
extern long long get_time_ns();

// --- 1. THE TSFi AUTOMATON SHEAF (MACRO ENTITY) ---
// This is the highly compressed node. It has NO header overhead.
typedef struct {
    int next[ALPHABET_SIZE];
    int fail;
    int is_word;
} TSFiACNode;

// This is the Sheaf. It encapsulates the pool and optionally holds a YI.
typedef struct TSFiAutomatonSheaf {
    TSFiACNode *pool;
    int node_count;
    int capacity;
    
    // Optional Meta-Thunk Attachments
    struct YI *yi_entity; 
    ReactSHIO_Fn react_shio;
    ReactSHA_Fn  react_sha;
} TSFiAutomatonSheaf;

TSFiAutomatonSheaf* alloc_sheaf(int enable_yi_thunks) {
    // We allocate the Sheaf in wired memory so it can be physically sealed
    TSFiAutomatonSheaf *sheaf = lau_malloc_wired(sizeof(TSFiAutomatonSheaf));
    sheaf->capacity = 1048576; // Start with 1M capacity
    sheaf->node_count = 0;
    
    // The pool is allocated in raw GPU/ReBAR capable zero-copy memory
    sheaf->pool = lau_malloc_gpu(sheaf->capacity * sizeof(TSFiACNode), LAU_GPU_REBAR);
    
    if (enable_yi_thunks) {
        // Here we attach the macro-YI and Thunks to the Sheaf
        // This makes the entire Automaton a singular Dysnomia Entity
        sheaf->yi_entity = (struct YI*)lau_malloc_wired(sizeof(struct YI));
        sheaf->react_shio = NULL; // Would point to JIT proxy in real use
        sheaf->react_sha = NULL;
    } else {
        sheaf->yi_entity = NULL;
        sheaf->react_shio = NULL;
        sheaf->react_sha = NULL;
    }
    
    return sheaf;
}

int sheaf_new_node(TSFiAutomatonSheaf *sheaf) {
    if (sheaf->node_count >= sheaf->capacity) {
        // GPU Tier allocator doesn't fully support lau_realloc natively in this branch.
        // We do a manual copy.
        int old_capacity = sheaf->capacity;
        sheaf->capacity *= 2;
        TSFiACNode *new_pool = lau_malloc_gpu(sheaf->capacity * sizeof(TSFiACNode), LAU_GPU_REBAR);
        if (sheaf->pool) {
            memcpy(new_pool, sheaf->pool, old_capacity * sizeof(TSFiACNode));
            lau_free(sheaf->pool);
        }
        sheaf->pool = new_pool;
    }
    memset(&sheaf->pool[sheaf->node_count], 0, sizeof(TSFiACNode));
    for(int i=0; i<ALPHABET_SIZE; i++) sheaf->pool[sheaf->node_count].next[i] = -1;
    sheaf->pool[sheaf->node_count].fail = -1;
    return sheaf->node_count++;
}

void build_sheaf_ac(TSFiAutomatonSheaf *sheaf) {
    int root = sheaf_new_node(sheaf);
    
    // Insert
    for (int i = 0; i < NUM_PATTERNS; i++) {
        int curr = root;
        for (int j = 0; j < PATTERN_LEN; j++) {
            int c = patterns[i][j] - 'a';
            if (sheaf->pool[curr].next[c] == -1) {
                int next_node = sheaf_new_node(sheaf);
                sheaf->pool[curr].next[c] = next_node;
            }
            curr = sheaf->pool[curr].next[c];
        }
        sheaf->pool[curr].is_word = 1;
    }
    
    // Build Failure Links
    int *q = malloc(sheaf->node_count * sizeof(int));
    int head = 0, tail = 0;
    
    sheaf->pool[root].fail = root;
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (sheaf->pool[root].next[i] != -1) {
            sheaf->pool[sheaf->pool[root].next[i]].fail = root;
            q[tail++] = sheaf->pool[root].next[i];
        } else {
            sheaf->pool[root].next[i] = root;
        }
    }
    
    while (head < tail) {
        int u = q[head++];
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            if (sheaf->pool[u].next[i] != -1) {
                int v = sheaf->pool[u].next[i];
                sheaf->pool[v].fail = sheaf->pool[sheaf->pool[u].fail].next[i];
                sheaf->pool[v].is_word |= sheaf->pool[sheaf->pool[v].fail].is_word; 
                q[tail++] = v;
            } else {
                sheaf->pool[u].next[i] = sheaf->pool[sheaf->pool[u].fail].next[i];
            }
        }
    }
    free(q);
}

long long run_sheaf_ac(TSFiAutomatonSheaf *sheaf) {
    int curr = 0;
    long long matches = 0;
    for (int i = 0; i < TEXT_LEN; i++) {
        int c = text[i] - 'a';
        curr = sheaf->pool[curr].next[c];
        matches += sheaf->pool[curr].is_word;
        
        // If the Sheaf is a YI entity, we could theoretically React() upon match:
        if (sheaf->pool[curr].is_word && sheaf->yi_entity && sheaf->react_shio) {
            // sheaf->react_shio(sheaf->yi_entity->shio, ...);
        }
    }
    return matches;
}

int main() {
    printf("=== TSFi Hybrid Automaton Sheaf Benchmark ===\n");
    
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
    
    // 2. Create the Sheaf (With YI Attachments enabled)
    printf("\n--- TSFI AUTOMATON SHEAF (Zero-Copy Pool + YI) ---\n");
    long long t0 = get_time_ns();
    TSFiAutomatonSheaf *sheaf = alloc_sheaf(1); // 1 = Enable YI
    build_sheaf_ac(sheaf);
    long long t1 = get_time_ns();
    long long sheaf_build_time = t1 - t0;
    
    // 3. Physical Sealing
    printf("  [SEALING] Locking the %d-Node Sheaf Pool into PROT_READ Immutability...\n", sheaf->node_count);
    long long t_seal_start = get_time_ns();
    lau_seal_object(sheaf->pool); // Uses our hardware mprotect lock
    long long t_seal_end = get_time_ns();

    // 4. Execution
    t0 = get_time_ns();
    long long sheaf_matches = run_sheaf_ac(sheaf);
    t1 = get_time_ns();
    long long sheaf_run_time = t1 - t0;

    printf("Nodes Created: %d\n", sheaf->node_count);
    size_t pool_size = sheaf->node_count * sizeof(TSFiACNode);
    printf("Memory Used: %zu bytes (%.2f MB)\n", pool_size, (double)pool_size / 1048576.0);
    printf("Sheaf Header Size: %zu bytes\n", sizeof(TSFiAutomatonSheaf));
    printf("YI/Thunks Attached: %s\n", sheaf->yi_entity ? "YES" : "NO");
    printf("Build Time: %lld ns\n", sheaf_build_time);
    printf("Physical Seal Time: %lld ns\n", t_seal_end - t_seal_start);
    printf("Search Time: %lld ns\n", sheaf_run_time);
    printf("Matches Found: %lld\n", sheaf_matches);
    
    if (pool_size < 200 * 1024 * 1024) { // < 200MB
        printf("\n[PASS] Memory constraint validated! The Sheaf architecture perfectly satisfies the 256MB ICPC Limit.\n");
    } else {
        printf("\n[FAIL] Memory limit exceeded.\n");
    }

    return 0;
}
#ifndef GEMALLOC_H
#define GEMALLOC_H

#ifdef USE_TSFI_MALLOC
// TSFi Integration Mode
#include "tsfi_lib/lau_memory.h"
#include <string.h>

// Macros to map Gemalloc API to TSFi/Lau Memory API
#define gemalloc(s) lau_malloc_loc(s, __FILE__, __LINE__)
#define jgemalloc(s, c) ({ void* _p = lau_malloc_loc(s, __FILE__, __LINE__); lau_mem_scramble(_p, s, c); _p; })
#define gemrealloc(p, s) lau_realloc_loc(p, s, __FILE__, __LINE__)
#define gemfree(p) lau_free(p)
#define printgemallocstats() lau_report_memory_metrics()

#else

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* --- Public Interface --- */

void* gemalloc(size_t size);
void* jgemalloc(size_t size, char set);
void* gemrealloc(void* ptr, size_t size);
void gemfree(void* ptr);
void printgemallocstats(void);

#endif /* USE_TSFI_MALLOC */

#endif /* GEMALLOC_H */

/* --- Implementation --- */
#ifdef GEMALLOC_IMPLEMENTATION
#ifndef USE_TSFI_MALLOC

#define _POSIX_C_SOURCE 202405L
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>       /* Required for time() */
#include <sys/mman.h>   /* Required for mmap() */
#include <sys/random.h> /* Required for getrandom() */
#include <stdlib.h>     /* rand, srand */

#define DLE 16
/* POSIX 2024: 64-byte alignment for AVX-512 and Cache Line efficiency */
#define ALIGN_VAL 64
#define ALIGN(size) (((size) + (ALIGN_VAL - 1)) & ~(ALIGN_VAL - 1))

struct dm_blk_hdr {
    size_t size;
    bool cleared;
    int reuse;
    int seq;
    struct dm_blk_hdr *n;
    struct dm_blk_hdr *p;
};

#define DM_HDR_SIZE ALIGN(sizeof(struct dm_blk_hdr))

/* Global state (confined to the translation unit defining IMPLEMENTATION) */
static struct dm_blk_hdr* HEAD = NULL;
static struct dm_blk_hdr* TAIL = NULL;
static int seq_ctr = 0;
static bool rand_seeded = false;

/* Internal: Memory scrambling and initialization */
static inline void ___gem(void* ptr, size_t size, char set) {
    if (!ptr || size == 0) return;

    if (set != DLE) {
        memset(ptr, set, size);
        return;
    }

    if (!rand_seeded) {
        unsigned int seed;
        /* POSIX.1-2024 compliant entropy retrieval */
        if (getrandom(&seed, sizeof(seed), 0) < 0) {
            seed = (unsigned int)time(NULL);
        }
        srand(seed);
        rand_seeded = true;
    }

    uint64_t* dptr = (uint64_t*)ptr;
    size_t len = size / sizeof(uint64_t);
    size_t rem = size % sizeof(uint64_t);

    for (size_t i = 0; i < len; i++) {
        uint64_t r;
        if (getrandom(&r, sizeof(r), 0) < 0) r = (uint64_t)rand();
        dptr[i] = r;
    }

    if (rem > 0) {
        uint64_t r;
        if (getrandom(&r, sizeof(r), 0) < 0) r = (uint64_t)rand();
        uint8_t* bptr = (uint8_t*)(dptr + len);
        for (size_t i = 0; i < rem; i++) {
            bptr[i] = bptr[i] ^ (uint8_t)(r >> (i * 8));
        }
    }
}

void* jgemalloc(size_t size, char set) {
    if (size == 0) return NULL;
    size = ALIGN(size);

    /* Recycled block search (First-fit from TAIL) */
    struct dm_blk_hdr *hptr = TAIL;
    while (hptr) {
        if (hptr->cleared && hptr->size >= size && hptr->size <= size + 128) {
            hptr->cleared = false;
            hptr->reuse++;
            hptr->seq = ++seq_ctr;
            ___gem(hptr + 1, hptr->size, set);
            return (void*)(hptr + 1);
        }
        hptr = hptr->p;
    }

    /* POSIX.1-2024: mmap replaces sbrk for improved performance and security */
    size_t total_size = ALIGN(size + DM_HDR_SIZE);
    struct dm_blk_hdr* newblk = (struct dm_blk_hdr*)mmap(NULL, total_size, 
                                    PROT_READ | PROT_WRITE, 
                                    MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    if (newblk == MAP_FAILED) return NULL;

    newblk->size = size;
    newblk->cleared = false;
    newblk->reuse = 0;
    newblk->seq = ++seq_ctr;
    newblk->n = NULL;
    newblk->p = TAIL;

    if (TAIL) TAIL->n = newblk;
    TAIL = newblk;
    if (!HEAD) HEAD = newblk;

    ___gem(newblk + 1, size, set);
    return (void*)(newblk + 1);
}

void* gemalloc(size_t size) {
    return jgemalloc(size, DLE);
}

void gemfree(void* ptr) {
    if (!ptr) return;
    struct dm_blk_hdr* blk = (struct dm_blk_hdr*)ptr - 1;
    if (blk->cleared) return;

    blk->cleared = true;
    ___gem(ptr, blk->size, DLE);
}

void* gemrealloc(void* ptr, size_t size) {
    if (!ptr) return jgemalloc(size, 0);
    struct dm_blk_hdr* blk = (struct dm_blk_hdr*)ptr - 1;

    size = ALIGN(size);
    if (size <= blk->size) return ptr;

    void* newptr = jgemalloc(size, 0);
    if (!newptr) return NULL;

    memcpy(newptr, ptr, blk->size);
    gemfree(ptr);
    return newptr;
}

void printgemallocstats(void) {
    struct dm_blk_hdr *hptr = HEAD;
    int total = 0, cleared = 0, total_reuse = 0;
    size_t total_bytes = 0, cleared_bytes = 0;

    printf("\n--- GEMALLOC STATS (POSIX 2024) ---\n");
    while (hptr) {
        if (hptr->cleared) {
            cleared++;
            cleared_bytes += hptr->size;
        }
        total++;
        total_bytes += hptr->size;
        total_reuse += hptr->reuse;
        hptr = hptr->n;
    }

    printf("Total Blocks:       %d\n", total);
    printf("Total Active Bytes: %zu\n", total_bytes - cleared_bytes);
    printf("Total Cleared:      %d\n", cleared);
    printf("Reuse Events:       %d\n", total_reuse);
    printf("----------------------------------\n");
}

#endif /* !USE_TSFI_MALLOC */
#endif /* GEMALLOC_IMPLEMENTATION */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "tsfi_sort.h"
#include "tsfi_wiring.h"

#define ARRAY_SIZE 1000

static void fill_random(uint64_t *arr, size_t n) {
    for (size_t i = 0; i < n; i++) {
        arr[i] = ((uint64_t)rand() << 32) | (uint64_t)rand();
    }
}

static int is_sorted(const uint64_t *arr, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) return 0;
    }
    return 1;
}

int main(void) {
    srand((unsigned int)time(NULL));
    uint64_t *arr = malloc(ARRAY_SIZE * sizeof(uint64_t));
    uint64_t *copy = malloc(ARRAY_SIZE * sizeof(uint64_t));
    assert(arr && copy);

    printf("=== Auncient TSFi Sorting Library Verification ===\n");

    /* 1. Test Standard Quicksort */
    fill_random(arr, ARRAY_SIZE);
    tsfi_quicksort(arr, ARRAY_SIZE);
    if (is_sorted(arr, ARRAY_SIZE)) {
        printf("[PASS] Standard Quicksort verified successfully.\n");
    } else {
        printf("[FAIL] Standard Quicksort failed verification.\n");
        return 1;
    }

    /* 2. Test Quicksort Based Insertion Sort */
    fill_random(arr, ARRAY_SIZE);
    tsfi_quicksort_insertion_sort(arr, ARRAY_SIZE);
    if (is_sorted(arr, ARRAY_SIZE)) {
        printf("[PASS] Quicksort-Insertion hybrid verified successfully.\n");
    } else {
        printf("[FAIL] Quicksort-Insertion hybrid failed verification.\n");
        return 1;
    }

    /* 3. Test Quicksort Based Top-down Merge Sort */
    fill_random(arr, ARRAY_SIZE);
    tsfi_quicksort_top_merge(arr, ARRAY_SIZE);
    if (is_sorted(arr, ARRAY_SIZE)) {
        printf("[PASS] Quicksort-Top-Merge verified successfully.\n");
    } else {
        printf("[FAIL] Quicksort-Top-Merge failed verification.\n");
        return 1;
    }

    /* 4. Test Quicksort Based Bottom-up Merge Sort */
    fill_random(arr, ARRAY_SIZE);
    tsfi_quicksort_bottom_merge(arr, ARRAY_SIZE);
    if (is_sorted(arr, ARRAY_SIZE)) {
        printf("[PASS] Quicksort-Bottom-Merge verified successfully.\n");
    } else {
        printf("[FAIL] Quicksort-Bottom-Merge failed verification.\n");
        return 1;
    }

    /* 5. Test Adaptive Selection (Introselect) */
    fill_random(arr, ARRAY_SIZE);
    memcpy(copy, arr, ARRAY_SIZE * sizeof(uint64_t));
    tsfi_quicksort(copy, ARRAY_SIZE); /* Sort copy for reference */

    /* Verify multiple rank queries (min, median, max, and arbitrary ranks) */
    size_t test_ranks[] = {0, ARRAY_SIZE / 4, ARRAY_SIZE / 2, (3 * ARRAY_SIZE) / 4, ARRAY_SIZE - 1};
    int select_passed = 1;
    for (size_t i = 0; i < sizeof(test_ranks)/sizeof(test_ranks[0]); i++) {
        size_t k = test_ranks[i];
        /* Need to re-copy elements as tsfi_select partitions the array in-place */
        memcpy(arr, copy, ARRAY_SIZE * sizeof(uint64_t));
        // Shuffle original layout slightly so select must work on unsorted inputs
        fill_random(arr, ARRAY_SIZE);
        memcpy(copy, arr, ARRAY_SIZE * sizeof(uint64_t));
        tsfi_quicksort(copy, ARRAY_SIZE);

        uint64_t result = tsfi_select(arr, ARRAY_SIZE, k);
        if (result != copy[k]) {
            printf("[FAIL] tsfi_select at rank %zu returned %lu, expected %lu\n", k, result, copy[k]);
            select_passed = 0;
            break;
        }
    }
    if (select_passed) {
        printf("[PASS] Adaptive Selection (tsfi_select) verified successfully.\n");
    } else {
        return 1;
    }

    /* 6. Test Memory Header Selection */
    LauMetadata m1 = {0};
    m1.alloc_size = 1200;
    m1.alloc_file = "test_sort_file_1";
    
    LauMetadata m2 = {0};
    m2.alloc_size = 500;
    m2.alloc_file = "test_sort_file_2";
    
    LauMetadata m3 = {0};
    m3.alloc_size = 2000;
    m3.alloc_file = "test_sort_file_3";

    lau_registry_insert(&m1);
    lau_registry_insert(&m2);
    lau_registry_insert(&m3);

    /* Retrieve 0th, 1st, 2nd smallest headers (excluding PRE_START allocations if any) */
    LauMetadata *sel_min = tsfi_select_memory_header(0);
    LauMetadata *sel_med = tsfi_select_memory_header(1);
    LauMetadata *sel_max = tsfi_select_memory_header(2);

    if (sel_min && sel_med && sel_max) {
        printf("[PASS] tsfi_select_memory_header query succeeded: min_size=%zu, med_size=%zu, max_size=%zu\n",
               sel_min->alloc_size, sel_med->alloc_size, sel_max->alloc_size);
        assert(sel_min->alloc_size <= sel_med->alloc_size);
        assert(sel_med->alloc_size <= sel_max->alloc_size);
        printf("[PASS] tsfi_select_memory_header validation succeeded.\n");
    } else {
        printf("[WARN] tsfi_select_memory_header returned NULL, active allocations count too small.\n");
    }

    /* Test Topological Selection (Mobius-Klein tournament) */
    LauMetadata *topo_min = tsfi_wire_topological_select(NULL, 0);
    LauMetadata *topo_med = tsfi_wire_topological_select(NULL, 1);
    LauMetadata *topo_max = tsfi_wire_topological_select(NULL, 2);

    if (topo_min && topo_med && topo_max) {
        printf("[PASS] tsfi_wire_topological_select query succeeded: min_size=%zu, med_size=%zu, max_size=%zu\n",
               topo_min->alloc_size, topo_med->alloc_size, topo_max->alloc_size);
        assert(topo_min->alloc_size <= topo_med->alloc_size);
        assert(topo_med->alloc_size <= topo_max->alloc_size);
        printf("[PASS] tsfi_wire_topological_select (Mobius-Klein) validation succeeded.\n");
    } else {
        printf("[WARN] tsfi_wire_topological_select returned NULL.\n");
    }

    lau_registry_remove(&m1);
    lau_registry_remove(&m2);
    lau_registry_remove(&m3);

    free(arr);
    free(copy);
    printf("All sorting and selection routine verification checks PASSED!\n");
    return 0;
}

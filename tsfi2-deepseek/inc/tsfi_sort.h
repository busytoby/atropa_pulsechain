#ifndef TSFI_SORT_H
#define TSFI_SORT_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief Standard Quicksort using Median-of-Three pivot selection.
 * @param arr The array of uint64_t elements to sort.
 * @param n The number of elements in the array.
 */
void tsfi_quicksort(uint64_t *arr, size_t n);

/**
 * @brief Hybrid Quicksort that switches to Insertion Sort for subarrays below threshold (16 elements).
 * @param arr The array of uint64_t elements to sort.
 * @param n The number of elements in the array.
 */
void tsfi_quicksort_insertion_sort(uint64_t *arr, size_t n);

/**
 * @brief Hybrid Top-down Merge Sort that uses Quicksort to sort subarrays below threshold (64 elements).
 * @param arr The array of uint64_t elements to sort.
 * @param n The number of elements in the array.
 */
void tsfi_quicksort_top_merge(uint64_t *arr, size_t n);

/**
 * @brief Hybrid Bottom-up Merge Sort that uses Quicksort to sort initial runs of size 64, then merges them.
 * @param arr The array of uint64_t elements to sort.
 * @param n The number of elements in the array.
 */
void tsfi_quicksort_bottom_merge(uint64_t *arr, size_t n);

/**
 * @brief Adaptive Selection (Introselect) that beats random sub-sampling vulnerabilities
 *        and single-strategy failures by dynamically monitoring partition quality and
 *        switching between quickselect partitioning and deterministic median-of-medians.
 * @param arr The array of uint64_t elements.
 * @param n The number of elements in the array.
 * @param k The target rank (0-indexed).
 * @return The k-th smallest element.
 */
uint64_t tsfi_select(uint64_t *arr, size_t n, size_t k);

#include "lau_registry.h"

/**
 * @brief Selects the k-th smallest memory metadata block by allocation size
 *        directly from the active lau_memory registry using Min-Heap Tournament Selection.
 * @param k The target rank (0-indexed).
 * @return Pointer to the k-th smallest LauMetadata block, or NULL if out of bounds.
 */
LauMetadata* tsfi_select_memory_header(size_t k);

#endif /* TSFI_SORT_H */

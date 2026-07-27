#include "tsfi_sort.h"
#include <stdlib.h>
#include <string.h>

/* --- Utility Helper Functions --- */
static inline void swap(uint64_t *a, uint64_t *b) {
    uint64_t tmp = *a;
    *a = *b;
    *b = tmp;
}

static inline size_t median_of_three(uint64_t *arr, size_t low, size_t high) {
    size_t mid = low + (high - low) / 2;
    if (arr[low] > arr[mid]) swap(&arr[low], &arr[mid]);
    if (arr[low] > arr[high]) swap(&arr[low], &arr[high]);
    if (arr[mid] > arr[high]) swap(&arr[mid], &arr[high]);
    return mid;
}

/* --- Standard Insertion Sort --- */
static void insertion_sort(uint64_t *arr, size_t low, size_t high) {
    for (size_t i = low + 1; i <= high; i++) {
        uint64_t key = arr[i];
        size_t j = i;
        while (j > low && arr[j - 1] > key) {
            arr[j] = arr[j - 1];
            j--;
        }
        arr[j] = key;
    }
}

/* --- 1. Standard Quicksort --- */
static size_t partition(uint64_t *arr, size_t low, size_t high) {
    size_t p_idx = median_of_three(arr, low, high);
    uint64_t pivot = arr[p_idx];
    swap(&arr[p_idx], &arr[high]); /* Move pivot to end */

    size_t i = low;
    for (size_t j = low; j < high; j++) {
        if (arr[j] < pivot) {
            swap(&arr[i], &arr[j]);
            i++;
        }
    }
    swap(&arr[i], &arr[high]);
    return i;
}

static void quicksort_recurse(uint64_t *arr, size_t low, size_t high) {
    while (low < high) {
        size_t p = partition(arr, low, high);
        /* Tail recursion optimization: recurse on smaller subarray */
        if (p - low < high - p) {
            if (p > 0) quicksort_recurse(arr, low, p - 1);
            low = p + 1;
        } else {
            quicksort_recurse(arr, p + 1, high);
            if (p > 0) high = p - 1;
            else break;
        }
    }
}

void tsfi_quicksort(uint64_t *arr, size_t n) {
    if (n < 2) return;
    quicksort_recurse(arr, 0, n - 1);
}

/* --- 2. Quicksort Based Insertion Sort --- */
static void quicksort_insertion_recurse(uint64_t *arr, size_t low, size_t high) {
    while (low < high) {
        if (high - low < 16) {
            insertion_sort(arr, low, high);
            return;
        }
        size_t p = partition(arr, low, high);
        if (p - low < high - p) {
            if (p > 0) quicksort_insertion_recurse(arr, low, p - 1);
            low = p + 1;
        } else {
            quicksort_insertion_recurse(arr, p + 1, high);
            if (p > 0) high = p - 1;
            else break;
        }
    }
}

void tsfi_quicksort_insertion_sort(uint64_t *arr, size_t n) {
    if (n < 2) return;
    quicksort_insertion_recurse(arr, 0, n - 1);
}

/* --- Merge Utility for Hybrid Merge Sorts --- */
static void merge(uint64_t *arr, uint64_t *tmp, size_t low, size_t mid, size_t high) {
    size_t i = low;
    size_t j = mid + 1;
    size_t k = low;

    while (i <= mid && j <= high) {
        if (arr[i] <= arr[j]) {
            tmp[k++] = arr[i++];
        } else {
            tmp[k++] = arr[j++];
        }
    }
    while (i <= mid) tmp[k++] = arr[i++];
    while (j <= high) tmp[k++] = arr[j++];

    for (i = low; i <= high; i++) {
        arr[i] = tmp[i];
    }
}

/* --- 3. Quicksort Based Top-down Merge Sort --- */
static void top_merge_recurse(uint64_t *arr, uint64_t *tmp, size_t low, size_t high) {
    if (high - low < 64) {
        /* Base case: sort using quicksort */
        if (high > low) {
            quicksort_recurse(arr, low, high);
        }
        return;
    }
    size_t mid = low + (high - low) / 2;
    top_merge_recurse(arr, tmp, low, mid);
    top_merge_recurse(arr, tmp, mid + 1, high);
    merge(arr, tmp, low, mid, high);
}

void tsfi_quicksort_top_merge(uint64_t *arr, size_t n) {
    if (n < 2) return;
    uint64_t *tmp = (uint64_t *)malloc(n * sizeof(uint64_t));
    if (!tmp) {
        tsfi_quicksort(arr, n);
        return;
    }
    top_merge_recurse(arr, tmp, 0, n - 1);
    free(tmp);
}

/* --- 4. Quicksort Based Bottom-up Merge Sort --- */
void tsfi_quicksort_bottom_merge(uint64_t *arr, size_t n) {
    if (n < 2) return;
    
    /* Step 1: Sort initial runs of size 64 using quicksort */
    size_t run_size = 64;
    for (size_t i = 0; i < n; i += run_size) {
        size_t end = i + run_size - 1;
        if (end >= n) end = n - 1;
        quicksort_recurse(arr, i, end);
    }

    uint64_t *tmp = (uint64_t *)malloc(n * sizeof(uint64_t));
    if (!tmp) return; /* If malloc fails, at least runs are sorted */

    /* Step 2: Iteratively merge runs bottom-up */
    for (size_t width = run_size; width < n; width *= 2) {
        for (size_t i = 0; i < n; i += 2 * width) {
            size_t low = i;
            size_t mid = i + width - 1;
            size_t high = i + 2 * width - 1;

            if (mid >= n) continue;
            if (high >= n) high = n - 1;

            merge(arr, tmp, low, mid, high);
        }
    }
    free(tmp);
}



static void min_heapify(uint64_t *arr, size_t n, size_t i) {
    size_t smallest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    if (left < n && arr[left] < arr[smallest]) smallest = left;
    if (right < n && arr[right] < arr[smallest]) smallest = right;

    if (smallest != i) {
        swap(&arr[i], &arr[smallest]);
        min_heapify(arr, n, smallest);
    }
}

uint64_t tsfi_select(uint64_t *arr, size_t n, size_t k) {
    if (n == 0 || k >= n) return 0;

    /* Build min-heap: O(N) */
    for (int i = (int)(n / 2) - 1; i >= 0; i--) {
        min_heapify(arr, n, (size_t)i);
    }

    /* Extract minimum element K times: O(K log N) */
    for (size_t i = 0; i < k; i++) {
        swap(&arr[0], &arr[n - 1 - i]);
        min_heapify(arr, n - 1 - i, 0);
    }

    /* The root is the K-th smallest element */
    return arr[0];
}

/* --- LauMetadata Selection Integrations --- */
static void swap_meta(LauMetadata **a, LauMetadata **b) {
    LauMetadata *tmp = *a;
    *a = *b;
    *b = tmp;
}

static void min_heapify_meta(LauMetadata **arr, size_t n, size_t i) {
    size_t smallest = i;
    size_t left = 2 * i + 1;
    size_t right = 2 * i + 2;

    if (left < n && arr[left]->alloc_size < arr[smallest]->alloc_size) smallest = left;
    if (right < n && arr[right]->alloc_size < arr[smallest]->alloc_size) smallest = right;

    if (smallest != i) {
        swap_meta(&arr[i], &arr[smallest]);
        min_heapify_meta(arr, n, smallest);
    }
}

LauMetadata* tsfi_select_memory_header(size_t k) {
    lau_registry_lock();
    LauMetadata *head = lau_registry_get_head();
    if (!head) {
        lau_registry_unlock();
        return NULL;
    }

    /* Count items in active memory registry list */
    size_t count = 0;
    LauMetadata *curr = head;
    while (curr) {
        count++;
        curr = curr->next;
    }

    if (k >= count) {
        lau_registry_unlock();
        return NULL;
    }

    LauMetadata **temp_arr = (LauMetadata**)malloc(count * sizeof(LauMetadata*));
    if (!temp_arr) {
        lau_registry_unlock();
        return NULL;
    }

    /* Populate references array */
    curr = head;
    for (size_t i = 0; i < count; i++) {
        temp_arr[i] = curr;
        curr = curr->next;
    }

    /* Build min-heap: O(N) */
    for (int i = (int)(count / 2) - 1; i >= 0; i--) {
        min_heapify_meta(temp_arr, count, (size_t)i);
    }

    /* Extract minimum element K times to find target: O(K log N) */
    for (size_t i = 0; i < k; i++) {
        swap_meta(&temp_arr[0], &temp_arr[count - 1 - i]);
        min_heapify_meta(temp_arr, count - 1 - i, 0);
    }

    LauMetadata *result = temp_arr[0];
    free(temp_arr);
    lau_registry_unlock();
    return result;
}

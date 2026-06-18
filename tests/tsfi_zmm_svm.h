#ifndef TSFI_ZMM_SVM_H
#define TSFI_ZMM_SVM_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define PAGE_SIZE 256
#define PAGE_COUNT 256
#define MAX_CARDS 6

// Guest memory structures mapping page directories
typedef struct {
    uint8_t* pages[PAGE_COUNT];
    uint8_t page_locks[PAGE_COUNT]; // 1 = Locked, 0 = Unlocked
} CardMemoryMap;

static CardMemoryMap g_vm_memory[MAX_CARDS];

/**
 * @notice Initialize the virtual memory page directories for all 6 card cores.
 */
static inline void tsfi_zmm_svm_init() {
    for (int c = 0; c < MAX_CARDS; c++) {
        g_vm_memory[c].page_locks[c] = 0;
        for (int p = 0; p < PAGE_COUNT; p++) {
            // Allocate a physical page for each logical page index
            g_vm_memory[c].pages[p] = (uint8_t*)malloc(PAGE_SIZE);
            memset(g_vm_memory[c].pages[p], 0, PAGE_SIZE);
            g_vm_memory[c].page_locks[p] = 0;
        }
    }
}

/**
 * @notice Clean up and release allocated page memories.
 */
static inline void tsfi_zmm_svm_destroy() {
    for (int c = 0; c < MAX_CARDS; c++) {
        for (int p = 0; p < PAGE_COUNT; p++) {
            // Check if page pointer is shared to avoid double free
            int is_shared = 0;
            for (int oc = c + 1; oc < MAX_CARDS; oc++) {
                if (g_vm_memory[oc].pages[p] == g_vm_memory[c].pages[p]) {
                    is_shared = 1;
                    break;
                }
            }
            if (!is_shared && g_vm_memory[c].pages[p] != NULL) {
                free(g_vm_memory[c].pages[p]);
                g_vm_memory[c].pages[p] = NULL;
            }
        }
    }
}

/**
 * @notice Mounts a page of Card A's memory directly into Card B's page directory.
 *         Implements zero-copy memory sharing at the host thunk layer.
 */
static inline int tsfi_zmm_svm_mount(uint8_t card_a, uint8_t card_b, uint8_t page_idx) {
    if (card_a >= MAX_CARDS || card_b >= MAX_CARDS) {
        return -1; // Out of bounds
    }
    
    // Check if the target page has an active lock
    if (g_vm_memory[card_b].page_locks[page_idx]) {
        return -2; // Page is locked, cannot mount or modify
    }

    uint8_t* source_page = g_vm_memory[card_a].pages[page_idx];
    uint8_t* target_page = g_vm_memory[card_b].pages[page_idx];

    if (source_page == target_page) {
        return 0; // Already mapped
    }

    // Check if target page is shared elsewhere before freeing to prevent leak
    int is_shared = 0;
    for (int c = 0; c < MAX_CARDS; c++) {
        if (c != card_b && g_vm_memory[c].pages[page_idx] == target_page) {
            is_shared = 1;
            break;
        }
    }
    if (!is_shared && target_page != NULL) {
        free(target_page);
    }

    // Map Card B's page pointer to point directly to Card A's physical buffer
    g_vm_memory[card_b].pages[page_idx] = source_page;
    return 0;
}

/**
 * @notice Exposes read access to the guest RAM page tables for emulator execution.
 */
static inline uint8_t tsfi_zmm_svm_read_byte(uint8_t card_id, uint16_t addr) {
    uint8_t page = (addr >> 8) & 0xFF;
    uint8_t offset = addr & 0xFF;
    return g_vm_memory[card_id].pages[page][offset];
}

/**
 * @notice Exposes write access to the guest RAM page tables for emulator execution.
 */
static inline void tsfi_zmm_svm_write_byte(uint8_t card_id, uint16_t addr, uint8_t val) {
    uint8_t page = (addr >> 8) & 0xFF;
    uint8_t offset = addr & 0xFF;
    
    // Write only if page is not locked
    if (!g_vm_memory[card_id].page_locks[page]) {
        g_vm_memory[card_id].pages[page][offset] = val;
    }
}

/**
 * @notice Locks a memory page to prevent overwrites or re-mount mapping changes.
 */
static inline void tsfi_zmm_svm_lock_page(uint8_t card_id, uint8_t page_idx) {
    if (card_id < MAX_CARDS) {
        g_vm_memory[card_id].page_locks[page_idx] = 1;
    }
}

/**
 * @notice Unlocks a locked memory page.
 */
static inline void tsfi_zmm_svm_unlock_page(uint8_t card_id, uint8_t page_idx) {
    if (card_id < MAX_CARDS) {
        g_vm_memory[card_id].page_locks[page_idx] = 0;
    }
}

#endif // TSFI_ZMM_SVM_H

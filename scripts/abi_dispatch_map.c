#include "abi_dispatch_map.h"
#include <string.h>

void abi_dispatch_init(ABIDispatchMap *map) {
    if (!map) return;
    for (int i = 0; i < DISPATCH_MAP_SIZE; i++) {
        atomic_init(&map->entries[i].selector, EMPTY_SELECTOR);
        atomic_init(&map->entries[i].ip_offset, 0);
    }
}

static inline unsigned int hash_selector(uint32_t selector) {
    // Knuth's multiplicative hash
    return (selector * 2654435761U) % DISPATCH_MAP_SIZE;
}

static inline unsigned int hash_selector_2(uint32_t selector) {
    // Secondary hash determines step size. Must be coprime to table size (256).
    // Forced odd value is always coprime to 2^N.
    return (1 + ((selector * 1099273U) % 255)) | 1;
}

bool abi_dispatch_register(ABIDispatchMap *map, uint32_t selector, uintptr_t ip_offset) {
    if (!map || selector == EMPTY_SELECTOR) return false;

    unsigned int idx = hash_selector(selector);
    unsigned int step = hash_selector_2(selector);
    
    for (int i = 0; i < DISPATCH_MAP_SIZE; i++) {
        unsigned int curr_idx = (idx + i * step) % DISPATCH_MAP_SIZE;
        uint32_t expected = EMPTY_SELECTOR;

        // Try to claim the slot lock-freely if it is empty
        if (atomic_compare_exchange_strong_explicit(
                &map->entries[curr_idx].selector,
                &expected,
                selector,
                memory_order_release,
                memory_order_acquire)) {
            
            // Successfully claimed the slot, write the instruction pointer offset
            atomic_store_explicit(&map->entries[curr_idx].ip_offset, ip_offset, memory_order_release);
            return true;
        }

        // If the slot is already claimed by our selector, update/overwrite the offset
        if (expected == selector) {
            atomic_store_explicit(&map->entries[curr_idx].ip_offset, ip_offset, memory_order_release);
            return true;
        }

        // Otherwise (collision), loop to probe the next slot using step offset
    }

    return false; // Map is full
}

bool abi_dispatch_lookup(const ABIDispatchMap *map, uint32_t selector, uintptr_t *out_ip_offset) {
    if (!map || selector == EMPTY_SELECTOR || !out_ip_offset) return false;

    unsigned int idx = hash_selector(selector);
    unsigned int step = hash_selector_2(selector);
    
    for (int i = 0; i < DISPATCH_MAP_SIZE; i++) {
        unsigned int curr_idx = (idx + i * step) % DISPATCH_MAP_SIZE;
        
        // Read the selector with acquire memory ordering
        uint32_t curr_sel = atomic_load_explicit(&map->entries[curr_idx].selector, memory_order_acquire);

        if (curr_sel == EMPTY_SELECTOR) {
            return false; // Not found (reached empty slot during open addressing)
        }

        if (curr_sel == selector) {
            // Read corresponding offset safely
            *out_ip_offset = atomic_load_explicit(&map->entries[curr_idx].ip_offset, memory_order_acquire);
            return true;
        }
    }

    return false; // Probed entire map, not found
}

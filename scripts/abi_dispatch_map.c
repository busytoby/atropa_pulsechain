#include "abi_dispatch_map.h"
#include <string.h>
#include "../tsfi2-deepseek/inc/lau_memory.h"

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

bool abi_dispatch_register_member(ABIDispatchMap *map, void *lau_payload) {
    if (!map || !lau_payload) return false;

    // Resolve the reflection header
    LauWiredHeader *h = (LauWiredHeader*)((char*)lau_payload - 8192);

    // Iterate and bind all signature mapping slots
    bool all_ok = true;
    for (int i = 0; i < h->schema_count; i++) {
        uint32_t selector = (uint32_t)h->schema[i].offset;
        uintptr_t ip_offset = (uintptr_t)h->schema[i].target_fn;

        if (!abi_dispatch_register(map, selector, ip_offset)) {
            all_ok = false;
        }
    }
    return all_ok;
}

static inline uint64_t hash_agent_state(const LauWiredHeader *h, uint32_t selector, const uint64_t *args, size_t arg_count) {
    (void)h;
    uint64_t hash = 14695981039346656037ULL;
    const uint64_t prime = 1099511628211ULL;

    hash ^= selector; hash *= prime;
    for (size_t i = 0; i < arg_count; i++) {
        hash ^= args[i]; hash *= prime;
    }

    return hash;
}

bool abi_dispatch_invoke(const ABIDispatchMap *map, uint32_t selector, void *lau_payload, const uint64_t *args, size_t arg_count, uint64_t *out_val) {
    uintptr_t func_ptr = 0;
    if (!abi_dispatch_lookup(map, selector, &func_ptr)) {
        return false; // Selector not found
    }

    LauWiredHeader *h = (LauWiredHeader*)((char*)lau_payload - 8192);

    uint64_t inv_hash = hash_agent_state(h, selector, args, arg_count);

    if (h->cache_valid && h->cache_input_hash == inv_hash && h->cache_state_epoch == h->counter) {
        *out_val = h->cache_output_val;
        return true; // Helmholtz cache hit
    }

    typedef uint64_t (*LauMethod)(void*, const uint64_t*, size_t);
    uint64_t result = ((LauMethod)func_ptr)(lau_payload, args, arg_count);

    h->cache_input_hash = inv_hash;
    h->cache_output_val = result;
    h->cache_state_epoch = h->counter;
    h->cache_valid = 1;

    *out_val = result;
    return true;
}

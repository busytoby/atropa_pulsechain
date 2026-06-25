#ifndef ABI_DISPATCH_MAP_H
#define ABI_DISPATCH_MAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>

#define DISPATCH_MAP_SIZE 256
#define EMPTY_SELECTOR 0x00000000

typedef struct {
    _Atomic uint32_t selector;   // 4-byte Solidity method selector
    _Atomic uintptr_t ip_offset; // VM Instruction Pointer offset
} DispatchEntry;

typedef struct {
    DispatchEntry entries[DISPATCH_MAP_SIZE];
} ABIDispatchMap;

// Initialize the dispatch map
void abi_dispatch_init(ABIDispatchMap *map);

// Register a method selector to an instruction pointer offset (Lock-free)
bool abi_dispatch_register(ABIDispatchMap *map, uint32_t selector, uintptr_t ip_offset);

// Lookup the instruction pointer offset for a method selector (Lock-free)
bool abi_dispatch_lookup(const ABIDispatchMap *map, uint32_t selector, uintptr_t *out_ip_offset);

#endif // ABI_DISPATCH_MAP_H

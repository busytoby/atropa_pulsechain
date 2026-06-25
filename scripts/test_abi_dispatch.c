#include <stdio.h>
#include <assert.h>
#include "abi_dispatch_map.h"

int main() {
    printf("=== TESTING LOCK-FREE ABI DISPATCH MAP ===\n");
    
    ABIDispatchMap map;
    abi_dispatch_init(&map);
    
    // 1. Register selectors
    // Example Solidity selectors:
    // resolve(bytes32,bytes32) -> 0x05de9943
    // transfer(address,uint256) -> 0xa9059cbb
    // balance(address) -> 0x70a08231
    assert(abi_dispatch_register(&map, 0x05de9943, 1000));
    assert(abi_dispatch_register(&map, 0xa9059cbb, 2000));
    assert(abi_dispatch_register(&map, 0x70a08231, 3000));
    
    printf("✓ Registered selectors successfully.\n");
    
    // 2. Query selectors
    uintptr_t offset = 0;
    assert(abi_dispatch_lookup(&map, 0x05de9943, &offset));
    assert(offset == 1000);
    
    assert(abi_dispatch_lookup(&map, 0xa9059cbb, &offset));
    assert(offset == 2000);
    
    assert(abi_dispatch_lookup(&map, 0x70a08231, &offset));
    assert(offset == 3000);
    
    printf("✓ Lookups resolved to correct offsets.\n");
    
    // 3. Test non-existent selector
    assert(!abi_dispatch_lookup(&map, 0xffffffff, &offset));
    printf("✓ Non-existent selector lookup failed correctly.\n");
    
    // 4. Overwrite selector offset
    assert(abi_dispatch_register(&map, 0x05de9943, 5000));
    assert(abi_dispatch_lookup(&map, 0x05de9943, &offset));
    assert(offset == 5000);
    printf("✓ Overwriting selector offset resolved successfully.\n");
    
    printf("=== ALL LOCK-FREE ABI DISPATCH MAP TESTS PASSED ===\n");
    return 0;
}

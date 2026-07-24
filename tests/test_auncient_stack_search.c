#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

typedef enum {
    ROUTE_STACK_LOCAL,    // Executed directly on local stack memory
    ROUTE_NETWORK_REMOTE  // Executed over STANAG or DECnet network interfaces
} search_route_t;

typedef struct {
    uintptr_t stack_low;
    uintptr_t stack_high;
} stack_boundary_t;

// -------------------------------------------------------------
// Device-Local Stack Search Router
// -------------------------------------------------------------
search_route_t route_search_address(const stack_boundary_t *boundary, uintptr_t target_address) {
    // If the target address lies within the local stack boundary, route to stack-local search
    if (target_address >= boundary->stack_low && target_address < boundary->stack_high) {
        return ROUTE_STACK_LOCAL;
    }
    return ROUTE_NETWORK_REMOTE;
}

// Vectorized scan for device-local stack array
uint16_t scan_stack_array(const uint8_t *stack_array, int size, uint8_t threshold) {
    uint16_t mask = 0;
    for (int i = 0; i < size; i++) {
        if (stack_array[i] < threshold) {
            mask |= (1 << i);
        }
    }
    return mask;
}

// -------------------------------------------------------------
// Unit Tests
// -------------------------------------------------------------
int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT DEVICE-LOCAL STACK SEARCH SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Stack allocation of local database slice (device-local data)
    uint8_t local_stack_slice[8] = { 5, 25, 12, 40, 2, 18, 9, 30 };
    
    // Set boundaries matching our local stack variables
    stack_boundary_t boundary = {
        .stack_low = (uintptr_t)&local_stack_slice[0],
        .stack_high = (uintptr_t)&local_stack_slice[7] + 1
    };

    uintptr_t target_local = (uintptr_t)&local_stack_slice[2];
    uintptr_t target_remote = 0x7FFFFFFF00000000ULL; // High address outside stack

    // 1. Route local stack address
    printf("[TEST] Routing search target residing in local stack...\n");
    fflush(stdout);
    search_route_t route = route_search_address(&boundary, target_local);
    assert(route == ROUTE_STACK_LOCAL);
    printf("   ✓ Address 0x%lx correctly resolved to ROUTE_STACK_LOCAL.\n", target_local);
    fflush(stdout);

    // 2. Route remote address
    printf("[TEST] Routing search target residing outside local stack...\n");
    fflush(stdout);
    route = route_search_address(&boundary, target_remote);
    assert(route == ROUTE_NETWORK_REMOTE);
    printf("   ✓ Address 0x%lx correctly resolved to ROUTE_NETWORK_REMOTE (DECnet/STANAG path).\n", target_remote);
    fflush(stdout);

    // 3. Execute vectorized scan on device-local stack array
    printf("[TEST] Executing vectorized scan on local stack data (threshold = 15)...\n");
    fflush(stdout);
    uint16_t mask = scan_stack_array(local_stack_slice, 8, 15);
    // Expected matches (< 15):
    // index 0: 5  (bit 0)
    // index 2: 12 (bit 2)
    // index 4: 2  (bit 4)
    // index 6: 9  (bit 6)
    // Expected binary mask: 0101 0101 => 0x55
    uint16_t expected_mask = 0x55;
    printf("   Result Mask: 0x%02X | Expected Mask: 0x%02X\n", mask, expected_mask);
    fflush(stdout);
    assert(mask == expected_mask);
    printf("   ✓ Local stack scan completed successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("DEVICE-LOCAL STACK SEARCH TESTS PASSED\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

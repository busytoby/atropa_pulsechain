#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "bgp_reflections.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define REQUIRED_QUORUM_MASK 0x07
#define MAX_PEERS      4

// BGP Peered Route
typedef struct {
    uint32_t prefix_space;
    uint32_t originating_as;
} reflected_route_t;

// BGP Route Reflector State
typedef struct {
    uint32_t peer_addresses[MAX_PEERS];
    int peer_count;
    reflected_route_t reflection_table[MAX_PEERS];
    int route_count;
    uint32_t border_color;
    bool switch_residency_red;
} bgp_reflector_t;

static bool reflect_advertised_route(bgp_reflector_t *reflector,
                                      uint32_t sender_addr,
                                      uint32_t prefix_space,
                                      uint32_t originating_as,
                                      uint32_t quorum_mask) {
    printf("[REFLECTOR] Intercepted route from peer 0x%X: Prefix 0x%X, Origin AS-%u...\n",
           sender_addr, prefix_space, originating_as);
    fflush(stdout);

    // Verify consensus signatures for reflection updates
    int signatures = 0;
    for (int i = 0; i < 32; i++) {
        if ((quorum_mask >> i) & 1) {
            signatures++;
        }
    }

    if (signatures < 3) {
        printf("   [QUORUM FAIL] Insufficient signatures (%d/3) for route reflection. Toggling RED.\n", signatures);
        reflector->border_color = COLOR_RED;
        reflector->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // Add and broadcast route to all peered nodes
    if (reflector->route_count >= MAX_PEERS) {
        return false;
    }

    reflected_route_t *r = &reflector->reflection_table[reflector->route_count++];
    r->prefix_space = prefix_space;
    r->originating_as = originating_as;

    reflector->border_color = COLOR_CYAN;
    reflector->switch_residency_red = false;

    // Simulate coaxial reflective broadcast
    for (int i = 0; i < reflector->peer_count; i++) {
        if (reflector->peer_addresses[i] != sender_addr) {
            printf("   [BROADCAST] Reflected route prefix 0x%X to peer 0x%X.\n",
                   prefix_space, reflector->peer_addresses[i]);
            fflush(stdout);
        }
    }

    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP ROUTE REFLECTOR & BROADCAST SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    bgp_reflector_t reflector = {
        .peer_addresses = { 0x1111, 0x2222, 0x3333 },
        .peer_count = 3,
        .route_count = 0,
        .border_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    // 1. Peer 0x1111 advertises a route with valid quorum signatures
    printf("[TEST] Reflecting valid route advertisement...\n");
    fflush(stdout);
    bool ok = reflect_advertised_route(&reflector, 0x1111, 0x4000, 65001, REQUIRED_QUORUM_MASK);
    assert(ok == true);
    assert(reflector.switch_residency_red == false);
    assert(reflector.route_count == 1);

    // 2. Peer 0x2222 advertises a route with insufficient quorum signatures
    printf("[TEST] Rejecting unauthenticated route advertisement...\n");
    fflush(stdout);
    ok = reflect_advertised_route(&reflector, 0x2222, 0x8000, 65002, 0x01); // 1 signature only
    assert(ok == false);
    assert(reflector.switch_residency_red == true);
    assert(reflector.border_color == COLOR_RED);

    // 3. Serialize reflected routes to DAT file
    printf("[TEST] Deploying BGP reflected routes to database %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(reflector.reflection_table, sizeof(reflected_route_t), reflector.route_count, file);
    assert(written == (size_t)reflector.route_count);
    fclose(file);

    // Read back and verify database
    printf("[TEST] Reading back BGP reflected routes...\n");
    fflush(stdout);

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);

    reflected_route_t reloaded_routes[MAX_PEERS];
    size_t read_bytes = fread(reloaded_routes, sizeof(reflected_route_t), reflector.route_count, file);
    assert(read_bytes == (size_t)reflector.route_count);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(reloaded_routes[0].prefix_space == 0x4000);
    assert(reloaded_routes[0].originating_as == 65001);

    printf("   ✓ BGP reflected route database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP ROUTE REFLECTION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "bgp_security_routes.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define REQUIRED_QUORUM_MASK 0x07
#define MAX_ROUTES     4

// Authorized Prefix Ownership
typedef struct {
    uint32_t prefix_space;
    uint32_t authorized_as;
} prefix_owner_t;

// BGP Route Record
typedef struct {
    uint32_t prefix_space;
    uint32_t originating_as;
    uint32_t hop_address;
} bgp_secure_route_t;

// BGP Security Router State
typedef struct {
    prefix_owner_t registry[MAX_ROUTES];
    int registry_count;
    bgp_secure_route_t routing_table[MAX_ROUTES];
    int route_count;
    uint32_t status_color;
    bool switch_residency_red;
} bgp_security_router_t;

static bool process_secure_bgp_route(bgp_security_router_t *router,
                                     const bgp_secure_route_t *route,
                                     uint32_t quorum_mask) {
    printf("[BGP SECURITY] Evaluating route advertisement: Prefix 0x%X from AS-%u via Hop 0x%X...\n",
           route->prefix_space, route->originating_as, route->hop_address);
    fflush(stdout);

    // Verify consensus signatures
    int signatures = 0;
    for (int i = 0; i < 32; i++) {
        if ((quorum_mask >> i) & 1) {
            signatures++;
        }
    }

    if (signatures < 3) {
        printf("   [SECURITY VIOLATION] Insufficient consensus signatures (%d/3)! Toggling RED.\n", signatures);
        router->status_color = COLOR_RED;
        router->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // Hijack detection: Verify that the originating AS owns the prefix space
    bool authorized = false;
    for (int i = 0; i < router->registry_count; i++) {
        if (router->registry[i].prefix_space == route->prefix_space) {
            if (router->registry[i].authorized_as == route->originating_as) {
                authorized = true;
            }
            break;
        }
    }

    if (!authorized) {
        printf("   [HIJACK DETECTED] Unauthorized prefix advertisement! AS-%u does not own Prefix 0x%X. Blocking route. Toggling RED.\n",
               route->originating_as, route->prefix_space);
        router->status_color = COLOR_RED;
        router->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // Insert route
    if (router->route_count >= MAX_ROUTES) {
        return false;
    }
    router->routing_table[router->route_count++] = *route;
    router->status_color = COLOR_CYAN;
    router->switch_residency_red = false;

    printf("   [SECURITY PASS] Route accepted and added to table.\n");
    fflush(stdout);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP ROUTING SECURITY & HIJACK DETECTION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize Authorized Ownership Registry
    bgp_security_router_t router = {
        .registry = {
            { .prefix_space = 0x4000, .authorized_as = 65001 },
            { .prefix_space = 0x8000, .authorized_as = 65002 }
        },
        .registry_count = 2,
        .route_count = 0,
        .status_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    // 1. Authorized advertisement with valid quorum
    bgp_secure_route_t route1 = {
        .prefix_space = 0x4000,
        .originating_as = 65001,
        .hop_address = 0x1111
    };
    printf("[TEST] Processing authorized route advertisement...\n");
    fflush(stdout);
    bool ok = process_secure_bgp_route(&router, &route1, REQUIRED_QUORUM_MASK);
    assert(ok == true);
    assert(router.switch_residency_red == false);
    assert(router.route_count == 1);

    // 2. Unauthorized advertisement (AS-65003 trying to advertise AS-65002's prefix 0x8000)
    bgp_secure_route_t route2 = {
        .prefix_space = 0x8000,
        .originating_as = 65003, // Hijacker AS
        .hop_address = 0x3333
    };
    printf("[TEST] Processing unauthorized hijacking route advertisement...\n");
    fflush(stdout);
    ok = process_secure_bgp_route(&router, &route2, REQUIRED_QUORUM_MASK);
    assert(ok == false);
    assert(router.switch_residency_red == true);
    assert(router.status_color == COLOR_RED);

    // 3. Serialize secure routes to DAT file
    printf("[TEST] Deploying secure routing table to database %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(router.routing_table, sizeof(bgp_secure_route_t), router.route_count, file);
    assert(written == (size_t)router.route_count);
    fclose(file);

    // Read back and verify database
    printf("[TEST] Verifying written database records...\n");
    fflush(stdout);

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);

    bgp_secure_route_t reloaded_routes[MAX_ROUTES];
    size_t read_bytes = fread(reloaded_routes, sizeof(bgp_secure_route_t), router.route_count, file);
    assert(read_bytes == (size_t)router.route_count);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(reloaded_routes[0].prefix_space == 0x4000);
    assert(reloaded_routes[0].originating_as == 65001);
    printf("   ✓ BGP secure route database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP SECURITY & HIJACK DETECTION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

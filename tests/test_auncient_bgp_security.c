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

// Secure Coaxial Peer Address Identity Check
typedef struct {
    uint32_t peer_address;
    uint32_t auth_signature;
} peer_identity_t;

// BGP Route Record
typedef struct {
    uint32_t prefix_space;
    uint32_t originating_as;
    uint32_t hop_address;
} bgp_secure_route_t;

// BGP Secure Connection Registry
typedef struct {
    peer_identity_t authorized_peers[MAX_ROUTES];
    int peer_count;
    bgp_secure_route_t routing_table[MAX_ROUTES];
    int route_count;
    uint32_t status_color;
    bool switch_residency_red;
} bgp_security_router_t;

// Verify secure connection address negotiation using signatures
static bool negotiate_peer_connection(bgp_security_router_t *router,
                                      uint32_t peer_address,
                                      uint32_t handshake_sig,
                                      const bgp_secure_route_t *route,
                                      uint32_t quorum_mask) {
    printf("[BGP COAXIAL] Negotiating connection with Peer 0x%X (Handshake Sig: 0x%X)...\n",
           peer_address, handshake_sig);
    fflush(stdout);

    // 1. Verify consensus quorum mask
    int signatures = 0;
    for (int i = 0; i < 32; i++) {
        if ((quorum_mask >> i) & 1) {
            signatures++;
        }
    }

    if (signatures < 3) {
        printf("   [NEGOTIATION FAIL] Insufficient consensus signatures (%d/3)! Toggling RED.\n", signatures);
        router->status_color = COLOR_RED;
        router->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // 2. Cryptographic signature check (handshake signature must derive from peer address)
    bool identity_verified = false;
    for (int i = 0; i < router->peer_count; i++) {
        if (router->authorized_peers[i].peer_address == peer_address) {
            if (router->authorized_peers[i].auth_signature == handshake_sig) {
                identity_verified = true;
            }
            break;
        }
    }

    if (!identity_verified) {
        printf("   [SPOOF DETECTED] Peer 0x%X handshake signature 0x%X verification failed! Rejecting connection. Toggling RED.\n",
               peer_address, handshake_sig);
        router->status_color = COLOR_RED;
        router->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    // Insert route since connection is authenticated and cannot be spoofed
    if (router->route_count >= MAX_ROUTES) {
        return false;
    }
    router->routing_table[router->route_count++] = *route;
    router->status_color = COLOR_CYAN;
    router->switch_residency_red = false;

    printf("   [NEGOTIATION PASS] Peer 0x%X successfully negotiated. Route prefix 0x%X accepted.\n",
           peer_address, route->prefix_space);
    fflush(stdout);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP SECURE CONNECTION ADDRESS NEGOTIATION SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    // Initialize Authorized Peer Identity Registry
    bgp_security_router_t router = {
        .authorized_peers = {
            { .peer_address = 0x1111, .auth_signature = 0xABC1111 },
            { .peer_address = 0x2222, .auth_signature = 0xABC2222 }
        },
        .peer_count = 2,
        .route_count = 0,
        .status_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    bgp_secure_route_t route1 = {
        .prefix_space = 0x4000,
        .originating_as = 65001,
        .hop_address = 0x1111
    };

    // 1. Genuine peer negotiates successfully using cryptographic handshake signature
    printf("[TEST] Authenticating valid peer connection...\n");
    fflush(stdout);
    bool ok = negotiate_peer_connection(&router, 0x1111, 0xABC1111, &route1, REQUIRED_QUORUM_MASK);
    assert(ok == true);
    assert(router.switch_residency_red == false);
    assert(router.route_count == 1);

    // 2. Spoof attempt (Peer 0x3333 trying to present an invalid handshake signature)
    bgp_secure_route_t route2 = {
        .prefix_space = 0x8000,
        .originating_as = 65002,
        .hop_address = 0x3333
    };
    printf("[TEST] Rejecting spoofed peer connection...\n");
    fflush(stdout);
    ok = negotiate_peer_connection(&router, 0x3333, 0xBAD3333, &route2, REQUIRED_QUORUM_MASK);
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
    assert(reloaded_routes[0].hop_address == 0x1111);
    printf("   ✓ BGP secure route database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP SECURE ADDRESS NEGOTIATION TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

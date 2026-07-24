#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define STATE_FILE "bgp_peers.dat.bin"
#define COLOR_BLACK    0x000000
#define COLOR_RED      0x0000FF
#define COLOR_CYAN     0xFFFF
#define MAX_PEERS      4

// BGP Peering Connection State Machine
typedef enum {
    BGP_PEER_IDLE = 0,
    BGP_PEER_CONNECTING,
    BGP_PEER_ESTABLISHED,
    BGP_PEER_FAILED
} bgp_peer_state_t;

// BGP Peer Node
typedef struct {
    uint32_t peer_address;
    char hostname[32];
    bgp_peer_state_t connection_state;
    uint32_t keepalive_ticks;
} bgp_peer_t;

// BGP Connection Coordinator
typedef struct {
    bgp_peer_t peers[MAX_PEERS];
    int peer_count;
    uint32_t border_color;
    bool switch_residency_red;
} bgp_coordinator_t;

// Discovers and establishes a peered connection over the coaxial link
static bool discover_and_peer_node(bgp_coordinator_t *coord,
                                    uint32_t peer_addr,
                                    const char *hostname,
                                    uint32_t auth_token) {
    printf("[BGP PEER] Auto-discovering peer %s at 0x%X...\n", hostname, peer_addr);
    fflush(stdout);

    // Rule: Authtoken must match dynamic address checksum conventions
    if (auth_token != (peer_addr ^ 0x51E9A4B)) {
        printf("   [PEER REJECT] Auth token mismatch! Rejecting handshake request.\n");
        coord->border_color = COLOR_RED;
        coord->switch_residency_red = true;
        fflush(stdout);
        return false;
    }

    if (coord->peer_count >= MAX_PEERS) {
        return false;
    }

    bgp_peer_t *p = &coord->peers[coord->peer_count++];
    p->peer_address = peer_addr;
    strncpy(p->hostname, hostname, sizeof(p->hostname) - 1);
    p->hostname[sizeof(p->hostname) - 1] = '\0';
    p->connection_state = BGP_PEER_ESTABLISHED;
    p->keepalive_ticks = 100; // Reset keepalive ticks

    coord->border_color = COLOR_CYAN;
    coord->switch_residency_red = false;

    printf("   [PEER SUCCESS] BGP session established with %s.\n", p->hostname);
    fflush(stdout);
    return true;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT BGP PEERING & AUTO-DISCOVERY SUITE\n");
    printf("=============================================================\n");
    fflush(stdout);

    bgp_coordinator_t coord = {
        .peer_count = 0,
        .border_color = COLOR_BLACK,
        .switch_residency_red = false
    };

    // 1. Discover a valid peer with matching authtoken signature
    printf("[TEST] Connecting valid peer...\n");
    fflush(stdout);
    uint32_t peer_addr_1 = 0x8888AAAA;
    uint32_t auth_token_1 = peer_addr_1 ^ 0x51E9A4B;

    bool ok = discover_and_peer_node(&coord, peer_addr_1, "Node_Ocean", auth_token_1);
    assert(ok == true);
    assert(coord.switch_residency_red == false);
    assert(coord.peer_count == 1);
    assert(coord.peers[0].connection_state == BGP_PEER_ESTABLISHED);

    // 2. Fail to connect a peer using an invalid authtoken signature
    printf("[TEST] Connecting invalid peer...\n");
    fflush(stdout);
    uint32_t peer_addr_2 = 0x9999BBBB;
    uint32_t auth_token_2 = 0xDEADBEEF; // Wrong token

    ok = discover_and_peer_node(&coord, peer_addr_2, "Node_Rogue", auth_token_2);
    assert(ok == false);
    assert(coord.switch_residency_red == true);
    assert(coord.border_color == COLOR_RED);

    // 3. Serialize active BGP peering database to DAT file
    printf("[TEST] Deploying BGP peer registry to %s...\n", STATE_FILE);
    fflush(stdout);

    FILE *file = fopen(STATE_FILE, "wb");
    assert(file != NULL);
    size_t written = fwrite(coord.peers, sizeof(bgp_peer_t), coord.peer_count, file);
    assert(written == (size_t)coord.peer_count);
    fclose(file);

    // Read back and verify database
    printf("[TEST] Reading back BGP peer registry database...\n");
    fflush(stdout);

    file = fopen(STATE_FILE, "rb");
    assert(file != NULL);

    bgp_peer_t reloaded_peers[MAX_PEERS];
    size_t read_bytes = fread(reloaded_peers, sizeof(bgp_peer_t), coord.peer_count, file);
    assert(read_bytes == (size_t)coord.peer_count);
    fclose(file);

    // Clean up temporary DAT file
    remove(STATE_FILE);

    assert(reloaded_peers[0].peer_address == peer_addr_1);
    assert(strcmp(reloaded_peers[0].hostname, "Node_Ocean") == 0);
    assert(reloaded_peers[0].connection_state == BGP_PEER_ESTABLISHED);

    printf("   ✓ BGP peering database serialization validated successfully.\n");
    fflush(stdout);

    printf("=============================================================\n");
    printf("BGP PEERING & DISCOVERY TESTS COMPLETE\n");
    printf("=============================================================\n");
    fflush(stdout);
    return 0;
}

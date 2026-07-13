#include "../firmware/tsfi_lib/tsfi_peer_discovery.c"
#include <stdio.h>
#include <assert.h>

int main(void) {
    printf("=============================================================\n");
    printf("Auncient ZMM VM: Peer Discovery Registry C Verification Suite\n");
    printf("=============================================================\n");

    PeerDiscoveryRegistry reg;
    peer_registry_init(&reg);

    // 1. Register MCP Peer
    printf("1. Registering unified peer endpoints...\n");
    bool r1 = peer_registry_register(&reg, "peer_mcp://127.0.0.1:3000", PEER_TYPE_MCP, NULL, 3000);
    assert(r1 == true);

    // Register Thunk Peer
    bool r2 = peer_registry_register(&reg, "peer_thunk://ALSA_AUDIO", PEER_TYPE_THUNK, (void*)0x8000, 0);
    assert(r2 == true);
    printf("   ✓ Registered MCP host and ALSA thunk endpoints.\n");

    // 2. Discover MCP Peer
    printf("2. Querying peer discovery registry...\n");
    RegisteredPeer *p1 = peer_registry_discover(&reg, "peer_mcp://127.0.0.1:3000");
    assert(p1 != NULL);
    assert(p1->type == PEER_TYPE_MCP);
    assert(p1->metadata == 3000);
    printf("   ✓ Successfully discovered MCP peer. Port: %u\n", p1->metadata);

    // 3. Discover non-existent Peer
    RegisteredPeer *p2 = peer_registry_discover(&reg, "peer_mcp://invalid");
    assert(p2 == NULL);
    printf("   ✓ Verified non-existent queries return NULL.\n");

    peer_registry_destroy(&reg);

    printf("\n=============================================================\n");
    printf("AUNCIENT PEER DISCOVERY REGISTRY TESTS PASSED\n");
    printf("=============================================================\n");
    return 0;
}

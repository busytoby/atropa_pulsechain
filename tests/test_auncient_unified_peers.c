#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef enum {
    PEER_TYPE_MCP,      // External MCP Host
    PEER_TYPE_THUNK,    // Local Host JIT Thunk
    PEER_TYPE_REGISTER  // Guest Register
} PeerType;

typedef struct {
    char address[64];
    PeerType type;
    union {
        struct {
            uint32_t port;
        } mcp;
        struct {
            const char *thunk_name;
        } thunk;
        struct {
            uint8_t *reg_ptr;
        } reg;
    } target;
} UnifiedPeer;

// Global registers for simulation
static uint8_t cpu_accumulator = 0;
static uint32_t mock_network_sent_port = 0;
static const char *last_executed_thunk = NULL;

// Unified routing logic delivering payloads to any peer type
bool route_payload_to_peer(const UnifiedPeer *peer, uint32_t val) {
    switch (peer->type) {
        case PEER_TYPE_MCP:
            mock_network_sent_port = peer->target.mcp.port;
            printf("   [ROUTER] Routed payload %u to MCP Host on port %u\n", val, mock_network_sent_port);
            return true;
            
        case PEER_TYPE_THUNK:
            last_executed_thunk = peer->target.thunk.thunk_name;
            printf("   [ROUTER] Routed payload %u to Thunk: %s\n", val, last_executed_thunk);
            return true;
            
        case PEER_TYPE_REGISTER:
            *(peer->target.reg.reg_ptr) = (uint8_t)(val & 0xFF);
            printf("   [ROUTER] Routed payload %u directly to Guest Register (value set: %u)\n", 
                   val, *(peer->target.reg.reg_ptr));
            return true;
    }
    return false;
}

int main(void) {
    printf("=============================================================\n");
    printf("AUNCIENT ZMM VM: UNIFIED PEER ROUTER VERIFICATION SUITE\n");
    printf("=============================================================\n");

    // 1. Define MCP Host Peer
    UnifiedPeer mcp_peer;
    strcpy(mcp_peer.address, "peer_mcp://127.0.0.1:3000");
    mcp_peer.type = PEER_TYPE_MCP;
    mcp_peer.target.mcp.port = 3000;

    // 2. Define JIT Thunk Peer
    UnifiedPeer thunk_peer;
    strcpy(thunk_peer.address, "peer_thunk://ALSA_AUDIO");
    thunk_peer.type = PEER_TYPE_THUNK;
    thunk_peer.target.thunk.thunk_name = "ALSA_AUDIO_THUNK";

    // 3. Define Register Peer (targeting guest CPU accumulator)
    UnifiedPeer reg_peer;
    strcpy(reg_peer.address, "peer_register://cpu6502/A");
    reg_peer.type = PEER_TYPE_REGISTER;
    reg_peer.target.reg.reg_ptr = &cpu_accumulator;

    printf("1. Routing data to MCP peer...\n");
    bool r1 = route_payload_to_peer(&mcp_peer, 100);
    assert(r1 == true);
    assert(mock_network_sent_port == 3000);

    printf("2. Routing data to Thunk peer...\n");
    bool r2 = route_payload_to_peer(&thunk_peer, 120);
    assert(r2 == true);
    assert(strcmp(last_executed_thunk, "ALSA_AUDIO_THUNK") == 0);

    printf("3. Routing data to Guest Register peer...\n");
    bool r3 = route_payload_to_peer(&reg_peer, 255);
    assert(r3 == true);
    assert(cpu_accumulator == 255);

    printf("\n=============================================================\n");
    printf("AUNCIENT UNIFIED PEER ROUTING VERIFICATION PASSED\n");
    printf("=============================================================\n");
    return 0;
}

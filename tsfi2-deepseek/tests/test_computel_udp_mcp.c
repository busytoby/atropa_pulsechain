#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "../src/tsfi_computel_udp_mcp.c"

int main(void) {
    printf("[TEST] Initializing Auncient Computel MCP over UDP test suite...\n");

    UdpReceiverState rcv;
    init_udp_receiver(&rcv);

    // Frame 0
    McpUdpFrame f0;
    f0.seq = 0;
    f0.type = 'D'; // Data
    f0.len = 5;
    strcpy((char *)f0.payload, "Pack0");
    f0.checksum = compute_mcp_checksum(&f0);

    // Frame 1
    McpUdpFrame f1;
    f1.seq = 1;
    f1.type = 'D';
    f1.len = 5;
    strcpy((char *)f1.payload, "Pack1");
    f1.checksum = compute_mcp_checksum(&f1);

    // Frame 2
    McpUdpFrame f2;
    f2.seq = 2;
    f2.type = 'D';
    f2.len = 5;
    strcpy((char *)f2.payload, "Pack2");
    f2.checksum = compute_mcp_checksum(&f2);

    // Transmit Frame 0 -> Should deliver
    bool ok = simulate_udp_transmit(&rcv, &f0, false);
    assert(ok == true);
    assert(rcv.packets_received == 1);

    // Transmit Frame 1 with drop simulation -> Should drop
    ok = simulate_udp_transmit(&rcv, &f1, true);
    assert(ok == false);
    assert(rcv.packets_dropped == 1);

    // Transmit Frame 2 -> Out of order since expected_seq is 1 -> Should fail
    ok = simulate_udp_transmit(&rcv, &f2, false);
    assert(ok == false);
    assert(rcv.retransmit_requests == 2);

    // Retransmit Frame 1 -> Should deliver
    ok = simulate_udp_transmit(&rcv, &f1, false);
    assert(ok == true);
    assert(rcv.packets_received == 2);

    // Transmit Frame 2 -> Should deliver now that seq 1 is completed
    ok = simulate_udp_transmit(&rcv, &f2, false);
    assert(ok == true);
    assert(rcv.packets_received == 3);

    printf("[SUCCESS] MCP over UDP packet recovery tests completed.\n");
    return 0;
}

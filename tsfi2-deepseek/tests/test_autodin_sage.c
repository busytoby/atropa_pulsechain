#include "tsfi_autodin_sage.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main(void) {
    printf("[INFO] Starting AUTODIN SAGE Transaction Compliance Test...\n");
    
    // Test 1: Latching relay snaps
    tsfi_autodin_relay relay;
    tsfi_autodin_relay_init(&relay);
    assert(relay.contacts_closed == false);
    
    // Exert flux on Set coil
    int rc = tsfi_autodin_relay_latch(&relay, 120, 0);
    assert(rc == 0);
    assert(relay.contacts_closed == false); // Not latched yet (flux: 120 <= 200)
    
    rc = tsfi_autodin_relay_latch(&relay, 100, 0);
    assert(rc == 0);
    assert(relay.contacts_closed == true); // snapped/latched! (flux: 220 > 200)
    
    // Exert flux on Reset coil
    rc = tsfi_autodin_relay_latch(&relay, 0, 250);
    assert(rc == 0);
    assert(relay.contacts_closed == false); // Reset snaps back!
    
    // Test 2: SAGE PLL Phase-Locked synchronization loop
    tsfi_sage_pll_fabric pll;
    memset(&pll, 0, sizeof(tsfi_sage_pll_fabric));
    pll.feedback_phase = 100;
    
    // Run sync loop towards target external phase 100
    rc = tsfi_sage_pll_sync(&pll, 100);
    assert(rc == 0); // Locked! (error voltage = 0)
    
    // Test 3: CICS transaction routing checks
    tsfi_autodin_manager mgr;
    memset(&mgr, 0, sizeof(tsfi_autodin_manager));
    mgr.system_unstable = false;
    mgr.pll = pll;
    
    tsfi_sage_cics_io cics_tx;
    cics_tx.tx_id = 7001;
    cics_tx.queue_id = 5;
    
    rc = tsfi_sage_cics_io_route(&cics_tx, &mgr);
    assert(rc == 0);
    assert(mgr.active_tx_id == 7001);
    
    // Check stability rejection
    mgr.system_unstable = true;
    rc = tsfi_sage_cics_io_route(&cics_tx, &mgr);
    assert(rc == -2);
    
    // Test 4: WinchesterMQ SCSI local loopback handshake
    int sv[2];
    rc = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    assert(rc == 0);
    
    tsfi_winchester_scsi scsi;
    memset(&scsi, 0, sizeof(tsfi_winchester_scsi));
    
    // Attempt keycode 32 (D/d)
    rc = tsfi_winchester_scsi_handshake(&scsi, sv[0], 32);
    assert(rc == 1); // Handshake step 1 in progress (REQ line high, sent register payload)
    assert(scsi.req_line == true);
    
    // Simulating target node loopback echo ACK
    uint8_t read_payload = 0;
    rc = recv(sv[1], &read_payload, 1, 0);
    assert(rc == 1 && read_payload == 32);
    
    rc = send(sv[1], &read_payload, 1, 0);
    assert(rc == 1);
    
    // Complete the handshake
    rc = tsfi_winchester_scsi_handshake(&scsi, sv[0], 32);
    assert(rc == 0); // Handshake successfully complete!
    assert(scsi.ack_line == true);
    
    close(sv[0]);
    close(sv[1]);
    
    printf("[SUCCESS] AUTODIN SAGE Transaction Compliance Test Passed!\n");
    return 0;
}

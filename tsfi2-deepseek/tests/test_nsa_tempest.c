#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "tsfi_nsa_tempest.h"

int main() {
    printf("[Test] Starting NSA TEMPEST Red-Black Zone suite...\n");
    
    tsfi_tempest_chamber chamber;
    
    // Test 1: Excellent shielding
    printf("[Test] Verifying excellent TEMPEST shielding...\n");
    tsfi_tempest_init(&chamber, 90.0);
    assert(chamber.shield_status == TEMPEST_SHIELD_EXCELLENT);
    assert(tsfi_tempest_verify_shielding(&chamber) == 0);
    assert(chamber.boundary_leakage_mv < 0.001); // 90 dB attenuation should result in very low leakage
    printf("  [PASS] Excellent shielding verified.\n");
    
    // Test 2: Degraded shielding warning
    printf("[Test] Verifying degraded TEMPEST shielding...\n");
    tsfi_tempest_init(&chamber, 50.0);
    assert(chamber.shield_status == TEMPEST_SHIELD_DEGRADED);
    assert(tsfi_tempest_verify_shielding(&chamber) == 1);
    printf("  [PASS] Degraded shielding verified.\n");
    
    // Test 3: Compromised shielding lockout
    printf("[Test] Verifying compromised TEMPEST shielding lockout...\n");
    tsfi_tempest_init(&chamber, 20.0);
    assert(chamber.shield_status == TEMPEST_SHIELD_COMPROMISED);
    assert(tsfi_tempest_verify_shielding(&chamber) == 2);
    
    const uint8_t red_tx[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88 };
    uint8_t black_storage[8];
    size_t black_len = 0;
    
    // Routing should fail under compromised shielding
    assert(tsfi_tempest_route_transaction(&chamber, red_tx, 8, black_storage, &black_len) == -2);
    printf("  [PASS] Compromised shielding lockout verified.\n");
    
    // Test 4: Red-Black data translation recovery
    printf("[Test] Verifying Red-to-Black and Black-to-Red packet routing recovery...\n");
    tsfi_tempest_init(&chamber, 85.0);
    assert(tsfi_tempest_route_transaction(&chamber, red_tx, 8, black_storage, &black_len) == 0);
    assert(black_len == 8);
    assert(memcmp(red_tx, black_storage, 8) != 0); // Must be scrambled
    
    uint8_t recovered_red[8];
    size_t red_len = 0;
    assert(tsfi_tempest_route_storage(&chamber, black_storage, 8, recovered_red, &red_len) == 0);
    assert(red_len == 8);
    assert(memcmp(red_tx, recovered_red, 8) == 0); // Must be recovered perfectly
    
    printf("  [PASS] Red-Black data translation recovery verified.\n");
    
    // Test 5: SCSI loopback bridge handshake
    printf("[Test] Verifying SCSI loopback bridge handshake...\n");
    tsfi_scsi_loopback_bridge bridge;
    memset(&bridge, 0, sizeof(bridge));
    
    tsfi_tempest_init(&chamber, 85.0);
    bridge.keycode_reg = 30; // keycode for 'a'
    
    // Cycle through states 0 to 4
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == 0); // 0 -> REQ (1)
    assert(bridge.status_reg == 1);
    
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == 0); // 1 -> ACK (2)
    assert(bridge.status_reg == 2);
    
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == 0); // 2 -> DATA_IN (3)
    assert(bridge.status_reg == 3);
    
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == 0); // 3 -> DATA_OUT (4), copies keycode
    assert(bridge.status_reg == 4);
    assert(bridge.data_reg == 30);
    
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == 0); // 4 -> Idle (0), increments loopback_cnt
    assert(bridge.status_reg == 0);
    assert(bridge.loopback_cnt == 1);
    
    // Test 5b: Anti-automation block
    bridge.synthetic_flag = 1;
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == -3);
    bridge.synthetic_flag = 0;
    
    // Test 5c: Invalid keycode rejection
    bridge.keycode_reg = 99;
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == -4);
    
    // Test 5d: Compromised shielding lockout
    bridge.keycode_reg = 32; // 'd'
    tsfi_tempest_init(&chamber, 20.0);
    assert(tsfi_tempest_scsi_bridge_handshake(&chamber, &bridge) == -2);
    
    printf("  [PASS] SCSI loopback bridge handshake verified.\n");
    printf("[SUCCESS] All NSA TEMPEST Red-Black zone isolation tests completed successfully!\n");
    return 0;
}

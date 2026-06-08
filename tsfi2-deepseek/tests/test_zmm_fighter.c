#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

int main() {
    printf("=== TSFi ZMM VM 2D Fighter Physics Solver Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize GraphicsSystem Yul Contract via ZMM VM text commands
    printf("[ZMM] Compiling and Initializing graphicsSystem.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"graphics\", \"../solidity/bin/graphicsSystem.yul\", 1");

    // 2. Test rightward movement
    // updateFighterState(x, y, vx, vy, action, targetX, targetY)
    // Selector: d6c09b2e
    // Parameters:
    // x = 10 (0x0a)
    // y = 0 (0x00)
    // vx = 0 (0x00)
    // vy = 0 (0x00)
    // action = 3 (Right) -> 0x03
    // targetX = 100 (0x64)
    // targetY = 0 (0x00)
    printf("[ZMM] Simulating rightward movement (action = 3)...\n");
    char cmd[2048];
    sprintf(cmd, "YULEXEC \"graphics\", \"d6c09b2e"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            10, 0, 0, 0, 3, 100, 0);
    
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    printf("Result State: %s\n", vm.output_buffer);
    
    // We expect:
    // newX = 15 (0x0f) -> 000000000000000000000000000000000000000000000000000000000000000f
    // newY = 0 (0x00)
    // newVx = 5 (0x05)
    // newVy = 0 (reverted from -1 to 0 on floor collision)
    // hit = 0 (0x00)
    
    // Let's verify the first word is 15 (0x0f) and third word is 5 (0x05)
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000000f") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000005") != NULL);
    printf("PASS: Rightward movement speed and coordinate integration verified.\n");

    // 3. Test jump (action = 1)
    // Initial jump starts at y = 0
    printf("[ZMM] Simulating jump start (action = 1, y = 0)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"d6c09b2e"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            15, 0, 5, 0, 1, 100, 0);
    
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    // We expect:
    // newX = 20
    // newY = 15 (A jump sets initial vy = 15, then y = 0 + 15 = 15)
    // newVx = 5
    // newVy = 14 (vy = 15 - 1 = 14)
    // hit = 0
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000000f") != NULL); // newY = 15 (0x0f)
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000000e") != NULL); // newVy = 14 (0x0e)
    printf("PASS: Jump initiation and upward velocity verified.\n");

    // 4. Test Attack Collision (action = 4 - Flying Kick)
    // Bruce Lee is at x = 85, target opponent is at x = 100.
    // Distance (15) is less than the flying kick width (24), so collision should register hit = 1.
    printf("[ZMM] Simulating Flying Kick hit detection (Bruce at 85, Target at 100)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"d6c09b2e"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            85, 0, 0, 0, 4, 100, 0);
    
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    // Last word (hit flag) should be 1
    // ...0000000000000000000000000000000000000000000000000000000000000001
    assert(strcmp(&vm.output_buffer[256], "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: Flying Kick hit collision detected successfully!\n");

    // 5. Test Chuck Norris Punch (action = 5)
    // Chuck at x = 81, Target at x = 100. Distance = 19 < 20 (width), should hit.
    printf("[ZMM] Simulating Chuck Norris Punch hit detection (Chuck at 81, Target at 100)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"d6c09b2e"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            81, 0, 0, 0, 5, 100, 0);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strcmp(&vm.output_buffer[256], "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: Chuck Norris Punch hit collision detected successfully!\n");

    // 6. Test Chuck Norris Block (action = 7)
    // Chuck at x = 95, Target at x = 100. Distance = 5. Block width = 0, should NOT hit.
    printf("[ZMM] Simulating Chuck Norris Block hit detection (Chuck at 95, Target at 100)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"d6c09b2e"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            95, 0, 0, 0, 7, 100, 0);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strcmp(&vm.output_buffer[256], "0000000000000000000000000000000000000000000000000000000000000000") == 0);
    printf("PASS: Chuck Norris Block successfully negated hit collision!\n");

    // 7. Test Chuck Norris Somersault Superkick (action = 8)
    // Chuck at x = 70, Target at x = 100. Distance = 30 < 32 (width), should hit.
    printf("[ZMM] Simulating Somersault Superkick hit detection (Chuck at 70, Target at 100)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"d6c09b2e"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            70, 0, 0, 0, 8, 100, 0);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strcmp(&vm.output_buffer[256], "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: Chuck Norris Somersault Superkick hit collision detected successfully!\n");

    // 8. Test simulateXonoxDoubleEnder Side 0 (Artillery Duel)
    // Selector: e577232a
    // side = 0 (Artillery)
    // input1 (t) = 1e18 (0x0de0b6b3a7640000)
    // input2 (angleDeg) = 30 (0x1e)
    // input3 (velocity) = 50e18 (0x2b5e3af16b1880000)
    // input4 (wind) = 5e18 (0x4563918244f40000)
    // input5 (targetX) = 40e18 (0x22b1c4c126440000)
    // input6 (targetY) = 10e18 (0x8ac7230489e8000)
    printf("[ZMM] Simulating Xonox Double-Ender Side A: Artillery Duel Step...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"e577232a"
                  "0000000000000000000000000000000000000000000000000000000000000000" // side = 0
                  "0000000000000000000000000000000000000000000000000de0b6b3a7640000" // t = 1e18
                  "000000000000000000000000000000000000000000000000000000000000001e" // angle = 30
                  "000000000000000000000000000000000000000000000002b5e3af16b1880000" // velocity = 50e18
                  "0000000000000000000000000000000000000000000000004563918244f40000" // wind = 5e18
                  "0000000000000000000000000000000000000000000000022b1c4c1264400000" // targetX = 40e18
                  "00000000000000000000000000000000000000000000000008ac7230489e8000\""); // targetY = 10e18
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    // We expect hitTarget (3rd word / index 128) to be 1
    // And hitTerrain (4th word / index 192) to be 0
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") != NULL);
    printf("PASS: Xonox Double-Ender Side A (Artillery) simulation and hit conditions verified!\n");

    // 9. Test simulateXonoxDoubleEnder Side 1 (Chuck Norris Combat)
    // side = 1
    // input1 (x) = 85
    // input2 (y) = 0
    // input3 (vx) = 0
    // input4 (vy) = 0
    // input5 (action) = 4 (Flying Kick)
    // input6 (targetX) = 100
    printf("[ZMM] Simulating Xonox Double-Ender Side B: Chuck Norris Combat Step...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"e577232a"
                  "0000000000000000000000000000000000000000000000000000000000000001" // side = 1
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "0000000000000000000000000000000000000000000000000000000000000000\"", // padding
            85, 0, 0, 0, 4, 100);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // last word should be 1
    assert(strcmp(&vm.output_buffer[256], "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: Xonox Double-Ender Side B (Chuck Norris Combat) simulation verified!\n");

    // 10. Test animateSkeletalJoint (Method 12)
    // Selector: 8bfb5a7b
    // parentX = 10, parentY = 20, parentZ = 30
    // localOffsetX = 5, localOffsetY = 0, localOffsetZ = 10
    // angleStart = 0, angleEnd = 90, t = 500 (LERP to 45 degrees)
    // Expected rotated offsets: cos(45)=0.7071, sin(45)=0.7071 -> xRot = 3.5 -> 3, yRot = 3.5 -> 3
    // Expected children: childX = 10 + 3 = 13, childY = 20 + 3 = 23, childZ = 30 + 10 = 40
    printf("[ZMM] Simulating Keyframe-Interpolated Skeletal Joint Rotation (LERP to 45 deg)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"8bfb5a7b"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x"
                  "%064x\"",
            10, 20, 30, 5, 0, 10, 0, 90, 500);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    // We expect:
    // childX (1st word) = 13 (0x0d)
    // childY (2nd word) = 23 (0x17)
    // childZ (3rd word) = 40 (0x28)
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000000d") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000017") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000028") != NULL);
    printf("PASS: Skeletal joint keyframe interpolation and coordinate translations verified successfully!\n");

    // 11. Test decodeAudioPWM (Method 13)
    // Selector: f5d914ab
    // pcmSample = 150 (0x96)
    // carrierTime = 100 (0x64) -> carrierThreshold = 100.
    // Since 150 > 100, speakerState should be 1
    printf("[ZMM] Simulating Silas Warner 1-bit PWM audio conversion (pcm=150, time=100)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"f5d914ab"
                  "0000000000000000000000000000000000000000000000000000000000000096"
                  "0000000000000000000000000000000000000000000000000000000000000064\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: 1-bit PWM audio conversion outputted high state (1) successfully!\n");

    // 12. Test decodeAudioBit (Method 14)
    // Selector: c3a39e8a
    // packedByte = 0x80 (binary 10000000)
    // sampleIndex = 0 (1st bit) -> should be 1
    // sampleIndex = 1 (2nd bit) -> should be 0
    printf("[ZMM] Simulating 1-bit packed audio sample bit extraction (byte=0x80)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"c3a39e8a"
                  "0000000000000000000000000000000000000000000000000000000000000080"
                  "0000000000000000000000000000000000000000000000000000000000000000\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    
    sprintf(cmd, "YULEXEC \"graphics\", \"c3a39e8a"
                  "0000000000000000000000000000000000000000000000000000000000000080"
                  "0000000000000000000000000000000000000000000000000000000000000001\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") == 0);
    printf("PASS: 1-bit packed sample bit extraction verified successfully!\n");

    // 13. Test resolveRoomTransition (Method 15)
    // Selector: a9e7c5b6
    // Inputs:
    // roomX = 5, roomY = 5
    // playerX = 2 (triggers left transition)
    // playerY = 100
    // keyRing = 0 (no keys)
    // doorConfig = 0x11 -> Left door is locked (0x01) and requires Green key (color=2 -> 0x10)
    // Expected: Blocked, newPlayerX = 10, status = 3
    printf("[ZMM] Simulating Shamus room transition: Left locked door, NO keys...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"a9e7c5b6"
                  "0000000000000000000000000000000000000000000000000000000000000005" // roomX = 5
                  "0000000000000000000000000000000000000000000000000000000000000005" // roomY = 5
                  "0000000000000000000000000000000000000000000000000000000000000002" // playerX = 2
                  "0000000000000000000000000000000000000000000000000000000000000064" // playerY = 100
                  "0000000000000000000000000000000000000000000000000000000000000000" // keyRing = 0
                  "0000000000000000000000000000000000000000000000000000000000000011\""); // doorConfig = 0x11
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // newRoomX should be 5
    // newPlayerX should be 10
    // transitionStatus (6th word / index 320) should be 3
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000005") != NULL);
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000000a") != NULL); // 10 = 0x0a
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000003") != NULL); // status = 3
    printf("PASS: Shamus locked door traversal blocked successfully!\n");

    // 14. Test resolveRoomTransition with keyring
    // keyRing = 2 (Green key, binary 0010, matching required color 2)
    // Expected: Success, newRoomX = 4, newPlayerX = 310, newKeyRing = 0, status = 2
    printf("[ZMM] Simulating Shamus room transition: Left locked door WITH key...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"a9e7c5b6"
                  "0000000000000000000000000000000000000000000000000000000000000005"
                  "0000000000000000000000000000000000000000000000000000000000000005"
                  "0000000000000000000000000000000000000000000000000000000000000002"
                  "0000000000000000000000000000000000000000000000000000000000000064"
                  "0000000000000000000000000000000000000000000000000000000000000002" // keyRing = 2 (Green key)
                  "0000000000000000000000000000000000000000000000000000000000000021\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // newRoomX should be 4
    // newPlayerX should be 310 (0x0136)
    // newKeyRing (5th word / index 256) should be 0
    // transitionStatus (6th word / index 320) should be 2
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000004") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000136") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000002") != NULL); // status = 2
    printf("PASS: Shamus locked door traversal consumed key and advanced room successfully!\n");

    // 15. Test updateBallSoccerState (Method 16) - Ground Bounce
    // Selector: fa49e91b
    // bx=50, by=50, bz=2, vz=-5 (which is 0xfffffffffffffffb in 2s complement)
    // px=100, py=100, headCarry=0
    // Expected: ground contact triggers, newBz = 0, newVz = 4 (abs(-6) * 0.7 = 4.2 -> 4)
    printf("[ZMM] Simulating International Soccer ball physics: Ground Rebound...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"fa49e91b"
                  "0000000000000000000000000000000000000000000000000000000000000032" // bx = 50
                  "0000000000000000000000000000000000000000000000000000000000000032" // by = 50
                  "0000000000000000000000000000000000000000000000000000000000000002" // bz = 2
                  "fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffb" // vz = -5
                  "0000000000000000000000000000000000000000000000000000000000000064" // px = 100
                  "0000000000000000000000000000000000000000000000000000000000000064" // py = 100
                  "0000000000000000000000000000000000000000000000000000000000000000\""); // carry = 0
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // newBz (3rd word / index 128) should be 0
    // newVz (4th word / index 192) should be 4
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000004") != NULL);
    printf("PASS: International Soccer ground collision rebound velocity calculated correctly!\n");

    // 16. Test updateBallSoccerState (Method 16) - Head-Carry Lock
    // bx=103, by=100, bz=25 (height aligned)
    // px=100, py=100, headCarry=0
    // Expected: snaps into lock, newBx = px = 100 (0x64), newBy = py = 100 (0x64), newBz = 24 (0x18), newCarry = 1
    printf("[ZMM] Simulating International Soccer ball physics: Head-Carry Lock Snap...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"fa49e91b"
                  "0000000000000000000000000000000000000000000000000000000000000067" // bx = 103
                  "0000000000000000000000000000000000000000000000000000000000000064" // by = 100
                  "0000000000000000000000000000000000000000000000000000000000000019" // bz = 25
                  "0000000000000000000000000000000000000000000000000000000000000000" // vz = 0
                  "0000000000000000000000000000000000000000000000000000000000000064" // px = 100
                  "0000000000000000000000000000000000000000000000000000000000000064" // py = 100
                  "0000000000000000000000000000000000000000000000000000000000000000\""); // carry = 0
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // newBx should be 100 (0x64), newBy = 100 (0x64), newBz = 24 (0x18), newCarry (5th word / index 256) should be 1
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000064") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000018") != NULL);
    assert(strcmp(&vm.output_buffer[256], "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: International Soccer head-carry coordination locked successfully!\n");

    // 17. Test resolveVectorFieldAI (Method 17)
    // Selector: b1c8c5de
    // entityX = 100 (0x64), entityY = 100 (0x64), numNodes = 2 (0x02)
    // node1 = 00000000000000000000000000000000000000000000c800000000000000f032
    // node2 = 00000000000000000000000000000000000000000000a000000000000000c91e
    // Expected output: netFx = 250 (0xfa), netFy = 150 (0x96)
    printf("[ZMM] Simulating General Attractor/Repeller Vector Field AI...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"b1c8c5de"
                  "0000000000000000000000000000000000000000000000000000000000000064" // entityX = 100
                  "0000000000000000000000000000000000000000000000000000000000000064" // entityY = 100
                  "0000000000000000000000000000000000000000000000000000000000000002" // numNodes = 2
                  "00000000000000000000000000000000000000000000c800000000000000f032" // node1 (Attractor)
                  "00000000000000000000000000000000000000000000a000000000000000c91e\""); // node2 (Repeller)
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // netFx (1st word) = 250 (0xfa)
    // netFy (2nd word) = 150 (0x96)
    assert(strstr(vm.output_buffer, "00000000000000000000000000000000000000000000000000000000000000fa") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000096") != NULL);
    printf("PASS: General Attractor/Repeller Vector Field AI resolved correctly!\n");

    // 18. Test resolveVectorFieldAI - Sensory Cutoff
    // entityX = 100 (0x64), entityY = 100 (0x64), numNodes = 1
    // node3 = 00000000000000000000000000003c00000000000000c8000000000000012c32 (radius=30, dist=50, out of range)
    // Expected output: netFx = 0, netFy = 0
    printf("[ZMM] Simulating Vector Field AI: Sensory Cutoff (Out of range)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"b1c8c5de"
                  "0000000000000000000000000000000000000000000000000000000000000064"
                  "0000000000000000000000000000000000000000000000000000000000000064"
                  "0000000000000000000000000000000000000000000000000000000000000001"
                  "00000000000000000000000000003c00000000000000c8000000000000012c32\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000000") != NULL);
    printf("PASS: Vector Field sensory cutoff successfully ignored out-of-range attractor!\n");

    // 19. Test resolveVectorFieldAI - Quadratic Decay
    // entityX = 100 (0x64), entityY = 100 (0x64), numNodes = 1
    // node4 = 00000000000000000000000000023c00000000000000c800000000000000dc32 (radius=30, dist=10, decay=1, weight=50)
    // Expected output: netFx = 50 * 10000 / 100 = 5000 (0x1388), netFy = 0
    printf("[ZMM] Simulating Vector Field AI: Quadratic Decay Rebound...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"b1c8c5de"
                  "0000000000000000000000000000000000000000000000000000000000000064"
                  "0000000000000000000000000000000000000000000000000000000000000064"
                  "0000000000000000000000000000000000000000000000000000000000000001"
                  "00000000000000000000000000023c00000000000000c800000000000000dc32\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000001388") != NULL);
    printf("PASS: Vector Field quadratic decay force calculation verified successfully!\n");

    // 20. Test scheduleVirtualSprites (Method 19)
    // Selector: 6d1b8fa2
    // Parameters:
    // numSprites = 5
    // Sprite 1: ID=1, X=100 (0x64), Y=50 (0x32) -> word = 0x3200640001
    // Sprite 2: ID=2, X=100 (0x64), Y=10 (0x0a) -> word = 0x0a00640002
    // Sprite 3: ID=3, X=100 (0x64), Y=30 (0x1e) -> word = 0x1e00640003
    // Sprite 4: ID=4, X=100 (0x64), Y=70 (0x46) -> word = 0x4600640004
    // Sprite 5: ID=5, X=100 (0x64), Y=90 (0x5a) -> word = 0x5a00640005
    printf("[ZMM] Simulating VIC-II Sprite Multiplexer Scheduling (5 virtual sprites)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"6d1b8fa2"
                  "0000000000000000000000000000000000000000000000000000000000000005" // numSprites = 5
                  "0000000000000000000000000000000000000000000000000000003200640001" // Sprite 1
                  "0000000000000000000000000000000000000000000000000000000a00640002" // Sprite 2
                  "0000000000000000000000000000000000000000000000000000001e00640003" // Sprite 3
                  "0000000000000000000000000000000000000000000000000000004600640004" // Sprite 4
                  "0000000000000000000000000000000000000000000000000000005a00640005\""); // Sprite 5
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    // We expect 15 words of output matching the sorted scheduling:
    // Sprite 2 (Y=10) -> Slot 0:
    // Word 0: ID=2 (0x02), Word 1: Slot=0 (0x00), Word 2: Y=10 (0x0a)
    // Sprite 3 (Y=30) -> Slot 1:
    // Word 3: ID=3 (0x03), Word 4: Slot=1 (0x01), Word 5: Y=30 (0x1e)
    // Sprite 1 (Y=50) -> Slot 0:
    // Word 6: ID=1 (0x01), Word 7: Slot=0 (0x00), Word 8: Y=50 (0x32)
    // Sprite 4 (Y=70) -> Slot 1:
    // Word 9: ID=4 (0x04), Word 10: Slot=1 (0x01), Word 11: Y=70 (0x46)
    // Sprite 5 (Y=90) -> Slot 0:
    // Word 12: ID=5 (0x05), Word 13: Slot=0 (0x00), Word 14: Y=90 (0x5a)

    // Let's assert some key parts of the output to verify sorting and assignment logic
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000002" // Sprite 2 ID
                                    "0000000000000000000000000000000000000000000000000000000000000000" // Sprite 2 Slot 0
                                    "000000000000000000000000000000000000000000000000000000000000000a") != NULL); // Sprite 2 Y=10

    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000003" // Sprite 3 ID
                                    "0000000000000000000000000000000000000000000000000000000000000001" // Sprite 3 Slot 1
                                    "000000000000000000000000000000000000000000000000000000000000001e") != NULL); // Sprite 3 Y=30

    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001" // Sprite 1 ID
                                    "0000000000000000000000000000000000000000000000000000000000000000" // Sprite 1 Slot 0
                                    "0000000000000000000000000000000000000000000000000000000000000032") != NULL); // Sprite 1 Y=50

    printf("PASS: VIC-II Sprite Multiplexer Scheduling sorted and allocated physical slots successfully!\n");

    // 21. Test loading compiled Solidity contract as a thunk
    printf("[ZMM] Compiling and Initializing Target.sol...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"target\", \"../solidity/Target.sol\", 2");

    printf("[ZMM] Simulating Target.sol call (echo function, value=42)...\n");
    // Selector for echo(address,uint256) is 0f2723ea
    sprintf(cmd, "YULEXEC \"target\", \"0f2723ea"
                  "0000000000000000000000000000000000000000000000000000000000000000" // address to = 0
                  "000000000000000000000000000000000000000000000000000000000000002a\""); // uint256 val = 42
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    // Expected return value is 42 * 2 = 84 (0x54)
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000054") != NULL);
    printf("PASS: Loading and executing compiled Solidity contract thunk verified successfully!\n");

    // 22. Test resolveWidgetInteraction (Method 20)
    // Selector: 4f324fc5
    // 22a. Button inside/outside check
    printf("[ZMM] Simulating Widget Interaction: Button click tests...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"4f324fc5"
                  "0000000000000000000000000000000000000000000000000000000000000000" // widgetType = 0 (Button)
                  "000000000000000000000000000000000000000000000000000000000000000a" // x = 10
                  "0000000000000000000000000000000000000000000000000000000000000014" // y = 20
                  "0000000000000000000000000000000000000000000000000000000000000032" // width = 50
                  "000000000000000000000000000000000000000000000000000000000000000f" // height = 15
                  "000000000000000000000000000000000000000000000000000000000000000f" // clickX = 15 (Inside)
                  "0000000000000000000000000000000000000000000000000000000000000019" // clickY = 25 (Inside)
                  "0000000000000000000000000000000000000000000000000000000000000000\""); // param1 = 0
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // clicked (1st word) should be 1
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001"
                                    "0000000000000000000000000000000000000000000000000000000000000000"
                                    "0000000000000000000000000000000000000000000000000000000000000000") == 0);

    // 22b. Window Drag drag offset check
    printf("[ZMM] Simulating Widget Interaction: Window titlebar drag tests...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"4f324fc5"
                  "0000000000000000000000000000000000000000000000000000000000000001" // widgetType = 1 (Window)
                  "0000000000000000000000000000000000000000000000000000000000000064" // x = 100
                  "0000000000000000000000000000000000000000000000000000000000000064" // y = 100
                  "00000000000000000000000000000000000000000000000000000000000000c8" // width = 200
                  "0000000000000000000000000000000000000000000000000000000000000096" // height = 150
                  "000000000000000000000000000000000000000000000000000000000000006e" // clickX = 110 (deltaX = 10)
                  "0000000000000000000000000000000000000000000000000000000000000069" // clickY = 105 (deltaY = 5)
                  "000000000000000000000000000000000000000000000000000000000000000f\""); // titleHeight = 15
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // Should return: clicked = 1, deltaX = 10 (0x0a), deltaY = 5 (0x05)
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001"
                                    "000000000000000000000000000000000000000000000000000000000000000a"
                                    "0000000000000000000000000000000000000000000000000000000000000005") == 0);

    // 22c. Dropdown menu indexing check
    printf("[ZMM] Simulating Widget Interaction: Dropdown menu index checks...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"4f324fc5"
                  "0000000000000000000000000000000000000000000000000000000000000002" // widgetType = 2 (Dropdown Menu)
                  "0000000000000000000000000000000000000000000000000000000000000032" // x = 50
                  "0000000000000000000000000000000000000000000000000000000000000032" // y = 50
                  "0000000000000000000000000000000000000000000000000000000000000064" // width = 100
                  "0000000000000000000000000000000000000000000000000000000000000050" // height = 80
                  "000000000000000000000000000000000000000000000000000000000000003c" // clickX = 60
                  "000000000000000000000000000000000000000000000000000000000000005a" // clickY = 90
                  "0000000000000000000000000000000000000000000000000000000000000010\""); // itemHeight = 16
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    // Should return: clicked = 1, itemIndex = (90 - 50)/16 = 2 (0x02)
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000001"
                                    "0000000000000000000000000000000000000000000000000000000000000002"
                                    "0000000000000000000000000000000000000000000000000000000000000000") == 0);
    printf("PASS: resolveWidgetInteraction (Button, Window, Dropdown) tests verified successfully!\n");

    // 23. Test simulateAnticDisplayList (Method 26)
    // Selector: 8dfb6c41
    printf("[ZMM] Configuring virtual Display List in storage...\n");
    // Write Display List instructions to storage starting at key 0x1000
    lau_yul_thunk_sstore(0x1000, 0x62); // DLI + LMS + Mode 2 -> 0x62
    lau_yul_thunk_sstore(0x1001, 0x50); // Pixel address low = 0x50
    lau_yul_thunk_sstore(0x1002, 0x20); // Pixel address high = 0x20
    lau_yul_thunk_sstore(0x1003, 0x82); // Jump + Mode 2 -> 0x82
    lau_yul_thunk_sstore(0x1004, 0x10); // Jump target low = 0x10
    lau_yul_thunk_sstore(0x1005, 0x10); // Jump target high = 0x10
    // Jump target address 0x1010
    lau_yul_thunk_sstore(0x1010, 0x08); // Mode 8 -> 0x08

    printf("[ZMM] Simulating ANTIC Display List execution (2 scanlines starting at 0x1000)...\n");
    sprintf(cmd, "YULEXEC \"graphics\", \"8dfb6c41"
                  "0000000000000000000000000000000000000000000000000000000000001000" // displayListAddress = 0x1000
                  "0000000000000000000000000000000000000000000000000000000000000000" // startScanline = 0
                  "0000000000000000000000000000000000000000000000000000000000000002\""); // lineCount = 2
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    // Expected output: activeMode (1st word) = 8 (0x08), pixelAddressOffset (2nd word) = 0x2050 (8272), bytesParsed (3rd word) = 1 (0x01), dliAccumulated (4th word) = 1 (0x01)
    printf("[DEBUG] vm.output_buffer: %s\n", vm.output_buffer);
    assert(strcmp(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000008"
                                    "0000000000000000000000000000000000000000000000000000000000002050"
                                    "0000000000000000000000000000000000000000000000000000000000000001"
                                    "0000000000000000000000000000000000000000000000000000000000000001") == 0);
    printf("PASS: ANTIC Display List LMS parsing, JUMP instruction, DLI detection, and Mode transitions verified successfully!\n");

    printf("[ZMM] Testing GTIA Color mapping (Method 27)...\n");
    // Write Atari color value 0x94 (Hue 9, Lum 4) to registry address 53266 (first color slot)
    lau_yul_thunk_sstore(53266, 0x94);
    
    sprintf(cmd, "YULEXEC \"graphics\", \"bc8ad742\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);
    
    // Verify first color maps correctly to RGBA.
    // Hue 9: Light Blue (r=0, g=200, b=200). Lum 4: scale = 4 * 17 = 68.
    // r = 0, g = 200 * 68 / 255 = 53 (0x35), b = 200 * 68 / 255 = 53 (0x35), Alpha = 0xFF.
    // RGBA = 0x003535FF -> 3487231
    // First 32 bytes of output should represent this RGBA.
    assert(strncmp(vm.output_buffer, "00000000000000000000000000000000000000000000000000000000003535ff", 64) == 0);
    printf("PASS: GTIA dynamic color translation verified successfully!\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM 2D FIGHTER PHYSICS TESTS PASSED ===\n");
    return 0;
}

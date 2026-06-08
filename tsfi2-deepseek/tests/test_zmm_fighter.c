#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

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

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM 2D FIGHTER PHYSICS TESTS PASSED ===\n");
    return 0;
}

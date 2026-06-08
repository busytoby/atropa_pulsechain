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
    char cmd[512];
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

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM 2D FIGHTER PHYSICS TESTS PASSED ===\n");
    return 0;
}

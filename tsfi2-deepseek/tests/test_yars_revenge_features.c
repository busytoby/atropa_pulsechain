#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Helper to unpack a uint64_t from a 32-byte hex chunk
uint64_t unpack_word(const char *buf, int word_idx) {
    char temp[65] = {0};
    memcpy(temp, &buf[word_idx * 64], 64);
    temp[64] = '\0';
    uint64_t val = 0;
    // Read the last 16 characters (64-bit value)
    sscanf(&temp[48], "%lx", &val);
    return val;
}

int main() {
    printf("=== TSFi ZMM VM Yars' Revenge Feature & Victory Validation Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize the Yul Contract
    printf("[TEST] Compiling and registering yars_revenge.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"yars_revenge\", \"../solidity/bin/yars_revenge.yul\", 1");

    // 2. Initialize Game State
    printf("[TEST] Calling initializeGame()...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"yars_revenge\", \"d6b046a2\"");

    // Re-verify output
    assert(unpack_word(vm.output_buffer, 0) == 1); // returns success

    // 3. Frame Tick Loop to Move Yar into the Shield and Eat It
    printf("[TEST] Simulating Yar flight towards the shield to eat blocks...\n");
    int eaten = 0;
    int cannon_primed = 0;
    int lives = 0;
    int score = 0;
    int status = 0;

    // Yar starts at X = 100 (10000). Shield is at X = 600 (60000).
    // Yar moves with tx = 1 (speed +400 per frame). 
    // It should take about 125 frames to reach the shield.
    for (int frame = 0; frame < 150; frame++) {
        int ty = 0;
        if (frame > 0) {
            uint64_t yarY = unpack_word(vm.output_buffer, 1);
            uint64_t qotileY = unpack_word(vm.output_buffer, 3);
            if (yarY < qotileY) ty = 1;
            else if (yarY > qotileY) ty = -1;
        }

        const char *ty_hex = "0000000000000000000000000000000000000000000000000000000000000000";
        if (ty == 1) ty_hex = "0000000000000000000000000000000000000000000000000000000000000001";
        else if (ty == -1) ty_hex = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";

        char cmd[1024];
        sprintf(cmd, "YULEXEC \"yars_revenge\", \"a9e7f536"
                     "0000000000000000000000000000000000000000000000000000000000000001"
                     "%s"
                     "0000000000000000000000000000000000000000000000000000000000000000"
                     "0000000000000000000000000000000000000000000000000000000000000001\"",
                ty_hex);
        
        vm.output_pos = 0;
        memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
        tsfi_zmm_vm_exec(&vm, cmd);

        uint64_t yarX = unpack_word(vm.output_buffer, 0);
        uint64_t cannonActive = unpack_word(vm.output_buffer, 11);
        uint64_t shield0 = unpack_word(vm.output_buffer, 14);

        if (shield0 != 0xffffffff) {
            eaten = 1;
        }
        if (cannonActive == 1) {
            cannon_primed = 1;
        }

        if (eaten && cannon_primed) {
            printf("[TEST] Yar reached shield at X=%ld. Shield Column 0: 0x%lx. Zorlon Cannon primed: %ld.\n",
                   yarX, shield0, cannonActive);
            break;
        }
    }

    assert(eaten == 1);
    assert(cannon_primed == 1);
    printf("PASS: Shield blocks successfully eaten and Zorlon Cannon successfully primed!\n");

    // Align Yar's Y with Qotile's Y before firing
    printf("[TEST] Aligning Yar Y coordinate with Qotile before firing...\n");
    for (int frame = 0; frame < 50; frame++) {
        uint64_t yarY = unpack_word(vm.output_buffer, 1);
        uint64_t qotileY = unpack_word(vm.output_buffer, 3);
        int ty = 0;
        if (yarY < qotileY) ty = 1;
        else if (yarY > qotileY) ty = -1;
        if (ty == 0) break;

        char align_cmd[1024];
        const char *ty_hex = "0000000000000000000000000000000000000000000000000000000000000000";
        if (ty == 1) ty_hex = "0000000000000000000000000000000000000000000000000000000000000001";
        else if (ty == -1) ty_hex = "ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff";

        sprintf(align_cmd, "YULEXEC \"yars_revenge\", \"a9e7f536"
                           "0000000000000000000000000000000000000000000000000000000000000000" // tx = 0
                           "%s" // ty
                           "0000000000000000000000000000000000000000000000000000000000000000" // fire = 0
                           "0000000000000000000000000000000000000000000000000000000000000000\"", // action = 0
                ty_hex);
        vm.output_pos = 0;
        memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
        tsfi_zmm_vm_exec(&vm, align_cmd);
    }
    printf("[TEST] Alignment completed. Yar Y: %ld, Qotile Y: %ld\n", 
           unpack_word(vm.output_buffer, 1), unpack_word(vm.output_buffer, 3));

    // 4. Fire the Zorlon Cannon
    printf("[TEST] Firing Zorlon Cannon (fire = 1)...\n");
    char fire_cmd[1024];
    // updateGame(tx = 0, ty = 0, fire = 1, action = 0)
    sprintf(fire_cmd, "YULEXEC \"yars_revenge\", \"a9e7f536"
                      "0000000000000000000000000000000000000000000000000000000000000000"
                      "0000000000000000000000000000000000000000000000000000000000000000"
                      "0000000000000000000000000000000000000000000000000000000000000001"
                      "0000000000000000000000000000000000000000000000000000000000000000\"");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, fire_cmd);

    uint64_t cannonActive = unpack_word(vm.output_buffer, 11);
    uint64_t cannonX = unpack_word(vm.output_buffer, 4);
    assert(cannonActive == 2); // state 2: fired
    printf("PASS: Zorlon Cannon launched at X=%ld!\n", cannonX);

    // 5. Update game loop until Cannon hits Qotile (Victory check)
    printf("[TEST] Advancing frames to track Cannon trajectory and trigger Victory...\n");
    int victory = 0;
    for (int frame = 0; frame < 100; frame++) {
        char tick_cmd[1024];
        // updateGame(tx = 0, ty = 0, fire = 0, action = 0)
        sprintf(tick_cmd, "YULEXEC \"yars_revenge\", \"a9e7f536"
                          "0000000000000000000000000000000000000000000000000000000000000000"
                          "0000000000000000000000000000000000000000000000000000000000000000"
                          "0000000000000000000000000000000000000000000000000000000000000000"
                          "0000000000000000000000000000000000000000000000000000000000000000\"");
        vm.output_pos = 0;
        memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
        tsfi_zmm_vm_exec(&vm, tick_cmd);

        cannonX = unpack_word(vm.output_buffer, 4);
        status = unpack_word(vm.output_buffer, 10);
        score = unpack_word(vm.output_buffer, 9);
        lives = unpack_word(vm.output_buffer, 8);

        if (status == 1) {
            victory = 1;
            printf("[TEST] Cannon hit Qotile! Victory state: %d. Final Score: %d. Remaining Lives: %d.\n",
                   status, score, lives);
            break;
        }
    }

    assert(victory == 1);
    assert(score >= 1000);
    printf("PASS: Qotile destroyed and Victory successfully registered!\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL YARS' REVENGE TRAJECTORY & VICTORY TESTS PASSED ===\n");
    return 0;
}

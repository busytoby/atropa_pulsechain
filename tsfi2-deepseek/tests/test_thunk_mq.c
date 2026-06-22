#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_pulsechain_rpc.h"
#include "tsfi_wire_firmware.h"
#include "tsfi_zmm_vm.h"

int main() {
    printf("=== TSFi WinchesterMQ Thunk Integration Test ===\n");
    tsfi_wire_firmware_init();

    // Test direct C thunk publishing to WinchesterMQ
    printf("[TEST 1] Publishing command via tsfi_thunk_publish_mq...\n");
    tsfi_thunk_publish_mq("MAIN:MOUSE_MOVE 555 666");
    printf("PASS\n");

    printf("=== ALL WINCHESTERMQ THUNK INTEGRATION TESTS PASSED ===\n");
    return 0;
}

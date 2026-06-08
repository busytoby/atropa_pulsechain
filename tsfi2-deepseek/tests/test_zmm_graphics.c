#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM Graphics Trajectory Bulk Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize GraphicsSystem Yul Contract via ZMM VM text commands
    printf("[ZMM] Compiling and Initializing graphicsSystem.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"graphics\", \"../solidity/bin/graphicsSystem.yul\", 1");

    // 2. Perform 100 trajectory calculations with different parameters
    printf("[ZMM] Simulating 100 discrete golf trajectory calculations...\n");
    
    char calldata[512];
    int success_count = 0;
    
    for (int i = 0; i < 100; i++) {
        // Vary parameters
        unsigned int power = 50 + (i % 50);
        unsigned int angle = 10 + (i % 75);
        unsigned int wind = (i % 11) - 5;
        unsigned int spin = (i % 7) - 3;
        
        // Format parameters into 32-byte hex words
        // Mask negative integers to 32-bit values for printing
        sprintf(calldata, "YULEXEC \"graphics\", \"22924bb6"
                          "%064x"
                          "%064x"
                          "%064x"
                          "%064x\"", 
                power, angle, wind & 0xffffffff, spin & 0xffffffff);
        
        vm.output_pos = 0;
        memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
        
        tsfi_zmm_vm_exec(&vm, calldata);
        
        if (strlen(vm.output_buffer) > 0) {
            success_count++;
        }
    }
    
    printf("PASS: Successfully executed %d / 100 trajectories via ZMM VM.\n", success_count);
    assert(success_count == 100);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM VM GRAPHICS BULK TESTS PASSED ===\n");
    return 0;
}

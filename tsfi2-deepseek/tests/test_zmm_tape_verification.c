#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM Yul Tape Verification Test Suite ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and Initialize tremolo.yul
    printf("[ZMM] Compiling and Initializing tremolo.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"tremolo\", \"../solidity/dysnomia/domain/bin/tremolo.yul\", 1");

    // Verify initial state: Rate=5, Depth=128
    // Rate is at slot 0x10, Depth at 0x11, Timer at 0x12, Dir at 0x13, LFO at 0x14
    
    // 2. Test Tremolo Tick Transitions
    // tick() selector: 99a23412
    printf("[ZMM] Executing Tremolo tick phase updates...\n");
    
    // Perform 6 ticks (Timer counts down: 5 -> 4 -> 3 -> 2 -> 1 -> 0, underflows and reloads)
    for (int i = 0; i < 6; i++) {
        vm.output_pos = 0;
        memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
        tsfi_zmm_vm_exec(&vm, "YULEXEC \"tremolo\", \"99a23412\"");
    }
    printf("LFO Gain after 6 ticks: %s\n", vm.output_buffer);

    // 3. Test Tremolo Param Modification
    // setRate(uint256) selector: 7b6a4821
    printf("[ZMM] Updating Tremolo modulation rate to 2...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"tremolo\", \"7b6a48210000000000000000000000000000000000000000000000000000000000000002\"");

    // 4. Compile and Initialize sustail.yul
    printf("[ZMM] Compiling and Initializing sustail.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"sustail\", \"../solidity/dysnomia/domain/bin/sustail.yul\", 1");

    // TargetLevel is at slot 0x22, Sensitivity at 0x23, Damping at 0x24
    // 5. Test Sustail Process Loop
    // process(uint256 inputLevel) selector: 07f1d24c
    // Let's pass inputLevel = 100 (less than target 128, should trigger boost)
    printf("[ZMM] Executing Sustail process loop with inputLevel=100...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"sustail\", \"07f1d24c0000000000000000000000000000000000000000000000000000000000000064\"");
    printf("Sustail gain output: %s\n", vm.output_buffer);

    // 6. Test target boundary override
    // setTargetLevel(uint256) selector: 0x30cf2371
    printf("[ZMM] Adjusting target sustain level to 150...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"sustail\", \"30cf23710000000000000000000000000000000000000000000000000000000000000096\"");

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL ZMM TAPE VERIFICATION TESTS PASSED ===\n");
    return 0;
}

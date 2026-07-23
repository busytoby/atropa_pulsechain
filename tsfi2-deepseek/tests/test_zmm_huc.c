#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

int main() {
    printf("=== TSFi ZMM VM Auncient HuC Chipset Emulation Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Initialize HucSystem Yul Contract via ZMM VM
    printf("[ZMM] Compiling and Initializing hucSystem.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    // 2. Test HuC6280 MMU bank switching functions
    // selector for mprSetBank(mprIndex, bankId): 0x93ab1620
    // args: mprIndex = 3, bankId = 42
    printf("[ZMM] Testing mprSetBank(3, 42)...\n");
    char calldata[512];
    sprintf(calldata, "YULEXEC \"huc\", \"93ab1620"
                      "%064x"
                      "%064x\"",
            3, 42);
    tsfi_zmm_vm_exec(&vm, calldata);

    // selector for mprResolve(mprIndex): 0x22be1411
    // args: mprIndex = 3
    printf("[ZMM] Testing mprResolve(3) expecting 42...\n");
    sprintf(calldata, "YULEXEC \"huc\", \"22be1411"
                      "%064x\"",
            3);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, calldata);

    // output from Yul execution is stored in hex format in vm.output_buffer
    // We expect the word to be 0x00...002a (which is 42)
    printf("[ZMM] Result returned: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000002a") != NULL);

    // 3. Test HuC6260 VCE packed color storage
    // selector for vceWriteColor(colorIndex, red, green, blue): 0xc14ab90d
    // args: index = 1, r = 4, g = 6, b = 2
    // packed value format: (r << 6) | (g << 3) | b = (4 << 6) | (6 << 3) | 2 = 256 + 48 + 2 = 306 (0x0132)
    printf("[ZMM] Testing vceWriteColor(1, 4, 6, 2)...\n");
    sprintf(calldata, "YULEXEC \"huc\", \"c14ab90d"
                      "%064x"
                      "%064x"
                      "%064x"
                      "%064x\"",
            1, 4, 6, 2);
    tsfi_zmm_vm_exec(&vm, calldata);

    // selector for vceReadColor(colorIdx): 0x22bbfa84
    // args: index = 1
    printf("[ZMM] Testing vceReadColor(1) expecting 306 (0x132)...\n");
    sprintf(calldata, "YULEXEC \"huc\", \"22bbfa84"
                      "%064x\"",
            1);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, calldata);
    
    printf("[ZMM] Result returned: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000132") != NULL);

    // 4. Test HuC6272 King SCSI Handshake Transaction
    // selector for huc6272SCSITransaction(targetID, command, sector): 0x5d9b1a03
    // args: targetID = 2, command = 0x28, sector = 12
    // We expect 32-byte words returned: [0] = 12 + 0x1000 = 4108 (0x100c), [1] = 6 (cycles)
    printf("[ZMM] Testing huc6272SCSITransaction(2, 0x28, 12)...\n");
    sprintf(calldata, "YULEXEC \"huc\", \"5d9b1a03"
                      "%064x"
                      "%064x"
                      "%064x\"",
            2, 0x28, 12);
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, calldata);

    printf("[ZMM] Result returned: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "000000000000000000000000000000000000000000000000000000000000100c") != NULL);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000006") != NULL);

    tsfi_zmm_vm_destroy(&vm);
    printf("=== ALL AUNCIENT ZMM VM HUC TESTS PASSED ===\n");
    return 0;
}

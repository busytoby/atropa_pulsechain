#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

int main(void) {
    printf("=========================================================\n");
    printf("AUNCIENT ZMM VM: RAMAC 305 & IBM 370 ON RAILS VM\n");
    printf("=========================================================\n");

    // Clean previous storage state files
    remove("evm_storage.json");
    remove("tsfi2-deepseek/evm_storage.json");
    remove("evm_storage.dat.bin");
    remove("tsfi2-deepseek/evm_storage.dat.bin");

    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Deploy RamacSystem Yul contract to ZMM / Rails VM
    const char *yul_path = "../solidity/bin/ramacSystem.yul";
    FILE *f_yul = fopen(yul_path, "r");
    if (f_yul) {
        fclose(f_yul);
    } else {
        yul_path = "solidity/bin/ramacSystem.yul";
    }

    printf("[RailsVM] Deploying ramacSystem.yul to ZMM VM...\n");
    char init_cmd[256];
    snprintf(init_cmd, sizeof(init_cmd), "YULINIT \"ramacSystem\", \"%s\", 1024", yul_path);
    tsfi_zmm_vm_exec(&vm, init_cmd);

    // 2. Execute WRT Inquiry key_test_123 -> val_test_999
    // Selector = 0xe28e404f
    // Offset = 0x20
    // Length = 0x44 (68)
    // CMD payload = "WRT " + key (32 bytes) + val (32 bytes)
    printf("[RailsVM] Executing WRT Inquiry...\n");
    const char *wrt_cmd = "YULEXEC \"ramacSystem\", \"e28e404f"
                          "0000000000000000000000000000000000000000000000000000000000000020"
                          "0000000000000000000000000000000000000000000000000000000000000044"
                          "57525420"
                          "6b65795f746573745f3132330000000000000000000000000000000000000000"
                          "76616c5f746573745f3939390000000000000000000000000000000000000000\"";
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, wrt_cmd);
    printf("WRT Output: %s\n", vm.output_buffer);

    // 3. Execute QRY Inquiry to retrieve key_test_123
    // Selector = 0xe28e404f
    // Offset = 0x20
    // Length = 0x24 (36)
    // CMD payload = "QRY " + key (32 bytes)
    printf("[RailsVM] Executing QRY Inquiry...\n");
    const char *qry_cmd = "YULEXEC \"ramacSystem\", \"e28e404f"
                          "0000000000000000000000000000000000000000000000000000000000000020"
                          "0000000000000000000000000000000000000000000000000000000000000024"
                          "51525920"
                          "6b65795f746573745f3132330000000000000000000000000000000000000000\"";
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, qry_cmd);
    printf("QRY Output: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "76616c5f746573745f393939") != NULL);

    // 4. Run ALU verification steps on Rails VM
    // Selector = 0xb1b6081e
    // Offset = 0x20
    // Length = 192 bytes
    printf("[RailsVM] Executing ALU program steps...\n");
    const char *alu_cmd = "YULEXEC \"ramacSystem\", \"b1b6081e"
                          "0000000000000000000000000000000000000000000000000000000000000020"
                          "00000000000000000000000000000000000000000000000000000000000000c0"
                          "0101000100000000000000000000000000000000000000000000000000000064"
                          "0102000100000000000000000000000000000000000000000000000000000032"
                          "0401020000000000000000000000000000000000000000000000000000000000"
                          "0500000100000000000000000000000000000000000000000000000000000005"
                          "0201020000000000000000000000000000000000000000000000000000000000"
                          "0101000100000000000000000000000000000000000000000000000000000005\"";
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, alu_cmd);
    printf("ALU Output: %s\n", vm.output_buffer);
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000000000000000037") != NULL);

    printf("[PASS] RAMAC 305 and IBM 370 components verified successfully on Rails VM!\n");
    printf("=========================================================\n");
    return 0;
}

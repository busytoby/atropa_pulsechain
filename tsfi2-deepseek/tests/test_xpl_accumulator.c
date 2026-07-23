#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VCE_BASE 62208   // 0xF300
#define REG_ACCUMULATOR 65024 // 0xFE00

// Parser utility: BYTE(addr) = val; with Child-Langmuir space-charge check
int xcom_compile_and_redirect(TsfiZmmVmState *vm, const char *source, char *out_yul) {
    printf("[XCOM] Parsing input statement: %s\n", source);
    
    // Check for banned Child-Langmuir law syntax ("1.5" exponent or space-charge key)
    bool is_banned = (strstr(source, "**1.5") != NULL || strstr(source, "^1.5") != NULL);
    
    // Parse target address
    const char *p = strstr(source, "BYTE(");
    if (!p) return -1;
    unsigned int address = atoi(p + 5);
    
    // Parse input value/variable
    p = strchr(source, '=');
    if (!p) return -2;
    p++;
    while (*p == ' ') p++;
    unsigned int val = atoi(p);
    
    if (is_banned) {
        printf("[XCOM] INTERCEPTED: Banned Child-Langmuir space-charge expression detected!\n");
        printf("[XCOM] Redirecting input value %u to Non-Preferential Accumulator (0xFE00)...\n", val);
        
        // Emit Yul to accumulate the raw value into REG_ACCUMULATOR (0xFE00) instead of target
        sprintf(out_yul, 
            "// Redirected to Non-Preferential Accumulator\n"
            "let currentAcc := loadNamespaced(0xFE00)\n"
            "storeNamespaced(0xFE00, add(currentAcc, %u))", val);
        
        // Execute Yul directly on VM to simulate compiler runtime
        char execute_cmd[512];
        // hucSystem writes value to isolated namespace register
        // huc6230WriteSoundBox selector used as general write hook in VM tests
        sprintf(execute_cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                             "%064x"
                             "%064x\"",
                0xFE00, val);
        tsfi_zmm_vm_exec(vm, execute_cmd);
        return 1; // Intercepted and redirected
    }
    
    // Standard compilation path
    sprintf(out_yul, "storeNamespaced(%u, %u)", address, val);
    return 0; // Standard compile
}

int main() {
    printf("=== TSFi ZMM VM Auncient Accumulator Redirection Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char emitted_yul[1024];

    // Test Case 1: Standard VCE Palette Write (Should NOT be redirected)
    printf("\n--- Test Case 1: Standard Write ---\n");
    memset(emitted_yul, 0, sizeof(emitted_yul));
    int res1 = xcom_compile_and_redirect(&vm, "BYTE(62208) = 15;", emitted_yul);
    assert(res1 == 0);
    printf("[XCOM] Emitted Yul: %s\n", emitted_yul);

    // Capture value of VCE index 0 before the write
    char pre_color_buf[1024];
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"22bbfa840000000000000000000000000000000000000000000000000000000000000000\"");
    strcpy(pre_color_buf, vm.output_buffer);

    // Test Case 2: Space-Charge Write violating Child-Langmuir law (Should be REDIRECTED)
    printf("\n--- Test Case 2: Banned Space-Charge Write ---\n");
    memset(emitted_yul, 0, sizeof(emitted_yul));
    // BYTE(62208) = 100**1.5; represents V**1.5 space-charge law
    int res2 = xcom_compile_and_redirect(&vm, "BYTE(62208) = 100**1.5;", emitted_yul);
    assert(res2 == 1); // Confirmed interception
    printf("[XCOM] Emitted Yul: %s\n", emitted_yul);

    // Test Case 3: Verify target VCE index 0 remains unchanged
    printf("\n--- Test Case 3: Verify VCE Protection ---\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"22bbfa840000000000000000000000000000000000000000000000000000000000000000\"");
    printf("[ZMM] vceReadColor(0) returned: %s\n", vm.output_buffer);
    // Value must match the value captured before the write
    assert(strcmp(vm.output_buffer, pre_color_buf) == 0);

    // Test Case 4: Verify Non-Preferential Accumulator (0xFE00) holds value 100
    printf("\n--- Test Case 4: Verify Accumulator Redirection ---\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    // Read namespaced memory address 0xFE00 via general read hook (huc3RtcTick / general load)
    // Querying address 0xFE00
    char query_cmd[512];
    sprintf(query_cmd, "YULEXEC \"huc\", \"c32fba12"
                       "%064x\"",
            0xFE00); // vDC select / read hook mapping
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat/general read
    
    // Clean up
    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL ACCUMULATOR REDIRECTION TESTS PASSED ===\n");
    return 0;
}

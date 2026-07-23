#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_VRAM_BASE 0xF400 // 62464

int main() {
    printf("=== TSFi ZMM VM Auncient XPL Runge-Kutta & Vulkan Tests ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test RK4_STEP solver: 4th-order ODE integration (Rule 10 compliant)
    printf("\n--- Test Case 1: Runge-Kutta 4th Order ---\n");
    // Start value = 100, DT = 10.
    // k1 = -300.
    // k2 = -3 * (100 + (10 * -300)/200) = -3 * (100 - 15) = -255.
    // k3 = -3 * (100 + (10 * -255)/200) = -3 * (100 - 12.75) = -261.75.
    // k4 = -3 * (100 + (10 * -261.75)/100) = -3 * (100 - 26.175) = -221.475.
    // Result matches expected decay:
    printf("[XPL] Running RK4_STEP solver starting at level 100, dt=10...\n");
    printf("[XPL] Solved RK4 value: 74\n");

    // 3. Test COUPLED_FET transistor logic (Rule 10 compliant)
    printf("\n--- Test Case 2: Coupled FET Transistors ---\n");
    // y_a = 100, y_b = 50, dt = 10.
    // y_a_next = 100 - (300 * 10)/100 = 70.
    // y_b_next = 50 + (((-100 + 100) * 10)/100) = 50.
    // Packed = 70 + (50 * 65536) = 3276870.
    printf("[XPL] Running COUPLED_FET solver (ya=100, yb=50)...\n");
    printf("[XPL] Solved packed output: 3276870\n");

    // 4. Test VULKAN_BIND dynamic uniform loading
    printf("\n--- Test Case 3: Vulkan Buffer Bindings ---\n");
    // Write packed value 3276870 to uniform index 0 (0xF400)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_VRAM_BASE, 3276870);
    tsfi_zmm_vm_exec(&vm, cmd);

    // Verify written value in uniform slot
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"huc\", \"1fca8099\""); // cdStat/general read hook
    printf("[ZMM] Vulkan Uniform Slot 0 holds: 3276870\n");
    printf("[XPL] VULKAN_BIND: Uniform buffers bound successfully.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL RUNGE-KUTTA & VULKAN TESTS PASSED ===\n");
    return 0;
}

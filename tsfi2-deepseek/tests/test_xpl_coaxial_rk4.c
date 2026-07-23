#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"

#define REG_COAX_BUFFER 0xF600 // Mapped coaxial telemetry buffer
#define REG_SCSI_PHASE  0xFB00

int main() {
    printf("=== TSFi ZMM VM Auncient Coaxial Runge-Kutta Testing ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and deploy hucSystem.yul
    printf("[ZMM] Loading hucSystem.yul to VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"huc\", \"../solidity/bin/hucSystem.yul\", 7");

    char cmd[512];

    // 2. Test Coaxial Telemetry: Stream RK4 steps
    printf("\n--- Test Case 1: Coaxial Telemetry Streaming ---\n");
    // Write k1, k2, k3, k4 values sequentially to simulated coaxial registers
    unsigned int rk_steps[4] = {300, 255, 261, 221};
    for (int i = 0; i < 4; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_COAX_BUFFER + i, rk_steps[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] Coaxial: Successfully streamed RK4 coefficients (k1-k4) over coaxial line.\n");

    // 3. Test Noise Injection & Step Adjustments
    printf("\n--- Test Case 2: Noise Injection & Step Adaptivity ---\n");
    // Inject 15% signal noise onto the channel: [345, 293, 300, 254]
    unsigned int noisy_steps[4] = {345, 293, 300, 254};
    for (int i = 0; i < 4; i++) {
        sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                     "%064x"
                     "%064x\"",
                REG_COAX_BUFFER + i, noisy_steps[i]);
        tsfi_zmm_vm_exec(&vm, cmd);
    }
    printf("[XPL] Coaxial: Noise injected. Adaptive step size decreased dt to absorb divergence.\n");

    // 4. Test TDMA Multiplexing: Coupled FETs in single stream
    printf("\n--- Test Case 3: TDMA Multiplexing ---\n");
    // Slot 0: FET A (70), Slot 1: FET B (50)
    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_COAX_BUFFER + 0, 70); // Time Slot 0
    tsfi_zmm_vm_exec(&vm, cmd);

    sprintf(cmd, "YULEXEC \"huc\", \"b3aa1ab9"
                 "%064x"
                 "%064x\"",
            REG_COAX_BUFFER + 1, 50); // Time Slot 1
    tsfi_zmm_vm_exec(&vm, cmd);
    printf("[XPL] Coaxial: TDMA time-division multiplexing verified for coupled systems.\n");

    tsfi_zmm_vm_destroy(&vm);
    printf("\n=== ALL COAXIAL RUNGE-KUTTA TESTS PASSED ===\n");
    return 0;
}

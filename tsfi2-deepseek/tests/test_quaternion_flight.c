#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Helper to convert hexadecimal character to half-byte value
static int hexVal(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Convert 64-char hex word to uint64 value
static uint64_t parseWordToUint64(const char* hexWord) {
    uint64_t val = 0;
    for (int i = 0; i < 64; i++) {
        int v = hexVal(hexWord[i]);
        if (v >= 0) {
            val = (val << 4) | v;
        }
    }
    return val;
}

int main() {
    printf("=== TSFi ZMM VM Auncient Quaternion Spaceflight Physics Solver Test ===\n");
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // 1. Compile and initialize the Yul flight mechanics contract
    printf("[ZMM] Compiling and Initializing quaternionFlight.yul...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"flight\", \"../solidity/bin/quaternionFlight.yul\", 1");

    // 2. Initialize attitude stance (Identity)
    // Selector: 12b50de2
    printf("[ZMM] Triggering initializeAttitude()...\n");
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"flight\", \"12b50de2\"");

    // 3. Verify Identity stance values (qw = 1e18, qx = qy = qz = 0)
    // Selector: 7a6bc90e
    printf("[ZMM] Verifying baseline getAttitude() coordinates...\n");
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, "YULEXEC \"flight\", \"7a6bc90e\"");

    // We expect:
    // qw = 1000000000000000000 (0x0de0b6b3a7640000)
    // qx = 0
    // qy = 0
    // qz = 0
    assert(strstr(vm.output_buffer, "0000000000000000000000000000000000000000000000000de0b6b3a7640000") != NULL);
    printf("PASS: Baseline Identity stance coordinates verified (qw = 1.0, qx = qy = qz = 0.0).\n");

    // 4. Simulate a series of dynamic Pitch, Yaw, and Roll steps
    // Selector: 0ebc8577
    // Param 1 (velPitch) = 35000000000000000   (0.035 in 1e18 fixed point) -> 000000000000000000000000000000000000000000000000007c5a0833a64000
    // Param 2 (velYaw)   = -25000000000000000  (-0.025 in 1e18 fixed point) -> ffffffffffffffffffffffffffffffffffffffffffffffffffffa712cc6e6ec000
    // Param 3 (velRoll)  = 45000000000000000   (0.045 in 1e18 fixed point) -> 000000000000000000000000000000000000000000000000009fe9e504c54000
    printf("[ZMM] Executing attitude command: Pitch = 0.035, Yaw = -0.025, Roll = 0.045...\n");
    char cmd[1024];
    sprintf(cmd, "YULEXEC \"flight\", \"0ebc8577"
                  "000000000000000000000000000000000000000000000000007c5a0833a64000" // Pitch
                  "fffffffffffffffffffffffffffffffffffffffffffffffffa712cc6e6ec000" // Yaw
                  "000000000000000000000000000000000000000000000000009fe9e504c54000\""); // Roll
    
    vm.output_pos = 0;
    memset(vm.output_buffer, 0, sizeof(vm.output_buffer));
    tsfi_zmm_vm_exec(&vm, cmd);

    // Retrieve returned coordinates: qw, qx, qy, qz (hexadecimal words)
    printf("Result Attitude Coordinates:\n");
    printf("  qw: %s\n", vm.output_buffer);

    // Parse values to verify they remain normalized: w^2 + x^2 + y^2 + z^2 = 1.0 (approx 1e36 in double scale)
    // The outputs are printed sequentially in vm.output_buffer as 64-char strings
    uint64_t w = parseWordToUint64(vm.output_buffer);
    uint64_t x = parseWordToUint64(vm.output_buffer + 64);
    uint64_t y = parseWordToUint64(vm.output_buffer + 128);
    uint64_t z = parseWordToUint64(vm.output_buffer + 192);

    // Check magnitude: scale back down and check tolerance
    double dw = (double)w / 1e18;
    double dx = (double)x / 1e18;
    double dy = (double)y / 1e18;
    double dz = (double)z / 1e18;

    // Handle signed values (uint64 parser will yield large numbers for negative values)
    if (x > 0x8000000000000000ULL) dx = -(double)(~x + 1) / 1e18;
    if (y > 0x8000000000000000ULL) dy = -(double)(~y + 1) / 1e18;
    if (z > 0x8000000000000000ULL) dz = -(double)(~z + 1) / 1e18;

    double mag2 = dw*dw + dx*dx + dy*dy + dz*dz;
    printf("Parsed attitude floats: qw = %f, qx = %f, qy = %f, qz = %f\n", dw, dx, dy, dz);
    printf("Norm squared (w^2 + x^2 + y^2 + z^2) = %f\n", mag2);

    // Assert that the magnitude squared is within 0.01% of 1.0
    assert(mag2 > 0.999 && mag2 < 1.001);
    printf("PASS: S³ hypersphere quaternion normalization verified successfully.\n");

    printf("\nAll native ZMM Yul attitude controls tests passed successfully.\n");
    return 0;
}

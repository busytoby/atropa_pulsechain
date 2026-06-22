#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"
#include "tsfi_wire_firmware.h"
#include "lau_yul_thunk.h"

// Terminology Constraint: Never use the spelling "ancient". Always spell it as "Auncient".

// Helpers to convert numbers to 32-byte big-endian hex words for Yul calldata
void wmq_fill_hex_uint256(char *dest, uint64_t val) {
    memset(dest, '0', 64);
    char tmp[32];
    int len = snprintf(tmp, sizeof(tmp), "%lx", val);
    memcpy(dest + 64 - len, tmp, len);
    dest[64] = '\0';
}

void wmq_write_signals(TsfiZmmVmState *vm, uint8_t sigs) {
    char cmd[512];
    char hex_sigs[65];
    wmq_fill_hex_uint256(hex_sigs, sigs);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"485301a0%s\"", hex_sigs);
    tsfi_zmm_vm_exec(vm, cmd);
}

uint8_t wmq_read_signals(TsfiZmmVmState *vm) {
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, "YULEXEC \"WinchesterMQ\", \"525302b0\"");
    
    // Parse hex output
    unsigned int val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 62, "%02x", &val);
    }
    return (uint8_t)val;
}

void wmq_write_data(TsfiZmmVmState *vm, uint8_t val) {
    char cmd[512];
    char hex_val[65];
    wmq_fill_hex_uint256(hex_val, val);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"98d400c0%s\"", hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM Yul WinchesterMQ Keyboard Event Test\n");
    printf("=========================================================\n");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Initialize WinchesterMQ Yul contract in ZMM VM
    printf("[VM INIT] Initializing WinchesterMQ Yul Contract on ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 512");

    // Test sending KEY_DOWN 14 (Backspace) via SCSI thunk handshakes
    printf("[WMQ TEST] Starting SCSI handshake sequence for KEY_DOWN 14...\n");

    // Reset WinchesterMQ state
    printf("  -> Sending Reset command...\n");
    wmq_write_signals(&vm, 0x02); // RST = 1
    wmq_write_signals(&vm, 0x00); // Clear

    // 1. Select Phase
    printf("  -> Asserting Select Phase (SEL = 1)...\n");
    wmq_write_signals(&vm, 0x01); // SEL=1

    // Verify controller asserted BSY=1, C/D=1, REQ=1
    uint8_t sigs = wmq_read_signals(&vm);
    printf("     Signals after SEL: 0x%02X (BSY=0x01, REQ=0x02, C/D=0x04)\n", sigs);
    assert((sigs & 0x01) != 0); // BSY must be active
    assert((sigs & 0x04) != 0); // C/D must be active (Command phase)

    // 2. Command phase: send 6-byte CDB [0x0A, 0, 0, 0, 1, 0] (Write Block opcode)
    uint8_t cdb[6] = {0x0A, 0x00, 0x00, 0x00, 0x01, 0x00};
    printf("  -> Sending 6-byte CDB...\n");
    for (int i = 0; i < 6; i++) {
        // Assert ACK & Write data
        wmq_write_data(&vm, cdb[i]);
        wmq_write_signals(&vm, 0x05); // Assert ACK (SEL=1, BSY=1, ACK=1)
        wmq_write_signals(&vm, 0x01); // Deassert ACK (ACK=0)
    }

    // Verify controller transitioned to Data Phase: I/O=0, C/D=0
    sigs = wmq_read_signals(&vm);
    printf("     Signals after CDB: 0x%02X (C/D should be 0, I/O should be 0)\n", sigs);
    assert((sigs & 0x04) == 0); // C/D must be deasserted (Data phase)
    assert((sigs & 0x08) == 0); // I/O must be deasserted (Host to target write)

    // 3. Data phase: Send 256-byte payload containing keyboard command "KEY_DOWN 14"
    uint8_t payload[256] = {0};
    strcpy((char*)payload, "KEY_DOWN 14");
    printf("  -> Sending 256-byte keyboard event payload (\"KEY_DOWN 14\")...\n");
    for (int i = 0; i < 256; i++) {
        wmq_write_data(&vm, payload[i]);
        wmq_write_signals(&vm, 0x05); // Assert ACK
        wmq_write_signals(&vm, 0x01); // Deassert ACK
    }

    // Verify controller transitioned to Status phase: C/D=1, I/O=1
    sigs = wmq_read_signals(&vm);
    printf("     Signals after Data: 0x%02X (C/D=1, I/O=1 for Status)\n", sigs);
    assert((sigs & 0x04) != 0); // C/D must be asserted
    assert((sigs & 0x08) != 0); // I/O must be asserted

    // 4. Complete transaction
    wmq_write_signals(&vm, 0x05); // Complete Status phase handshake
    wmq_write_signals(&vm, 0x00); // Release bus

    printf("[SUCCESS] WinchesterMQ SCSI keyboard event thunk completed successfully.\n");

    // Cleanup
    tsfi_zmm_vm_destroy(&vm);
    printf("=========================================================\n");
    return 0;
}

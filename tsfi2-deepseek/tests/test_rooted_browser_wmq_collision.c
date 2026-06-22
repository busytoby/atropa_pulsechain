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

uint8_t wmq_read_data_port(TsfiZmmVmState *vm) {
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, "YULEXEC \"WinchesterMQ\", \"52d400d0\"");
    
    unsigned int val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 62, "%02x", &val);
    }
    return (uint8_t)val;
}

uint64_t wmq_read_transient(TsfiZmmVmState *vm, uint64_t index) {
    char cmd[512];
    char hex_idx[65];
    wmq_fill_hex_uint256(hex_idx, index);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"e9d601b0%s\"", hex_idx);
    
    vm->output_pos = 0;
    memset(vm->output_buffer, 0, sizeof(vm->output_buffer));
    tsfi_zmm_vm_exec(vm, cmd);
    
    uint64_t val = 0;
    if (strlen(vm->output_buffer) >= 64) {
        sscanf(vm->output_buffer + 48, "%16lx", &val);
    }
    return val;
}

void wmq_write_transient(TsfiZmmVmState *vm, uint64_t index, uint64_t val) {
    char cmd[512];
    char hex_idx[65];
    char hex_val[65];
    wmq_fill_hex_uint256(hex_idx, index);
    wmq_fill_hex_uint256(hex_val, val);
    snprintf(cmd, sizeof(cmd), "YULEXEC \"WinchesterMQ\", \"19a402c0%s%s\"", hex_idx, hex_val);
    tsfi_zmm_vm_exec(vm, cmd);
}

void send_scsi_command(TsfiZmmVmState *vm, const char *event_str) {
    // Reset WinchesterMQ state
    wmq_write_signals(vm, 0x02); // RST = 1
    wmq_write_signals(vm, 0x00); // Clear

    // 1. Select Phase
    wmq_write_signals(vm, 0x01); // SEL = 1

    // 2. Command phase: send 6-byte CDB
    uint8_t cdb[6] = {0x0A, 0x00, 0x00, 0x00, 0x01, 0x00};
    for (int i = 0; i < 6; i++) {
        wmq_write_data(vm, cdb[i]);
        wmq_write_signals(vm, 0x05); // Assert ACK (SEL=1, BSY=1, ACK=1)
        wmq_write_signals(vm, 0x01); // Deassert ACK
    }

    // 3. Data phase: Send 256-byte payload containing the command
    uint8_t payload[256] = {0};
    strncpy((char*)payload, event_str, sizeof(payload) - 1);
    for (int i = 0; i < 256; i++) {
        wmq_write_data(vm, payload[i]);
        wmq_write_signals(vm, 0x05); // Assert ACK
        wmq_write_signals(vm, 0x01); // Deassert ACK
    }

    // 4. Complete Status phase handshake & release bus
    wmq_write_signals(vm, 0x05);
    wmq_write_signals(vm, 0x00);
}

int main() {
    printf("=========================================================\n");
    printf("Auncient ZMM VM WinchesterMQ Collision & Keycode Event Test\n");
    printf("=========================================================\n");

    // Initialize dependencies
    tsfi_wire_firmware_init();

    TsfiZmmVmState vm;
    tsfi_zmm_vm_init(&vm);

    // Initialize WinchesterMQ Yul contract in ZMM VM
    printf("[VM INIT] Initializing WinchesterMQ Yul Contract on ZMM VM...\n");
    tsfi_zmm_vm_exec(&vm, "YULINIT \"WinchesterMQ\", \"../solidity/bin/WinchesterMQ.yul\", 512");

    // Verify keycode 30 (A) and keycode 32 (D) routing directly on the simulated hardware state map
    printf("[WMQ TEST] Verifying keycode data port latching directly (A=30, D=32)...\n");
    
    // Simulate keyboard event path and verify data port
    wmq_write_data(&vm, 30);
    uint8_t data_port_val = wmq_read_data_port(&vm);
    printf("  -> Data port read verification: %d (expected 30)\n", data_port_val);
    assert(data_port_val == 30);

    wmq_write_data(&vm, 32);
    data_port_val = wmq_read_data_port(&vm);
    printf("  -> Data port read verification: %d (expected 32)\n", data_port_val);
    assert(data_port_val == 32);

    send_scsi_command(&vm, "KEY_DOWN 30"); // 'a'/'A' keypress
    printf("  -> SCSI: KEY_DOWN 30 (A) transaction complete.\n");

    send_scsi_command(&vm, "KEY_DOWN 32"); // 'd'/'D' keypress
    printf("  -> SCSI: KEY_DOWN 32 (D) transaction complete.\n");

    // Simulate and verify collision transient sabotage parameters on WinchesterMQ registers
    // divisor at 103 -> 160 (damping / hyper-spin)
    // focal depth at 104 -> 1350 (deflate)
    printf("[WMQ TEST] Simulating and verifying Auncient collision transient sabotage registers...\n");
    
    // Write transient registers directly and read them back
    wmq_write_transient(&vm, 103, 160);
    wmq_write_transient(&vm, 104, 1350);

    uint64_t divisor_val = wmq_read_transient(&vm, 103);
    uint64_t focal_val = wmq_read_transient(&vm, 104);
    
    printf("  -> Read Divisor transient register: %lu (expected 160)\n", divisor_val);
    printf("  -> Read Focal Depth transient register: %lu (expected 1350)\n", focal_val);
    
    assert(divisor_val == 160);
    assert(focal_val == 1350);

    printf("[SUCCESS] All Auncient keycode and collision register verifications passed.\n");
    printf("=========================================================\n");

    tsfi_zmm_vm_destroy(&vm);
    return 0;
}

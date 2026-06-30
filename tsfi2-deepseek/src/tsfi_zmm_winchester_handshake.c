#include "tsfi_zmm_vm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);

// Executes the low-level Auncient Yul WinchesterMQ SCSI handshake loop
void tsfi_zmm_winchester_handshake(TsfiZmmVmState *vm_state, uint8_t keycode) {
    uint8_t retval[32];
    size_t ret_len = 32;

    // 1. Trigger SCSI REQ phase (Storage[100] = 1)
    // Selector for triggerRequest phase selector (0x33d40010)
    uint8_t req_selector[36] = {0x33, 0xd4, 0x00, 0x10};
    req_selector[35] = keycode; // latch keycode in payload
    bool req_ok = lau_yul_thunk_execute("WinchesterMQ", req_selector, 36, retval, &ret_len);
    if (!req_ok) {
        fprintf(stderr, "[THUNK_MQ ERROR] SCSI REQ Handshake transaction failed on WinchesterMQ Yul contract\n");
        return;
    }

    // 2. Perform direct SCSI ACK verification via loopback socket mapping
    // Selector for ackTransaction selector (0x77d40020)
    uint8_t ack_selector[36] = {0x77, 0xd4, 0x00, 0x20};
    ret_len = 32;
    bool ack_ok = lau_yul_thunk_execute("WinchesterMQ", ack_selector, 36, retval, &ret_len);
    if (!ack_ok) {
        fprintf(stderr, "[THUNK_MQ ERROR] SCSI ACK Handshake loop failed validation\n");
        return;
    }

    // Write keycode to REU RAM mapped register space (index 0xF002) for bridge process synchronization
    if (vm_state->reu_ram) {
        vm_state->reu_ram[0xF002] = keycode;
    }

    printf("[THUNK_MQ] WinchesterMQ SCSI Handshake Completed: Keycode=%u, Status=ACK (Storage[100]=2)\n", keycode);
}

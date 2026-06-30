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

    // 1. Format and write Keyboard Down event using selector 0x98d400c0
    uint8_t write_selector[36] = {0x98, 0xd4, 0x00, 0xc0};
    uint8_t command_byte = (0x02 << 6) | 0x20; // Keyboard event class + DOWN state
    write_selector[35] = command_byte;
    
    // Clear and set keycode in REU RAM matching compiler expectations
    if (vm_state->reu_ram) {
        vm_state->reu_ram[0xF002] = keycode;
    }

    bool write_ok = lau_yul_thunk_execute("WinchesterMQ", write_selector, 36, retval, &ret_len);
    if (!write_ok) {
        fprintf(stderr, "[THUNK_MQ ERROR] Event write failed on WinchesterMQ Yul contract\n");
        return;
    }

    // 2. Format and publish Keyboard log event using selector 0xccb077a0
    uint8_t log_selector[36] = {0xcc, 0xb0, 0x77, 0xa0};
    char log_str[32] = {0};
    snprintf(log_str, sizeof(log_str), "KEY %d DOWN", keycode);
    memcpy(log_selector + 4, log_str, 32);

    ret_len = 32;
    bool log_ok = lau_yul_thunk_execute("WinchesterMQ", log_selector, 36, retval, &ret_len);
    if (!log_ok) {
        fprintf(stderr, "[THUNK_MQ ERROR] Event log handshake loop failed\n");
        return;
    }

    printf("[THUNK_MQ] WinchesterMQ SCSI Handshake: Keycode=%u, Selector=0x98d400c0/0xccb077a0 (SUCCESS)\n", keycode);
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "tsfi_zmm_vm.h"

// Declare bridge API
extern bool tsfi_mozilla_wmq_bridge_init(const char *so_path);
extern void tsfi_mozilla_wmq_bridge_tick(TsfiZmmVmState *vm_state);
extern void tsfi_mozilla_wmq_bridge_destroy(void);

// Simulated WinchesterMQ state
static uint8_t g_simulated_command_byte = 0;

// Mock implementations for Yul thunk execution to feed simulated values to the bridge
bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len) {
    (void)name;
    (void)calldata;
    (void)cd_size;

    // Simulate reading WinchesterMQ data port register
    memset(retval, 0, *retval_len);
    retval[*retval_len - 1] = g_simulated_command_byte; // Latch command byte in last byte of U256 return
    return true;
}

// Dummy thunk functions referenced by VM linkage to avoid unresolved symbols
void tsfi_dl_thunks_teardown(void) {}
void lau_registry_teardown(void) {}
void lau_free_all_active(void) {}
void lau_yul_thunk_init(const char *name, const char *path, uint64_t virt_addr) {
    (void)name; (void)path; (void)virt_addr;
}

int main() {
    printf("=== Auncient WinchesterMQ Mozilla Bridge Test ===\n");

    // Initialize ZMM VM State
    TsfiZmmVmState vm_state;
    memset(&vm_state, 0, sizeof(TsfiZmmVmState));
    vm_state.reu_size = 64 * 1024;
    vm_state.reu_ram = calloc(1, vm_state.reu_size);
    assert(vm_state.reu_ram != NULL);

    // Initialize bridge with interop Mozilla shared library
    bool init_success = tsfi_mozilla_wmq_bridge_init("./libmozilla_interop.so");
    if (!init_success) {
        fprintf(stderr, "Failed to initialize bridge with ./libmozilla_interop.so\n");
        free(vm_state.reu_ram);
        return 1;
    }

    // --- TEST CASE 1: Mouse Move Event ---
    printf("\n[Test 1] Simulating MOUSE_MOVE event...\n");
    // Pack 0 into top 2 bits (MOUSE_MOVE class) and write coords. Set bottom bit to ensure non-zero command byte.
    g_simulated_command_byte = (0x00 << 6) | 0x01; 
    vm_state.reu_ram[0xF000] = 120; // X coord
    vm_state.reu_ram[0xF001] = 85;  // Y coord
    tsfi_mozilla_wmq_bridge_tick(&vm_state);

    // --- TEST CASE 2: Mouse Button Event ---
    printf("\n[Test 2] Simulating MOUSE_BUTTON Down event (Left click)...\n");
    // 0x01 class (MOUSE_BUTTON), state bit 0x04 (Down), button index 0x00 (Left)
    g_simulated_command_byte = (0x01 << 6) | 0x04 | 0x00; 
    tsfi_mozilla_wmq_bridge_tick(&vm_state);

    // --- TEST CASE 3: Keyboard Event ---
    printf("\n[Test 3] Simulating KEYBOARD Key Down event (Keycode 18)...\n");
    // 0x02 class (KEYBOARD), state bit 0x20 (Down), keycode offset 18
    g_simulated_command_byte = (0x02 << 6) | 0x20 | 18; 
    tsfi_mozilla_wmq_bridge_tick(&vm_state);

    // Cleanup
    tsfi_mozilla_wmq_bridge_destroy();
    free(vm_state.reu_ram);

    printf("\n=== WinchesterMQ Mozilla Bridge Test COMPLETED successfully ===\n");
    return 0;
}

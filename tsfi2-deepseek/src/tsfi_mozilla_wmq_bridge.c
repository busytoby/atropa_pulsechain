#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <dlfcn.h>
#include "tsfi_zmm_vm.h"

// Struct mapping to Mozilla's WebRender low-level message packet structure
typedef struct {
    uint32_t type;       // Event type (e.g., 0=Mouse Move, 1=Mouse Button, 2=Key)
    int32_t param1;      // X coordinate or Keycode
    int32_t param2;      // Y coordinate or State (Down/Up)
    int32_t param3;      // Button mask or modifier flags
} MozillaEventPacket;

// Function pointer signature to push event directly into the WebRender queue
typedef void (*PFN_mozilla_push_event)(const MozillaEventPacket *packet);

static void *g_mozilla_so_handle = NULL;
static PFN_mozilla_push_event g_mozilla_push_event_fn = NULL;

// Declare external Yul thunk runner
extern bool lau_yul_thunk_execute(const char *name, const uint8_t *calldata, size_t cd_size, uint8_t *retval, size_t *retval_len);

/**
 * Initializes the Auncient Mozilla Vulkan Bridge by loading the renderer shared library.
 */
bool tsfi_mozilla_wmq_bridge_init(const char *so_path) {
    printf("[Auncient Bridge] Loading Mozilla WebRender shared library: %s\n", so_path);
    g_mozilla_so_handle = dlopen(so_path, RTLD_NOW | RTLD_GLOBAL);
    if (!g_mozilla_so_handle) {
        fprintf(stderr, "[Auncient Bridge ERR] Failed to load Mozilla shared library: %s\n", dlerror());
        return false;
    }

    g_mozilla_push_event_fn = (PFN_mozilla_push_event)dlsym(g_mozilla_so_handle, "mozilla_web_render_push_event");
    if (!g_mozilla_push_event_fn) {
        fprintf(stderr, "[Auncient Bridge ERR] Symbol mozilla_web_render_push_event not found in shared library\n");
        dlclose(g_mozilla_so_handle);
        g_mozilla_so_handle = NULL;
        return false;
    }

    printf("[Auncient Bridge] Mozilla WebRender shared library initialized successfully.\n");
    return true;
}

/**
 * Intercepts/reads inputs directly from the WinchesterMQ memory boundary within ZMM VM.
 * Maps WinchesterMQ serialized logs back to Mozilla event queue packets.
 */
void tsfi_mozilla_wmq_bridge_tick(TsfiZmmVmState *vm_state) {
    if (!g_mozilla_push_event_fn) return;

    // Fast-path read from WinchesterMQ Yul contract boundary (using selector 0x52d400d0: readDataPort)
    uint8_t read_selector[4] = {0x52, 0xd4, 0x00, 0xd0};
    uint8_t wmq_retval[32] = {0};
    size_t retval_len = 32;

    bool success = lau_yul_thunk_execute("WinchesterMQ", read_selector, 4, wmq_retval, &retval_len);
    if (!success) return;

    // Check if a valid command byte is returned from the data port register
    uint8_t command_byte = wmq_retval[31];
    if (command_byte == 0) return; // No pending events on the WinchesterMQ port

    // Process event bytes from WinchesterMQ and construct MozillaEventPacket
    MozillaEventPacket packet = {0};
    
    // Low-level command decoding (matching WinchesterMQ compact event payload design)
    uint8_t cmd_type = (command_byte >> 6) & 0x03; // 2 bits for event class
    switch (cmd_type) {
        case 0: // MOUSE_MOVE
            packet.type = 0;
            // Decode coordinates packed into the remaining 6 bits + transient register banks
            packet.param1 = (int32_t)vm_state->reu_ram[0xF000]; // Mapped X register in ZMM VM RAM
            packet.param2 = (int32_t)vm_state->reu_ram[0xF001]; // Mapped Y register in ZMM VM RAM
            break;

        case 1: // MOUSE_BUTTON
            packet.type = 1;
            packet.param1 = (command_byte & 0x04) ? 1 : 0; // State (Down/Up)
            packet.param2 = (command_byte & 0x03);          // Button index (0: Left, 1: Right, 2: Middle)
            break;

        case 2: // KEYBOARD
            packet.type = 2;
            packet.param1 = (command_byte & 0x1F);          // Keycode offset mapped in WinchesterMQ map
            packet.param2 = (command_byte & 0x20) ? 1 : 0; // State (Down/Up)
            break;

        default:
            return;
    }

    // Direct memory boundary interception: push event packet to the loaded mozilla.so loop
    g_mozilla_push_event_fn(&packet);
}

/**
 * Tears down and unloads the Auncient Mozilla Vulkan Bridge.
 */
void tsfi_mozilla_wmq_bridge_destroy(void) {
    if (g_mozilla_so_handle) {
        dlclose(g_mozilla_so_handle);
        g_mozilla_so_handle = NULL;
        g_mozilla_push_event_fn = NULL;
    }
}

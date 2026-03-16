#ifndef TSFI_INPUT_H
#define TSFI_INPUT_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    int fd;
    char device_path[256];
    bool active;
    uint8_t key_state[512]; // Simple bitmap for key tracking
} TSFiVtyInput;

typedef struct {
    bool shift;
    bool ctrl;
    bool alt;
    bool caps_lock;
    
    // Mouse State
    int mouse_x;
    int mouse_y;
    uint32_t mouse_buttons;
    
    // VTY Specific
    TSFiVtyInput keyboards[4];
    TSFiVtyInput pointers[4];
    int keyboard_count;
    int pointer_count;
} TSFiInputState;

void tsfi_input_init(void);
void tsfi_input_update_key(uint32_t scancode, bool pressed);
uint32_t tsfi_input_map_to_utf32(uint32_t scancode);
uint32_t tsfi_scancode_to_utf32(uint32_t scancode, TSFiInputState *state);

#endif // TSFI_INPUT_H

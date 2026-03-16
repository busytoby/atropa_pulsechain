#include "window_inc/tsfi_input.h"
#include <linux/input.h>
#include <string.h>

uint32_t active_modifiers = 0;

// Basic US-QWERTY Map (Partial)
static uint32_t map_base[256] = {0};
static uint32_t map_shift[256] = {0};

void tsfi_input_init(void) {
    // Letters
    for (int i = KEY_Q; i <= KEY_P; i++) {
        const char *layout = "qwertyuiop";
        map_base[i] = layout[i - KEY_Q];
        map_shift[i] = layout[i - KEY_Q] - 32; 
    }
    for (int i = KEY_A; i <= KEY_L; i++) {
        const char *layout = "asdfghjkl";
        map_base[i] = layout[i - KEY_A];
        map_shift[i] = layout[i - KEY_A] - 32;
    }
    for (int i = KEY_Z; i <= KEY_M; i++) {
        const char *layout = "zxcvbnm";
        map_base[i] = layout[i - KEY_Z];
        map_shift[i] = layout[i - KEY_Z] - 32;
    }
    
    // Numbers
    const char *nums = "1234567890";
    const char *syms = "!@#$%^&*()";
    for (int i = KEY_1; i <= KEY_0; i++) {
        map_base[i] = nums[i - KEY_1];
        map_shift[i] = syms[i - KEY_1];
    }
    
    map_base[KEY_SPACE] = ' '; map_shift[KEY_SPACE] = ' ';
    map_base[KEY_ENTER] = '\n'; map_shift[KEY_ENTER] = '\n';
    map_base[KEY_BACKSPACE] = '\b'; map_shift[KEY_BACKSPACE] = '\b';

    map_base[KEY_MINUS] = '-'; map_shift[KEY_MINUS] = '_';
    map_base[KEY_EQUAL] = '='; map_shift[KEY_EQUAL] = '+';
    map_base[KEY_LEFTBRACE] = '['; map_shift[KEY_LEFTBRACE] = '{';
    map_base[KEY_RIGHTBRACE] = ']'; map_shift[KEY_RIGHTBRACE] = '}';
    map_base[KEY_BACKSLASH] = '\\'; map_shift[KEY_BACKSLASH] = '|';
    map_base[KEY_SEMICOLON] = ';'; map_shift[KEY_SEMICOLON] = ':';
    map_base[KEY_APOSTROPHE] = '\''; map_shift[KEY_APOSTROPHE] = '"';
    map_base[KEY_GRAVE] = '`'; map_shift[KEY_GRAVE] = '~';
    map_base[KEY_COMMA] = ','; map_shift[KEY_COMMA] = '<';
    map_base[KEY_DOT] = '.'; map_shift[KEY_DOT] = '>';
    map_base[KEY_SLASH] = '/'; map_shift[KEY_SLASH] = '?';
}

void tsfi_input_update_key(uint32_t scancode, bool pressed) {
    uint32_t mod = 0;
    if (scancode == KEY_LEFTSHIFT || scancode == KEY_RIGHTSHIFT) mod = 1; // TSFI_MOD_SHIFT
    else if (scancode == KEY_LEFTCTRL || scancode == KEY_RIGHTCTRL) mod = 4; // TSFI_MOD_CTRL
    else if (scancode == KEY_LEFTALT || scancode == KEY_RIGHTALT) mod = 8; // TSFI_MOD_ALT
    else if (scancode == KEY_CAPSLOCK && pressed) {
        active_modifiers ^= 16; // TSFI_MOD_CAPS
        return;
    }

    if (mod != 0) {
        if (pressed) active_modifiers |= mod;
        else active_modifiers &= ~mod;
    }
}

uint32_t tsfi_input_map_to_utf32(uint32_t scancode) {
    if (scancode >= 256) return 0;
    
    bool shift = (active_modifiers & 1); // Shift
    
    // CAPS Logic
    uint32_t base = map_base[scancode];
    bool is_letter = (base >= 'a' && base <= 'z');
    
    if ((active_modifiers & 16) && is_letter) {
        shift = !shift; 
    }

    return shift ? map_shift[scancode] : map_base[scancode];
}

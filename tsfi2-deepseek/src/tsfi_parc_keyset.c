#include "tsfi_parc_keyset.h"

// Maps 5-bit chord combinations to alphanumeric ASCII characters
static const char chord_map[32] = {
    '\0', 'a',  'b',  'c',  'd',  'e',  'f',  'g',
    'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
    'p',  'q',  'r',  's',  't',  'u',  'v',  'w',
    'x',  'y',  'z',  ' ',  '\n', '\r', '\t', '\b'
};

char tsfi_parc_decode_keyset(uint8_t chord_state) {
    // Only map the lower 5 bits of the chord state
    uint8_t index = chord_state & 0x1F;
    char c = chord_map[index];
    // If bit 5 (0x20) is active, convert alphabetical characters to uppercase
    if ((chord_state & 0x20) && c >= 'a' && c <= 'z') {
        c = c - 'a' + 'A';
    }
    return c;
}

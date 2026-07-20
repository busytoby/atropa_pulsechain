#ifndef TSFI_PARC_KEYBOARD_H
#define TSFI_PARC_KEYBOARD_H

#include <stdint.h>

/* Translate Xerox Alto physical keyboard scan code matrix coordinate to ASCII */
char tsfi_parc_kb_translate(uint8_t row, uint8_t column, int shift_active);

#endif // TSFI_PARC_KEYBOARD_H

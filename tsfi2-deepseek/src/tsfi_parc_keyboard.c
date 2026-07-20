#include "tsfi_parc_keyboard.h"

// 4 rows by 15 columns scan matrix mapping
static const char scan_matrix[4][15] = {
    { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\\', '\b', '\0' },
    { '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', '\0' },
    { '\0', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '\n', '\0', '\0' },
    { '\0', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', '\0', ' ', '\0', '\0' }
};

static const char scan_matrix_shift[4][15] = {
    { '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '|', '\b', '\0' },
    { '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', '\0' },
    { '\0', 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '\n', '\0', '\0' },
    { '\0', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', '\0', ' ', '\0', '\0' }
};

char tsfi_parc_kb_translate(uint8_t row, uint8_t column, int shift_active) {
    if (row >= 4 || column >= 15) return '\0';

    if (shift_active) {
        return scan_matrix_shift[row][column];
    }
    return scan_matrix[row][column];
}

#include "tsfi_comm.h"
#include <string.h>

#define CARD_SIZE 80
#define BLOCK_SIZE 512
#define BLANK_MARKER 0x1F // Unit Separator as compression marker

// Task 1: Translation Table (Example: Simple Caesar-like shift for non-alphanumerics)
static uint8_t translate_char(uint8_t c) {
    if (c >= 'a' && c <= 'z') return c - ('a' - 'A'); // Simple toupper for example translation
    return c;
}

size_t tsfi_FacilitateComm(const uint8_t *input_cards, size_t num_cards, uint8_t *output_blocks) {
    size_t current_block_idx = 0;
    size_t block_offset = 0;
    memset(output_blocks, 0, BLOCK_SIZE);

    for (size_t i = 0; i < num_cards; i++) {
        const uint8_t *card = input_cards + (i * CARD_SIZE);
        
        // Pass 1: Translation
        uint8_t translated[CARD_SIZE];
        for (int j = 0; j < CARD_SIZE; j++) {
            translated[j] = translate_char(card[j]);
        }

        // Pass 2: Blank Compression
        uint8_t compressed[CARD_SIZE * 2]; // Worst case no compression
        size_t c_idx = 0;
        for (int j = 0; j < CARD_SIZE; j++) {
            if (translated[j] == ' ') {
                uint8_t count = 1;
                while (j + 1 < CARD_SIZE && translated[j+1] == ' ' && count < 255) {
                    count++;
                    j++;
                }
                if (count > 1) {
                    compressed[c_idx++] = BLANK_MARKER;
                    compressed[c_idx++] = count;
                } else {
                    compressed[c_idx++] = ' ';
                }
            } else {
                compressed[c_idx++] = translated[j];
            }
        }

        // Pass 3: Record Blocking (Each record prefixed by its length byte)
        size_t record_len = c_idx;
        // Total bytes needed: length byte + compressed data
        if (block_offset + 1 + record_len > BLOCK_SIZE) {
            // Finalize current block and move to next
            current_block_idx++;
            block_offset = 0;
            memset(output_blocks + (current_block_idx * BLOCK_SIZE), 0, BLOCK_SIZE);
        }

        uint8_t *dest = output_blocks + (current_block_idx * BLOCK_SIZE) + block_offset;
        dest[0] = (uint8_t)record_len;
        memcpy(dest + 1, compressed, record_len);
        block_offset += (1 + record_len);
    }

    return current_block_idx + 1;
}

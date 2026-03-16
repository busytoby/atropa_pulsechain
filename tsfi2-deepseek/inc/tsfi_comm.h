#ifndef TSFI_COMM_H
#define TSFI_COMM_H

#include <stdint.h>
#include <stddef.h>

// Core Communication Facilitator Interface
// input_cards: Pointer to buffer containing N 80-character card images.
// num_cards: Number of 80-character cards to process.
// output_blocks: Buffer to receive 512-byte physical records.
// Returns the number of 512-byte blocks generated.
size_t tsfi_FacilitateComm(const uint8_t *input_cards, size_t num_cards, uint8_t *output_blocks);

#endif

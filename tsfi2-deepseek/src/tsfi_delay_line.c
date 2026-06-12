#include "tsfi_delay_line.h"
#include <stdlib.h>
#include <string.h>

void tsfi_delay_line_init(TsfiDelayLineProcessor *proc, int length) {
    memset(proc, 0, sizeof(TsfiDelayLineProcessor));
    proc->buffer = calloc(length, sizeof(uint8_t));
    proc->length = length;
    proc->write_index = 0;
    proc->write_enable = 0;
    proc->recirculate_enable = 1;
}

void tsfi_delay_line_free(TsfiDelayLineProcessor *proc) {
    if (proc->buffer != NULL) {
        free(proc->buffer);
        proc->buffer = NULL;
    }
}

uint8_t tsfi_delay_line_step(TsfiDelayLineProcessor *proc, uint8_t input_bit) {
    if (proc->buffer == NULL || proc->length <= 0) return 0;

    // Read the output bit (delayed by 'length' steps)
    uint8_t output_bit = proc->buffer[proc->write_index];

    // Determine the next bit to store
    uint8_t next_bit = 0;
    if (proc->write_enable) {
        next_bit = input_bit;
    } else if (proc->recirculate_enable) {
        next_bit = output_bit; // Reshaped and recirculated
    }

    // Write back to current index
    proc->buffer[proc->write_index] = next_bit;

    // Shift pointer circularly
    proc->write_index = (proc->write_index + 1) % proc->length;

    return output_bit;
}

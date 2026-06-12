#ifndef TSFI_DELAY_LINE_H
#define TSFI_DELAY_LINE_H

#include <stdint.h>

typedef struct {
    uint8_t *buffer;
    int length;
    int write_index;
    int write_enable;
    int recirculate_enable;
} TsfiDelayLineProcessor;

void tsfi_delay_line_init(TsfiDelayLineProcessor *proc, int length);
void tsfi_delay_line_free(TsfiDelayLineProcessor *proc);
uint8_t tsfi_delay_line_step(TsfiDelayLineProcessor *proc, uint8_t input_bit);

#endif /* TSFI_DELAY_LINE_H */
